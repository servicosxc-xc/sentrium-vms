#include "DeviceWorker.hpp"

#include <QByteArray>
#include <algorithm>

namespace {
std::string utf8(const QString& value) {
    const QByteArray bytes = value.toUtf8();
    return std::string(bytes.constData(), static_cast<std::size_t>(bytes.size()));
}

QString qstring(const std::string& value) {
    return QString::fromUtf8(value.data(), static_cast<qsizetype>(value.size()));
}
} // namespace

DeviceWorker::DeviceWorker(QObject* parent) : QObject(parent) {
    connect(this, &DeviceWorker::registrationReceived,
            this, &DeviceWorker::onRegistrationReceived,
            Qt::QueuedConnection);
}

void DeviceWorker::connectDevice(
    const QString& id,
    const QString& host,
    quint16 port,
    const QString& username,
    const QString& password) {

    const std::string key = utf8(id);
    if (sessions_.contains(key)) {
        emit connectFinished(id, false, 0, QStringLiteral("Device already connected"));
        return;
    }

    sentrium::device::LoginRequest request{
        utf8(host),
        static_cast<std::uint16_t>(port),
        utf8(username),
        utf8(password)};

    const auto result = adapter_.connect(request);
    if (result.success) {
        sessions_.emplace(key, result.session);
    }

    emit connectFinished(
        id,
        result.success,
        result.channel_count,
        qstring(result.diagnostic));
}

void DeviceWorker::startLive(
    const QString& id, int channel, quint64 nativeWindow, bool extra) {
    // A single preview at a time is intentional for this first field gate.
    // Reuse the established NetSDK session, including AutoRegistration.
    stopLive(live_device_id_);
    const auto it = sessions_.find(utf8(id));
    if (it == sessions_.end() || channel < 0 || channel >= 4096 || nativeWindow == 0) {
        emit liveStarted(id, false, QStringLiteral("Sessão ou canal não disponível para vídeo"));
        return;
    }
    const auto result = adapter_.startLive(
        it->second, channel, static_cast<std::uintptr_t>(nativeWindow), extra);
    if (result.success) {
        live_session_ = result.session;
        live_device_id_ = id;
    }
    emit liveStarted(id, result.success, qstring(result.diagnostic));
}

void DeviceWorker::stopLive(const QString& id) {
    if (!live_session_ || id != live_device_id_) return;
    // Always drop the live handle before NetSDK logout or listener teardown.
    const auto result = adapter_.stopLive(live_session_);
    if (!result.success) {
        emit registrationObservation(QStringLiteral(
            "Prévia ao vivo: não foi possível confirmar StopRealPlayEx"));
    }
    live_session_ = {};
    live_device_id_.clear();
}

void DeviceWorker::disconnectDevice(const QString& id) {
    stopLive(id);
    const std::string key = utf8(id);
    const auto found = sessions_.find(key);
    if (found == sessions_.end()) {
        emit disconnectFinished(id, true, {});
        return;
    }

    const auto result = adapter_.disconnect(found->second);
    if (result.success) {
        sessions_.erase(found);
        stale_registration_sessions_.erase(key);
    } else if (registered_.contains(key)) {
        stale_registration_sessions_.insert(key);
    }

    emit disconnectFinished(id, result.success, qstring(result.diagnostic));
}


void DeviceWorker::startRegistrationServer(const QString& bindAddress, quint16 port) {
    if (listening_) {
        emit registrationServerChanged(true, QStringLiteral("Registro Automático já ativo"));
        return;
    }
    const auto result = adapter_.startRegistration(
        utf8(bindAddress), port,
        [this](const sentrium::intelbras::RegistrationNotice& event) {
            // SDK thread: copy bounded event values into Qt's queued signal.
            // Never call NetSDK Login from inside a native SDK callback.
            emit registrationReceived(
                qstring(event.registration_id),
                qstring(event.peer_address),
                event.peer_port,
                event.disconnected);
        });
    listening_ = result.success;
    emit registrationServerChanged(listening_, qstring(result.diagnostic));
}

void DeviceWorker::stopRegistrationServer() {
    // Logout must happen while the reverse-registration service still owns
    // its connections; closing the listener first may invalidate live sessions.
    for (const auto& entry : registered_) {
        const auto& key = entry.first;
        const auto session = sessions_.find(key);
        if (session == sessions_.end()) continue;

        stopLive(qstring(key));
        const auto result = adapter_.disconnect(session->second);
        if (result.success) {
            sessions_.erase(session);
            stale_registration_sessions_.erase(key);
        } else {
            stale_registration_sessions_.insert(key);
            emit registrationObservation(
                QStringLiteral("Servidor: não foi possível confirmar logout de um dispositivo"));
        }
        emit disconnectFinished(qstring(key), result.success, qstring(result.diagnostic));
    }

    adapter_.stopRegistration();
    listening_ = false;
    emit registrationServerChanged(false, QStringLiteral("Registro Automático desativado"));
}

void DeviceWorker::registerAutoDevice(
    const QString& id, const QString& registrationId,
    const QString& username, const QString& password) {
    const auto key = utf8(id);
    // Registration ID uniqueness prevents accidentally authenticating the same
    // recorder with credentials intended for a different logical device.
    for (const auto& [existing_id, target] : registered_) {
        if (existing_id != key && target.registrationId == registrationId) {
            emit connectFinished(id, false, 0,
                                 QStringLiteral("ID de Registro Automático já cadastrado"));
            return;
        }
    }
    registered_.insert_or_assign(key, RegisteredTarget{registrationId, username, password});
}

void DeviceWorker::unregisterAutoDevice(const QString& id) {
    const auto key = utf8(id);
    registered_.erase(key);
    disconnectDevice(id);
}

void DeviceWorker::onRegistrationReceived(
    const QString& registrationId, const QString& peer, quint16 port, bool disconnected) {
    if (!listening_) return;
    for (const auto& [id, target] : registered_) {
        if (registrationId != target.registrationId) continue;
        // Do not expose this ID, peer IP, username or password to UI/logs.
        emit registrationObservation(disconnected
            ? QStringLiteral("Desconexão informada por dispositivo cadastrado")
            : QStringLiteral("Registro recebido de dispositivo cadastrado"));
        auto existing = sessions_.find(id);
        if (disconnected) {
            if (existing != sessions_.end()) {
                stopLive(qstring(id));
                const auto logout = adapter_.disconnect(existing->second);
                if (!logout.success) {
                    emit registrationObservation(QStringLiteral(
                        "Gravador desconectou, mas o NetSDK não confirmou a liberação da sessão"));
                }
                if (logout.success) {
                    sessions_.erase(existing);
                    stale_registration_sessions_.erase(id);
                } else {
                    // Keep the handle for retry on the next registration,
                    // rather than accepting a duplicate NetSDK session.
                    stale_registration_sessions_.insert(id);
                }
                emit registrationLost(qstring(id));
            }
            return;
        }
        if (existing != sessions_.end()) {
            if (!stale_registration_sessions_.contains(id)) return;
            stopLive(qstring(id));
            const auto logout = adapter_.disconnect(existing->second);
            if (!logout.success) {
                emit registrationObservation(QStringLiteral(
                    "Novo registro aguardando liberação da sessão NetSDK anterior"));
                return;
            }
            sessions_.erase(existing);
            stale_registration_sessions_.erase(id);
        }
        const QString logicalId = qstring(id);
        if (peer.isEmpty() || port == 0) {
            emit registrationObservation(QStringLiteral("Registro reconhecido sem endpoint válido"));
            emit connectFinished(logicalId, false, 0,
                                 QStringLiteral("Registro recebido sem endereço válido"));
            return;
        }
        const sentrium::device::LoginRequest request{
            utf8(peer), static_cast<std::uint16_t>(port),
            utf8(target.username), utf8(target.password)};
        const auto result = adapter_.connectRegistered(request, utf8(registrationId));
        if (result.success) {
            sessions_.emplace(id, result.session);
            emit registrationObservation(QStringLiteral(
                "Login NetSDK por Registro Automático concluído; canais reais obtidos: %1")
                .arg(result.channel_count));
        } else {
            emit registrationObservation(QStringLiteral(
                "Registro reconhecido, mas login NetSDK falhou (verifique diagnóstico do dispositivo)"));
        }
        emit connectFinished(logicalId, result.success,
                             result.channel_count, qstring(result.diagnostic));
        return;
    }
    // Never auto-enroll unexpected equipment, but show a privacy-safe hint
    // so the operator can detect an ID mismatch when using a real recorder.
    emit registrationObservation(QStringLiteral(
        "Registro recebido com ID não cadastrado (verifique o ID no gravador)"));
}

void DeviceWorker::shutdown() {
    stopLive(live_device_id_);
    adapter_.stopRegistration();
    listening_ = false;
    registered_.clear();
    for (auto it = sessions_.begin(); it != sessions_.end();) {
        const auto result = adapter_.disconnect(it->second);
        if (result.success) {
            it = sessions_.erase(it);
        } else {
            ++it;
        }
    }

    // Adapter destruction performs CLIENT_Cleanup even if an individual
    // logout could not be completed, so process shutdown remains bounded.
    sessions_.clear();
    stale_registration_sessions_.clear();
}
