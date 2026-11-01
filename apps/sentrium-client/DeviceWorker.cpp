#include "DeviceWorker.hpp"

#include <QByteArray>
#include <QTimer>
#include <algorithm>
#include <vector>

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
    connect(this, &DeviceWorker::sdkSessionStatus,
            this, &DeviceWorker::onSdkSessionStatus,
            Qt::QueuedConnection);
    adapter_.setSessionStatusCallback([this](sentrium::device::Session session, bool online) {
        // NetSDK callback is not the Qt worker thread: send only the opaque
        // login handle through the queued signal; no SDK calls here.
        emit sdkSessionStatus(static_cast<quint64>(session.value), online);
    });
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

void DeviceWorker::startLive(int slot, quint64 token, const QString& id,
                             int channel, quint64 nativeWindow, bool extra) {
    // One NetSDK RealPlay handle per independently embedded Qt tile.
    // All NetSDK operations remain on the existing worker thread.
    if (slot < 0 || slot >= 64) {
        emit liveStarted(slot, token, id, false, QStringLiteral("Índice de mosaico inválido"));
        return;
    }
    stopLive(slot);
    const auto it = sessions_.find(utf8(id));
    if (it == sessions_.end() || channel < 0 || channel >= 4096 || nativeWindow == 0) {
        emit liveStarted(slot, token, id, false,
                         QStringLiteral("Sessão, canal ou superfície de vídeo inválida"));
        return;
    }
    const auto result = adapter_.startLive(
        it->second, channel, static_cast<std::uintptr_t>(nativeWindow), extra);
    if (result.success) {
        live_sessions_.emplace(slot, LiveEntry{id, result.session});
    }
    emit liveStarted(slot, token, id, result.success, qstring(result.diagnostic));
}

void DeviceWorker::stopLive(int slot) {
    const auto it = live_sessions_.find(slot);
    if (it == live_sessions_.end()) return;
    const auto result = adapter_.stopLive(it->second.handle);
    if (!result.success) {
        emit registrationObservation(QStringLiteral(
            "Mosaico: StopRealPlayEx não confirmou a liberação de um canal"));
    }
    live_sessions_.erase(it);
}

void DeviceWorker::stopLiveForDevice(const QString& id) {
    std::vector<int> slot_ids;
    for (const auto& [slot, entry] : live_sessions_) {
        if (entry.deviceId == id) slot_ids.push_back(slot);
    }
    for (const int slot : slot_ids) stopLive(slot);
}

void DeviceWorker::stopAllLive() {
    while (!live_sessions_.empty()) stopLive(live_sessions_.begin()->first);
}

void DeviceWorker::disconnectDevice(const QString& id) {
    stopLiveForDevice(id);
    const std::string key = utf8(id);
    const auto found = sessions_.find(key);
    if (found == sessions_.end()) {
        offline_sessions_.erase(key);
        stale_registration_sessions_.erase(key);
        session_channel_counts_.erase(key);
        emit disconnectFinished(id, true, {});
        return;
    }

    const auto result = adapter_.disconnect(found->second);
    if (result.success) {
        sessions_.erase(found);
        stale_registration_sessions_.erase(key);
        offline_sessions_.erase(key);
        session_channel_counts_.erase(key);
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
    if (listening_) {
        if (!registration_retry_timer_) {
            registration_retry_timer_ = new QTimer(this);
            registration_retry_timer_->setInterval(15000);
            connect(registration_retry_timer_, &QTimer::timeout,
                    this, &DeviceWorker::retryPendingRegistrations);
        }
        registration_retry_timer_->start();
    }
    emit registrationServerChanged(listening_, qstring(result.diagnostic));
}

void DeviceWorker::stopRegistrationServer() {
    if (registration_retry_timer_) registration_retry_timer_->stop();
    // Logout must happen while the reverse-registration service still owns
    // its connections; closing the listener first may invalidate live sessions.
    for (const auto& entry : registered_) {
        const auto& key = entry.first;
        const auto session = sessions_.find(key);
        if (session == sessions_.end()) continue;

        stopLiveForDevice(qstring(key));
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
    offline_sessions_.clear();
    for (auto& [id, target] : registered_) {
        target.lastPeer.clear();
        target.lastPort = 0;
    }
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
    auto current = registered_.find(key);
    if (current != registered_.end()) {
        // Keep the last verified endpoint when the user explicitly retries
        // the same registration ID; no spurious shutdown/reconfiguration.
        current->second.registrationId = registrationId;
        current->second.username = username;
        current->second.password = password;
    } else {
        registered_.emplace(key, RegisteredTarget{registrationId, username, password, {}, 0});
    }
}

void DeviceWorker::unregisterAutoDevice(const QString& id) {
    const auto key = utf8(id);
    registered_.erase(key);
    offline_sessions_.erase(key);
    stale_registration_sessions_.erase(key);
    session_channel_counts_.erase(key);
    disconnectDevice(id);
}

void DeviceWorker::attemptRegisteredLogin(
    const std::string& id, const RegisteredTarget& target) {
    if (!listening_ || target.lastPeer.isEmpty() || target.lastPort == 0)
        return;

    const QString logicalId = qstring(id);
    auto existing = sessions_.find(id);
    if (existing != sessions_.end()) {
        if (!offline_sessions_.contains(id)) return;
        stopLiveForDevice(logicalId);
        // A lost NetSDK login handle must not block every future
        // re-registration forever if Logout fails on a dead connection.
        const auto logout = adapter_.disconnect(existing->second);
        if (!logout.success) {
            emit registrationObservation(QStringLiteral(
                "Sessão NetSDK anterior indisponível; tentando recuperar o registro"));
        }
        sessions_.erase(existing);
        session_channel_counts_.erase(id);
    }

    const sentrium::device::LoginRequest request{
        utf8(target.lastPeer), static_cast<std::uint16_t>(target.lastPort),
        utf8(target.username), utf8(target.password)};
    const auto result = adapter_.connectRegistered(request, utf8(target.registrationId));
    if (result.success) {
        sessions_.insert_or_assign(id, result.session);
        session_channel_counts_.insert_or_assign(id, result.channel_count);
        offline_sessions_.erase(id);
        stale_registration_sessions_.erase(id);
        emit registrationObservation(QStringLiteral(
            "Registro Automático reconectado; canais reais: %1").arg(result.channel_count));
    } else {
        offline_sessions_.insert(id);
        emit registrationObservation(QStringLiteral(
            "Registro Automático: login indisponível; tentativa automática programada"));
    }
    emit connectFinished(logicalId, result.success,
                         result.channel_count, qstring(result.diagnostic));
}

void DeviceWorker::retryPendingRegistrations() {
    if (!listening_) return;
    for (const auto& [id, target] : registered_) {
        if (target.lastPeer.isEmpty() || target.lastPort == 0) continue;
        if (sessions_.contains(id) && !offline_sessions_.contains(id)) continue;
        attemptRegisteredLogin(id, target);
    }
}

void DeviceWorker::onSdkSessionStatus(quint64 handle, bool online) {
    if (!handle) return;
    for (const auto& [id, session] : sessions_) {
        if (static_cast<quint64>(session.value) != handle) continue;
        if (!registered_.contains(id)) return; // Ordinary LAN sessions are unchanged.
        if (!online) {
            if (offline_sessions_.insert(id).second) {
                stopLiveForDevice(qstring(id));
                emit registrationObservation(QStringLiteral(
                    "NetSDK detectou queda de sessão; aguardando reconexão automática"));
                emit registrationLost(qstring(id));
            }
        } else if (offline_sessions_.erase(id) != 0) {
            // CLIENT_SetAutoReconnect already restored this SDK login handle.
            // Do not create a competing LoginEx2 or require the operator to
            // edit/salvar the NVR again.
            emit registrationObservation(QStringLiteral(
                "NetSDK restabeleceu a sessão automaticamente"));
            const auto found = session_channel_counts_.find(id);
            const int channels = found == session_channel_counts_.end() ? 0 : found->second;
            emit connectFinished(qstring(id), true, channels,
                                 QStringLiteral("Reconexão NetSDK automática"));
        }
        return;
    }
}

void DeviceWorker::onRegistrationReceived(
    const QString& registrationId, const QString& peer, quint16 port, bool disconnected) {
    if (!listening_) return;
    for (auto& [id, target] : registered_) {
        if (registrationId != target.registrationId) continue;
        const QString logicalId = qstring(id);
        if (disconnected) {
            // Registration transport can disappear independently from the
            // NetSDK login handle. Keep that handle for SDK auto-reconnect;
            // never immediately tear it down and lose the callback.
            if (sessions_.contains(id) && offline_sessions_.insert(id).second) {
                stopLiveForDevice(logicalId);
                emit registrationLost(logicalId);
            }
            emit registrationObservation(QStringLiteral(
                "Registrador informou desconexão; aguardando retorno automático"));
            return;
        }
        if (peer.isEmpty() || port == 0) {
            emit registrationObservation(QStringLiteral(
                "Registro reconhecido sem endereço válido"));
            return;
        }
        const bool endpointChanged = peer != target.lastPeer || port != target.lastPort;
        target.lastPeer = peer;
        target.lastPort = port;
        if (sessions_.contains(id) && !offline_sessions_.contains(id) && !endpointChanged) {
            return; // duplicate heartbeat/registration on the current session
        }
        if (sessions_.contains(id) && endpointChanged) {
            if (offline_sessions_.insert(id).second) {
                stopLiveForDevice(logicalId);
                emit registrationLost(logicalId);
            }
        }
        attemptRegisteredLogin(id, target);
        return;
    }
    emit registrationObservation(QStringLiteral(
        "Registro recebido com ID não cadastrado (verifique o ID no gravador)"));
}

void DeviceWorker::shutdown() {
    if (registration_retry_timer_) registration_retry_timer_->stop();
    adapter_.setSessionStatusCallback({});
    stopAllLive();
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
    offline_sessions_.clear();
    session_channel_counts_.clear();
}
