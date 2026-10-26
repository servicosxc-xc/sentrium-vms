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

void DeviceWorker::disconnectDevice(const QString& id) {
    const std::string key = utf8(id);
    const auto found = sessions_.find(key);
    if (found == sessions_.end()) {
        emit disconnectFinished(id, true, {});
        return;
    }

    const auto result = adapter_.disconnect(found->second);
    if (result.success) {
        sessions_.erase(found);
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
                qstring(event.serial),
                qstring(event.peer_address),
                event.peer_port,
                event.disconnected);
        });
    listening_ = result.success;
    emit registrationServerChanged(listening_, qstring(result.diagnostic));
}

void DeviceWorker::stopRegistrationServer() {
    adapter_.stopRegistration();
    listening_ = false;
    for (const auto& [key, target] : registered_) {
        const auto session = sessions_.find(key);
        if (session != sessions_.end()) {
            adapter_.disconnect(session->second);
            sessions_.erase(session);
            emit disconnectFinished(qstring(key), true, {});
        }
    }
    emit registrationServerChanged(false, QStringLiteral("Registro Automático desativado"));
}

void DeviceWorker::registerAutoDevice(
    const QString& id, const QString& serial,
    const QString& username, const QString& password) {
    const auto key = utf8(id);
    // Serial uniqueness prevents accidentally authenticating the same
    // recorder with credentials intended for a different logical device.
    for (const auto& [existing_id, target] : registered_) {
        if (existing_id != key && target.serial == serial) {
            emit connectFinished(id, false, 0,
                                 QStringLiteral("Número de série já cadastrado"));
            return;
        }
    }
    registered_.insert_or_assign(key, RegisteredTarget{serial, username, password});
}

void DeviceWorker::unregisterAutoDevice(const QString& id) {
    const auto key = utf8(id);
    registered_.erase(key);
    disconnectDevice(id);
}

void DeviceWorker::onRegistrationReceived(
    const QString& serial, const QString& peer, quint16 port, bool disconnected) {
    if (!listening_) return;
    for (const auto& [id, target] : registered_) {
        if (serial != target.serial) continue;
        auto existing = sessions_.find(id);
        if (disconnected) {
            if (existing != sessions_.end()) {
                adapter_.disconnect(existing->second);
                sessions_.erase(existing);
                emit registrationLost(qstring(id));
            }
            return;
        }
        if (existing != sessions_.end()) return;
        const QString logicalId = qstring(id);
        if (peer.isEmpty() || port == 0) {
            emit connectFinished(logicalId, false, 0,
                                 QStringLiteral("Registro recebido sem endereço válido"));
            return;
        }
        const sentrium::device::LoginRequest request{
            utf8(peer), static_cast<std::uint16_t>(port),
            utf8(target.username), utf8(target.password)};
        const auto result = adapter_.connectRegistered(request, utf8(serial));
        if (result.success) {
            sessions_.emplace(id, result.session);
        }
        emit connectFinished(logicalId, result.success,
                             result.channel_count, qstring(result.diagnostic));
        return;
    }
    // Unknown serial: ignore it; never auto-enroll unexpected equipment.
}

void DeviceWorker::shutdown() {
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
}
