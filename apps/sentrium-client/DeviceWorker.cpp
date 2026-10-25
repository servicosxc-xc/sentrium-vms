#include "DeviceWorker.hpp"

#include <QByteArray>

namespace {
std::string utf8(const QString& value) {
    const QByteArray bytes = value.toUtf8();
    return std::string(bytes.constData(), static_cast<std::size_t>(bytes.size()));
}

QString qstring(const std::string& value) {
    return QString::fromUtf8(value.data(), static_cast<qsizetype>(value.size()));
}
} // namespace

DeviceWorker::DeviceWorker(QObject* parent) : QObject(parent) {}

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

void DeviceWorker::shutdown() {
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
