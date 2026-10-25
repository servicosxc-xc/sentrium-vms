#include "DeviceController.hpp"
#include "DeviceWorker.hpp"

#include <QByteArray>
#include <QMetaObject>
#include <QUuid>
#include <QVariantMap>

#include <algorithm>
#include <stdexcept>

namespace {
std::string utf8(const QString& value) {
    const QByteArray bytes = value.toUtf8();
    return std::string(bytes.constData(), static_cast<std::size_t>(bytes.size()));
}

QString qstring(std::string_view value) {
    return QString::fromUtf8(value.data(), static_cast<qsizetype>(value.size()));
}
} // namespace

DeviceController::DeviceController(QObject* parent) : QObject(parent) {
    worker_ = new DeviceWorker;
    worker_->moveToThread(&worker_thread_);

    connect(
        &worker_thread_,
        &QThread::finished,
        worker_,
        &QObject::deleteLater);
    connect(
        this,
        &DeviceController::requestConnect,
        worker_,
        &DeviceWorker::connectDevice,
        Qt::QueuedConnection);
    connect(
        this,
        &DeviceController::requestDisconnect,
        worker_,
        &DeviceWorker::disconnectDevice,
        Qt::QueuedConnection);
    connect(
        worker_,
        &DeviceWorker::connectFinished,
        this,
        &DeviceController::onConnectFinished,
        Qt::QueuedConnection);
    connect(
        worker_,
        &DeviceWorker::disconnectFinished,
        this,
        &DeviceController::onDisconnectFinished,
        Qt::QueuedConnection);

    worker_thread_.setObjectName(QStringLiteral("sentrium-device-worker"));
    worker_thread_.start();
}

DeviceController::~DeviceController() {
    shutdown();
}

QVariantList DeviceController::devices() const {
    QVariantList result;
    result.reserve(static_cast<qsizetype>(records_.size()));

    for (const auto& record : records_) {
        QVariantList channels;
        channels.reserve(static_cast<qsizetype>(record.device.channels.size()));
        for (const auto& channel : record.device.channels) {
            QVariantMap item;
            item.insert(QStringLiteral("index"), channel.index);
            item.insert(QStringLiteral("name"), qstring(channel.name));
            channels.push_back(item);
        }

        QVariantMap item;
        item.insert(QStringLiteral("id"), qstring(record.device.id));
        item.insert(QStringLiteral("name"), qstring(record.device.display_name));
        item.insert(QStringLiteral("host"), qstring(record.device.host));
        item.insert(QStringLiteral("port"), static_cast<int>(record.device.port));
        item.insert(QStringLiteral("manufacturer"), qstring(record.device.manufacturer));
        item.insert(QStringLiteral("state"), qstring(sentrium::device::name(record.device.state)));
        item.insert(QStringLiteral("diagnostic"), qstring(record.device.diagnostic));
        item.insert(QStringLiteral("channels"), channels);
        result.push_back(item);
    }

    return result;
}

void DeviceController::setError(const QString& error) {
    if (last_error_ == error) {
        return;
    }
    last_error_ = error;
    emit lastErrorChanged();
}

DeviceController::Record* DeviceController::find(const QString& id) {
    const std::string key = utf8(id);
    const auto found = std::find_if(
        records_.begin(),
        records_.end(),
        [&](const Record& record) { return record.device.id == key; });
    return found == records_.end() ? nullptr : &*found;
}

const DeviceController::Record* DeviceController::find(const QString& id) const {
    const std::string key = utf8(id);
    const auto found = std::find_if(
        records_.cbegin(),
        records_.cend(),
        [&](const Record& record) { return record.device.id == key; });
    return found == records_.cend() ? nullptr : &*found;
}

void DeviceController::addAndConnect(
    const QString& displayName,
    const QString& host,
    int port,
    const QString& username,
    const QString& password) {

    if (shutdown_) {
        setError(QStringLiteral("Device runtime is shutting down"));
        return;
    }

    try {
        if (displayName.trimmed().isEmpty() || displayName.size() > 128) {
            throw std::invalid_argument("Device name must contain 1 to 128 characters");
        }
        if (port < 1 || port > 65535) {
            throw std::invalid_argument("Port must be between 1 and 65535");
        }

        sentrium::device::LoginRequest request{
            utf8(host.trimmed()),
            static_cast<std::uint16_t>(port),
            utf8(username),
            utf8(password)};
        request.validate();

        const QString id = QUuid::createUuid().toString(QUuid::WithoutBraces);

        Record record;
        record.device.id = utf8(id);
        record.device.display_name = utf8(displayName.trimmed());
        record.device.host = request.host;
        record.device.port = request.port;
        record.device.state = sentrium::device::ConnectionState::Connecting;
        record.username = request.username;
        record.password = request.password;

        records_.push_back(std::move(record));
        setError({});
        emit devicesChanged();
        emit auditEvent(
            QStringLiteral("device-connect-requested"),
            QStringLiteral("Device added in memory and connection requested"));
        emit requestConnect(id, host.trimmed(), static_cast<quint16>(port), username, password);
    } catch (const std::exception& error) {
        setError(QString::fromUtf8(error.what()));
    }
}

void DeviceController::connectDevice(const QString& id) {
    if (shutdown_) {
        return;
    }

    auto* record = find(id);
    if (!record) {
        setError(QStringLiteral("Device not found"));
        return;
    }

    if (record->device.state == sentrium::device::ConnectionState::Connecting
        || record->device.state == sentrium::device::ConnectionState::Connected
        || record->device.state == sentrium::device::ConnectionState::Disconnecting) {
        return;
    }

    record->device.state = sentrium::device::ConnectionState::Connecting;
    record->device.diagnostic.clear();
    setError({});
    emit devicesChanged();
    emit auditEvent(
        QStringLiteral("device-connect-requested"),
        QStringLiteral("Device connection requested"));
    emit requestConnect(
        id,
        qstring(record->device.host),
        record->device.port,
        qstring(record->username),
        qstring(record->password));
}

void DeviceController::disconnectDevice(const QString& id) {
    auto* record = find(id);
    if (!record) {
        setError(QStringLiteral("Device not found"));
        return;
    }

    if (record->device.state != sentrium::device::ConnectionState::Connected
        && record->device.state != sentrium::device::ConnectionState::Failed) {
        return;
    }

    record->device.state = sentrium::device::ConnectionState::Disconnecting;
    record->device.diagnostic.clear();
    emit devicesChanged();
    emit auditEvent(
        QStringLiteral("device-disconnect-requested"),
        QStringLiteral("Device disconnect requested"));
    emit requestDisconnect(id);
}

void DeviceController::removeDevice(const QString& id) {
    auto* record = find(id);
    if (!record) {
        return;
    }

    if (record->device.state == sentrium::device::ConnectionState::Connected
        || record->device.state == sentrium::device::ConnectionState::Disconnecting) {
        record->pending_remove = true;
        if (record->device.state == sentrium::device::ConnectionState::Connected) {
            disconnectDevice(id);
        }
        return;
    }

    if (record->device.state == sentrium::device::ConnectionState::Connecting) {
        record->pending_remove = true;
        return;
    }

    erase(id);
    emit auditEvent(
        QStringLiteral("device-removed"),
        QStringLiteral("Device removed from memory"));
}

void DeviceController::erase(const QString& id) {
    const std::string key = utf8(id);
    const auto found = std::find_if(
        records_.begin(),
        records_.end(),
        [&](const Record& record) { return record.device.id == key; });
    if (found == records_.end()) {
        return;
    }

    std::fill(found->password.begin(), found->password.end(), '\0');
    found->password.clear();
    found->username.clear();
    records_.erase(found);
    emit devicesChanged();
}

void DeviceController::onConnectFinished(
    const QString& id,
    bool success,
    int channelCount,
    const QString& diagnostic) {

    auto* record = find(id);
    if (!record) {
        if (success) {
            emit requestDisconnect(id);
        }
        return;
    }

    record->device.diagnostic = utf8(diagnostic);
    if (success) {
        try {
            record->device.channels = sentrium::device::make_channels(channelCount);
            record->device.state = sentrium::device::ConnectionState::Connected;
            emit auditEvent(
                QStringLiteral("device-connected"),
                QStringLiteral("Device connected and channel count received"));
        } catch (const std::exception& error) {
            record->device.state = sentrium::device::ConnectionState::Failed;
            record->device.diagnostic = error.what();
            emit requestDisconnect(id);
        }
    } else {
        record->device.channels.clear();
        record->device.state = sentrium::device::ConnectionState::Failed;
        emit auditEvent(
            QStringLiteral("device-connect-failed"),
            QStringLiteral("Device connection failed"));
    }

    const bool remove_after = record->pending_remove;
    emit devicesChanged();

    if (remove_after) {
        if (success) {
            disconnectDevice(id);
        } else {
            erase(id);
        }
    }
}

void DeviceController::onDisconnectFinished(
    const QString& id,
    bool success,
    const QString& diagnostic) {

    auto* record = find(id);
    if (!record) {
        return;
    }

    if (success) {
        record->device.state = sentrium::device::ConnectionState::Disconnected;
        record->device.channels.clear();
        record->device.diagnostic.clear();
        emit auditEvent(
            QStringLiteral("device-disconnected"),
            QStringLiteral("Device disconnected"));
        const bool remove_after = record->pending_remove;
        emit devicesChanged();
        if (remove_after) {
            erase(id);
            emit auditEvent(
                QStringLiteral("device-removed"),
                QStringLiteral("Device removed from memory"));
        }
    } else {
        record->pending_remove = false;
        record->device.state = sentrium::device::ConnectionState::Failed;
        record->device.diagnostic = utf8(diagnostic);
        emit devicesChanged();
        emit auditEvent(
            QStringLiteral("device-disconnect-failed"),
            QStringLiteral("Device disconnect failed"));
    }
}

void DeviceController::shutdown() {
    if (shutdown_) {
        return;
    }
    shutdown_ = true;

    if (worker_ && worker_thread_.isRunning()) {
        QMetaObject::invokeMethod(
            worker_,
            &DeviceWorker::shutdown,
            Qt::BlockingQueuedConnection);
        worker_thread_.quit();
        worker_thread_.wait();
    }

    worker_ = nullptr;
    for (auto& record : records_) {
        std::fill(record.password.begin(), record.password.end(), '\0');
        record.password.clear();
        record.username.clear();
        record.device.channels.clear();
        record.device.state = sentrium::device::ConnectionState::Disconnected;
    }
}
