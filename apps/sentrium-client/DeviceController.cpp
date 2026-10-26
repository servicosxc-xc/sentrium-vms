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


    connect(this, &DeviceController::requestStartRegistrationServer,
            worker_, &DeviceWorker::startRegistrationServer, Qt::QueuedConnection);
    connect(this, &DeviceController::requestStopRegistrationServer,
            worker_, &DeviceWorker::stopRegistrationServer, Qt::QueuedConnection);
    connect(this, &DeviceController::requestRegisterAutoDevice,
            worker_, &DeviceWorker::registerAutoDevice, Qt::QueuedConnection);
    connect(this, &DeviceController::requestUnregisterAutoDevice,
            worker_, &DeviceWorker::unregisterAutoDevice, Qt::QueuedConnection);
    connect(worker_, &DeviceWorker::registrationServerChanged,
            this, &DeviceController::onRegistrationServerChanged, Qt::QueuedConnection);
    connect(worker_, &DeviceWorker::registrationLost,
            this, &DeviceController::onRegistrationLost, Qt::QueuedConnection);
    connect(worker_, &DeviceWorker::registrationObservation,
            this, &DeviceController::onRegistrationObservation, Qt::QueuedConnection);

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

        item.insert(QStringLiteral("method"),
                    qstring(sentrium::device::name(record.device.access_method)));
        item.insert(QStringLiteral("cloudRoute"),
                    qstring(sentrium::device::name(record.device.cloud_route)));
        item.insert(QStringLiteral("connectionLabel"), record.auto_registration
                    ? QStringLiteral("Registro Automático • ID de registro · final ")
                        + qstring(record.registration_id.substr(
                            record.registration_id.size() > 4
                            ? record.registration_id.size() - 4 : 0))
                    : record.device.access_method == sentrium::device::AccessMethod::Cloud
                      ? QStringLiteral("Túnel Cloud local • 127.0.0.1:")
                          + QString::number(record.device.port)
                          + QStringLiteral(" • rota não verificada")
                      : qstring(record.device.host) + QStringLiteral(":")
                          + QString::number(record.device.port));
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


void DeviceController::addCloudBridgeAndConnect(
    const QString& displayName, int localPort,
    const QString& username, const QString& password) {
    // A Windows Defender detection suspended the external Cloud bridge.
    // Enforce the same safety hold at the C++ boundary, not only in QML:
    // an unknown loopback process must never receive NVR credentials.
    Q_UNUSED(displayName)
    Q_UNUSED(localPort)
    Q_UNUSED(username)
    Q_UNUSED(password)
    if (shutdown_) return;
    setError(QStringLiteral(
        "Cloud/P2P bloqueado: não existe transporte independente verificado"));
    emit auditEvent(QStringLiteral("cloud-blocked"),
                    QStringLiteral("Túnel de terceiros bloqueado por segurança"));
}

bool DeviceController::addAutoRegistration(
    const QString& displayName, const QString& registrationId,
    const QString& username, const QString& password) {
    if (shutdown_) return false;
    const QString cleanRegistrationId = registrationId.trimmed();
    if (displayName.trimmed().isEmpty() || displayName.size() > 128
        || cleanRegistrationId.isEmpty() || cleanRegistrationId.size() >= 32
        || username.isEmpty() || username.size() > 128 || password.size() > 256) {
        setError(QStringLiteral("Dados de Registro Automático inválidos"));
        return false;
    }
    for (const QChar c : cleanRegistrationId) {
        if (!((c >= QLatin1Char('0') && c <= QLatin1Char('9'))
              || (c >= QLatin1Char('A') && c <= QLatin1Char('Z'))
              || (c >= QLatin1Char('a') && c <= QLatin1Char('z'))
              || c == QLatin1Char('-') || c == QLatin1Char('_')
              || c == QLatin1Char('.'))) {
            setError(QStringLiteral("ID de Registro Automático contém caracteres inválidos"));
            return false;
        }
    }
    for (const auto& record : records_) {
        if (record.auto_registration && qstring(record.registration_id) == cleanRegistrationId) {
            setError(QStringLiteral("ID de Registro Automático já cadastrado"));
            return false;
        }
    }

    const QString id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    Record record;
    record.auto_registration = true;
    record.device.access_method = sentrium::device::AccessMethod::AutoRegistration;
    record.device.id = utf8(id);
    record.device.display_name = utf8(displayName.trimmed());
    record.registration_id = utf8(cleanRegistrationId);
    record.username = utf8(username);
    record.password = utf8(password);
    record.device.state = sentrium::device::ConnectionState::Connecting;
    record.device.diagnostic = registration_listening_
        ? "Aguardando registro do dispositivo" : "Aguardando ativação do servidor";
    records_.push_back(std::move(record));
    setError({});
    emit devicesChanged();
    emit requestRegisterAutoDevice(id, cleanRegistrationId, username, password);
    emit auditEvent(QStringLiteral("auto-register-expected-device-added"),
                    QStringLiteral("Dispositivo aguardando registro, sem sessão simulada"));
    return true;
}

void DeviceController::startRegistrationServer(const QString& bindAddress, int port) {
    if (shutdown_ || port < 1 || port > 65535) {
        setError(QStringLiteral("Porta de Registro Automático inválida"));
        return;
    }
    registration_status_ = QStringLiteral("Iniciando serviço de registro...");
    emit registrationStatusChanged();
    emit requestStartRegistrationServer(bindAddress.trimmed(), static_cast<quint16>(port));
}

void DeviceController::stopRegistrationServer() {
    if (shutdown_) return;
    emit requestStopRegistrationServer();
}

void DeviceController::onRegistrationServerChanged(
    bool listening, const QString& diagnostic) {
    registration_listening_ = listening;
    registration_status_ = diagnostic;
    emit registrationStatusChanged();
    for (auto& record : records_) {
        if (record.auto_registration && !record.pending_remove
            && record.device.state == sentrium::device::ConnectionState::Connecting) {
            record.device.diagnostic = listening
                ? "Aguardando registro do dispositivo"
                : "Serviço de registro desativado";
        }
    }
    emit devicesChanged();
}

void DeviceController::onRegistrationObservation(const QString& message) {
    if (shutdown_) return;
    registration_last_event_ = message;
    if (registration_event_count_ < 1000000000) ++registration_event_count_;
    emit registrationEventsChanged();
}

void DeviceController::onRegistrationLost(const QString& id) {
    auto* record = find(id);
    if (!record || !record->auto_registration || record->pending_remove) return;
    record->device.state = sentrium::device::ConnectionState::Connecting;
    record->device.channels.clear();
    record->device.diagnostic = "Aguardando reconexão do gravador";
    emit devicesChanged();
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
    if (record->auto_registration) {
        record->device.diagnostic = "Aguardando registro do dispositivo";
        emit devicesChanged();
        emit requestRegisterAutoDevice(
            id, qstring(record->registration_id),
            qstring(record->username), qstring(record->password));
    } else {
        emit requestConnect(
            id,
            qstring(record->device.host),
            record->device.port,
            qstring(record->username),
            qstring(record->password));
    }
}

void DeviceController::disconnectDevice(const QString& id) {
    auto* record = find(id);
    if (!record) {
        setError(QStringLiteral("Device not found"));
        return;
    }

    if (record->device.state != sentrium::device::ConnectionState::Connected
        && record->device.state != sentrium::device::ConnectionState::Failed
        && !(record->auto_registration
             && record->device.state == sentrium::device::ConnectionState::Connecting)) {
        return;
    }

    record->device.state = sentrium::device::ConnectionState::Disconnecting;
    record->device.diagnostic.clear();
    emit devicesChanged();
    emit auditEvent(
        QStringLiteral("device-disconnect-requested"),
        QStringLiteral("Device disconnect requested"));
    if (record->auto_registration) {
        emit requestUnregisterAutoDevice(id);
    } else {
        emit requestDisconnect(id);
    }
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
        if (record->auto_registration) {
            disconnectDevice(id);
        }
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

    if (found->auto_registration) {
        emit requestUnregisterAutoDevice(id);
    }
    std::fill(found->registration_id.begin(), found->registration_id.end(), '\0');
    found->registration_id.clear();
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

    record->device.diagnostic = record->device.access_method
        == sentrium::device::AccessMethod::Cloud && success
        ? "NetSDK autenticado via TCP local; origem Cloud e rota P2P ainda nao verificadas"
        : utf8(diagnostic);
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
        std::fill(record.registration_id.begin(), record.registration_id.end(), '\0');
        record.registration_id.clear();
        std::fill(record.password.begin(), record.password.end(), '\0');
        record.password.clear();
        record.username.clear();
        record.device.channels.clear();
        record.device.state = sentrium::device::ConnectionState::Disconnected;
    }
}
