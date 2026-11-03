#include "DeviceController.hpp"
#include "DeviceWorker.hpp"

#include <Windows.h>

#include <QByteArray>
#include <QCoreApplication>
#include <QCursor>
#include <QMetaObject>
#include <QQmlEngine>
#include <QColor>
#include <QWindow>
#include <QUuid>
#include <QVariantMap>

#include <algorithm>
#include <stdexcept>
#include <utility>

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
    // Observe mouse input on NetSDK native preview HWNDs. Ordinary QML
    // MouseArea and DropArea cannot reliably sit over WindowContainer.
    if (auto* app = QCoreApplication::instance())
        app->installNativeEventFilter(this);
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
    connect(this, &DeviceController::requestStartLive,
            worker_, &DeviceWorker::startLive, Qt::QueuedConnection);
    connect(this, &DeviceController::requestStopLive,
            worker_, &DeviceWorker::stopLive, Qt::QueuedConnection);
    connect(worker_, &DeviceWorker::liveStarted,
            this, &DeviceController::onLiveStarted, Qt::QueuedConnection);
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
    if (auto* app = QCoreApplication::instance())
        app->removeNativeEventFilter(this);
    shutdown();
    // Controller outlives the QQmlApplicationEngine in main.cpp.
    for (auto& surface : surfaces_) {
        if (surface) delete surface.data();
    }
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
        item.insert(QStringLiteral("group"), record.group_name);

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

QVariantList DeviceController::mosaicSlots() const {
    QVariantList result;
    result.reserve(max_mosaic_slots_);
    for (int slot = 0; slot < max_mosaic_slots_; ++slot) {
        const auto& tile = tiles_[static_cast<std::size_t>(slot)];
        QVariantMap item;
        item.insert(QStringLiteral("slot"), slot);
        item.insert(QStringLiteral("deviceId"), tile.deviceId);
        item.insert(QStringLiteral("channel"), tile.channel);
        item.insert(QStringLiteral("occupied"), !tile.deviceId.isEmpty());
        item.insert(QStringLiteral("deviceName"), tile.deviceName);
        item.insert(QStringLiteral("channelName"), tile.channelName);
        item.insert(QStringLiteral("status"), tile.status);
        item.insert(QStringLiteral("extra"), tile.extra);
        // IA has no verified server-side rule engine in Foundation 005.
        // Keep an explicit unknown/unavailable state rather than inferring
        // "active" from a connected stream or a QML-only preference.
        item.insert(QStringLiteral("aiState"), QStringLiteral("Unavailable"));
        item.insert(QStringLiteral("aiMode"), QString());

        result.push_back(item);
    }
    return result;
}

bool DeviceController::nativeEventFilter(
    const QByteArray& eventType, void* message, qintptr*) {
    if (shutdown_ || eventType != QByteArrayLiteral("windows_generic_MSG")
        || message == nullptr)
        return false;

    const auto* msg = static_cast<const MSG*>(message);
    if (msg->message != WM_RBUTTONUP && msg->message != WM_LBUTTONDOWN)
        return false;

    // Never read credentials, manipulate NetSDK handles or swallow native
    // messages. Match only a QWindow created for one mosaic tile.
    for (int slot = 0; slot < max_mosaic_slots_; ++slot) {
        const auto& surface = surfaces_[static_cast<std::size_t>(slot)];
        if (!surface) continue;
        const HWND nativeVideoWindow = reinterpret_cast<HWND>(surface->winId());
        if (nativeVideoWindow != msg->hwnd
            && !IsChild(nativeVideoWindow, msg->hwnd))
            continue;

        if (msg->message == WM_RBUTTONUP) {
            const QPoint screen = QCursor::pos();
            QMetaObject::invokeMethod(this, [this, slot, screen] {
                if (!shutdown_)
                    emit videoSurfaceContextRequested(slot, screen.x(), screen.y());
            }, Qt::QueuedConnection);
        } else if (msg->message == WM_LBUTTONDOWN) {
            QMetaObject::invokeMethod(this, [this, slot] {
                if (!shutdown_) emit videoSurfaceLeftPressed(slot);
            }, Qt::QueuedConnection);
        }
        break;
    }
    return false;
}

bool DeviceController::isLeftMouseButtonDown() const {
    return (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
}

QPoint DeviceController::cursorScreenPos() const {
    return QCursor::pos();
}

QWindow* DeviceController::liveSurface(int slot) {
    if (slot < 0 || slot >= max_mosaic_slots_ || shutdown_) return nullptr;
    auto& surface = surfaces_[static_cast<std::size_t>(slot)];
    if (!surface) {
        auto* window = new QWindow;
        window->setFlags(Qt::FramelessWindowHint);
        window->create();
        QQmlEngine::setObjectOwnership(window, QQmlEngine::CppOwnership);
        surface = window;
    }
    return surface.data();
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
    if (!listening) resume_on_reconnect_.clear();
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
    const auto key = utf8(id);
    // Capture the existing visual layout once, before clearing the dead
    // real-play handles. Repeated SDK disconnect notifications must not
    // accidentally replace the original desired layout with an empty one.
    if (!resume_on_reconnect_.contains(key)) {
        std::vector<ResumeSlot> snapshots;
        for (int slot = 0; slot < mosaic_capacity_; ++slot) {
            const auto& tile = tiles_[static_cast<std::size_t>(slot)];
            if (tile.deviceId == id && tile.channel >= 0)
                snapshots.push_back(ResumeSlot{slot, tile.channel, tile.extra});
        }
        resume_on_reconnect_.emplace(key, std::move(snapshots));
    }
    clearLiveForDevice(id);
    record->device.state = sentrium::device::ConnectionState::Connecting;
    record->device.channels.clear();
    record->device.diagnostic = "Aguardando reconexão automática do gravador";
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

void DeviceController::openLive(const QString& id, int channel, bool extra) {
    // Fill the next free tile; when full, replace the selected round-robin tile.
    int slot = -1;
    for (int offset = 0; offset < mosaic_capacity_; ++offset) {
        const int candidate = (next_slot_ + offset) % mosaic_capacity_;
        if (tiles_[static_cast<std::size_t>(candidate)].deviceId.isEmpty()) {
            slot = candidate;
            break;
        }
    }
    if (slot < 0) slot = next_slot_ % mosaic_capacity_;
    next_slot_ = (slot + 1) % mosaic_capacity_;
    openLiveInSlot(id, channel, slot, extra);
}

void DeviceController::openLiveInSlot(
    const QString& id, int channel, int slot, bool extra) {
    if (shutdown_ || slot < 0 || slot >= mosaic_capacity_) return;
    const auto* record = find(id);
    if (!record || record->pending_remove
        || record->device.state != sentrium::device::ConnectionState::Connected
        || channel < 0
        || static_cast<std::size_t>(channel) >= record->device.channels.size()) {
        setError(QStringLiteral("Canal não conectado; aguarde o Registro Automático"));
        return;
    }
    QWindow* surface = liveSurface(slot);
    if (!surface) {
        setError(QStringLiteral("Falha ao criar a superfície nativa de vídeo"));
        return;
    }
    // The tile QWindow persists during session changes, so StopRealPlayEx is
    // always processed on the worker before the native HWND can be destroyed.
    emit requestStopLive(slot);
    auto& tile = tiles_[static_cast<std::size_t>(slot)];
    tile.deviceId = id;
    tile.deviceName = qstring(record->device.display_name);
    tile.channelName =
        qstring(record->device.channels[static_cast<std::size_t>(channel)].name);
    tile.channel = channel;
    tile.extra = extra;
    tile.status = QStringLiteral("Conectando vídeo...");
    tile.token = ++preview_token_;
    setError({});
    emit mosaicSlotsChanged();
    emit requestStartLive(slot, tile.token, id, channel,
                          static_cast<quint64>(surface->winId()), extra);
    emit auditEvent(QStringLiteral("mosaic-preview-requested"),
                    QStringLiteral("NetSDK RealPlay requested for an embedded tile"));
}

void DeviceController::swapLiveSlots(int sourceSlot, int targetSlot) {
    // Native render windows remain owned by physical slots. Switching positions
    // reopens streams against the destination HWND; it never swaps QWindow
    // ownership or bypasses the SDK's worker-side stop/start ordering.
    if (shutdown_ || sourceSlot < 0 || targetSlot < 0
        || sourceSlot >= mosaic_capacity_ || targetSlot >= mosaic_capacity_
        || sourceSlot == targetSlot) {
        return;
    }
    const MosaicTile source = tiles_[static_cast<std::size_t>(sourceSlot)];
    const MosaicTile destination = tiles_[static_cast<std::size_t>(targetSlot)];
    if (source.deviceId.isEmpty()) return;

    auto validTile = [this](const MosaicTile& tile) {
        if (tile.deviceId.isEmpty()) return true;
        const auto* record = find(tile.deviceId);
        return record && !record->pending_remove
            && record->device.state == sentrium::device::ConnectionState::Connected
            && tile.channel >= 0
            && static_cast<std::size_t>(tile.channel) < record->device.channels.size();
    };
    if (!validTile(source) || !validTile(destination)) {
        setError(QStringLiteral("Não foi possível mover: canal desconectado"));
        return;
    }

    openLiveInSlot(source.deviceId, source.channel, targetSlot, source.extra);
    if (destination.deviceId.isEmpty()) {
        clearLiveSlot(sourceSlot);
    } else {
        openLiveInSlot(destination.deviceId, destination.channel,
                       sourceSlot, destination.extra);
    }
    emit auditEvent(QStringLiteral("mosaic-slots-swapped"),
                    QStringLiteral("Operator moved live tiles within the mosaic"));
}

void DeviceController::clearLiveSlot(int slot) {
    if (slot < 0 || slot >= max_mosaic_slots_) return;
    auto& tile = tiles_[static_cast<std::size_t>(slot)];
    emit requestStopLive(slot);
    tile = {};
    emit mosaicSlotsChanged();
}

void DeviceController::closeDeviceChannels(const QString& id) {
    if (shutdown_ || !find(id)) return;
    clearLiveForDevice(id);
}

void DeviceController::clearAllLive() {
    for (int slot = 0; slot < max_mosaic_slots_; ++slot) {
        emit requestStopLive(slot);
        tiles_[static_cast<std::size_t>(slot)] = {};
    }
    emit mosaicSlotsChanged();
}

void DeviceController::clearLiveForDevice(const QString& id) {
    for (int slot = 0; slot < max_mosaic_slots_; ++slot) {
        if (tiles_[static_cast<std::size_t>(slot)].deviceId == id)
            clearLiveSlot(slot);
    }
}

void DeviceController::setMosaicCapacity(int capacity) {
    // Standard grids (1..64) and r25-style featured-tile variants.
    // This changes available native preview slots only; channel limits and
    // actual decode performance are established by device and Windows tests.
    static constexpr std::array<int, 13> supported{
        1, 4, 6, 8, 9, 10, 12, 14, 16, 25, 36, 49, 64
    };
    if (std::find(supported.begin(), supported.end(), capacity) == supported.end()) {
        setError(QStringLiteral("Layout de mosaico inválido"));
        return;
    }
    if (capacity == mosaic_capacity_) return;
    for (int slot = capacity; slot < max_mosaic_slots_; ++slot)
        clearLiveSlot(slot);
    mosaic_capacity_ = capacity;
    next_slot_ %= capacity;
    emit mosaicSlotsChanged();
}

void DeviceController::onLiveStarted(
    int slot, quint64 token, const QString& id,
    bool success, const QString& diagnostic) {
    if (shutdown_ || slot < 0 || slot >= max_mosaic_slots_) return;
    auto& tile = tiles_[static_cast<std::size_t>(slot)];
    if (tile.deviceId != id || tile.token != token) return;
    if (!success) {
        tile.status = QStringLiteral("Erro ao abrir vídeo");
        setError(diagnostic);
        emit auditEvent(QStringLiteral("mosaic-preview-failed"),
                        QStringLiteral("NetSDK RealPlayEx failed"));
    } else {
        // An SDK handle is not proof of visible motion; validate in the GUI.
        tile.status = QStringLiteral("Reprodução solicitada");
        emit auditEvent(QStringLiteral("mosaic-preview-handle-started"),
                        QStringLiteral("NetSDK RealPlayEx handle created"));
    }
    emit mosaicSlotsChanged();
}

void DeviceController::disconnectDevice(const QString& id) {
    resume_on_reconnect_.erase(utf8(id));
    clearLiveForDevice(id);
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

bool DeviceController::createDeviceGroup(const QString& name) {
    if (shutdown_) return false;
    const QString normalized = name.trimmed();
    if (normalized.isEmpty() || normalized.size() > 80
        || normalized.compare(QStringLiteral("Sem grupo"), Qt::CaseInsensitive) == 0) {
        setError(QStringLiteral("Nome de grupo inválido (máximo 80 caracteres)"));
        return false;
    }
    for (const QChar character : normalized) {
        if (!character.isPrint() || character == QLatin1Char('/')
            || character == QLatin1Char('\\')) {
            setError(QStringLiteral("Nome de grupo contém caracteres inválidos"));
            return false;
        }
    }
    for (const QString& existing : device_groups_) {
        if (existing.compare(normalized, Qt::CaseInsensitive) == 0) {
            setError(QStringLiteral("Grupo já cadastrado"));
            return false;
        }
    }
    device_groups_.append(normalized);
    setError({});
    emit devicesChanged();
    emit auditEvent(QStringLiteral("device-group-added"),
                    QStringLiteral("Organizational group created in memory"));
    return true;
}

bool DeviceController::setDeviceGroup(const QString& id, const QString& name) {
    if (shutdown_) return false;
    Record* record = find(id);
    if (!record) {
        setError(QStringLiteral("Dispositivo não encontrado"));
        return false;
    }
    const QString normalized = name.trimmed();
    QString target;
    if (!normalized.isEmpty()) {
        for (const QString& group : device_groups_) {
            if (group.compare(normalized, Qt::CaseInsensitive) == 0) {
                target = group;
                break;
            }
        }
        if (target.isEmpty()) {
            setError(QStringLiteral("Grupo não cadastrado"));
            return false;
        }
    }
    if (record->group_name == target) return true;
    record->group_name = target;
    setError({});
    emit devicesChanged();
    emit auditEvent(QStringLiteral("device-group-changed"),
                    QStringLiteral("Device organization updated in memory"));
    return true;
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
    clearLiveForDevice(id);
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

    // An asynchronous SDK callback may race with a user-initiated manual
    // disconnect. Never silently resurrect a deliberately stopped recorder.
    if (record->auto_registration
        && (record->device.state == sentrium::device::ConnectionState::Disconnected
            || record->device.state == sentrium::device::ConnectionState::Disconnecting)) {
        if (success) emit requestDisconnect(id);
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
        record->device.state = record->auto_registration
            ? sentrium::device::ConnectionState::Connecting
            : sentrium::device::ConnectionState::Failed;
        emit auditEvent(
            QStringLiteral("device-connect-failed"),
            QStringLiteral("Device connection failed"));
    }

    const bool remove_after = record->pending_remove;
    emit devicesChanged();

    if (success && !remove_after && record->auto_registration) {
        const auto key = utf8(id);
        const auto pending = resume_on_reconnect_.find(key);
        if (pending != resume_on_reconnect_.end()) {
            const auto previews = std::move(pending->second);
            resume_on_reconnect_.erase(pending);
            for (const auto& preview : previews) {
                if (preview.slot >= 0 && preview.slot < mosaic_capacity_
                    && preview.channel >= 0 && preview.channel < channelCount
                    && tiles_[static_cast<std::size_t>(preview.slot)].deviceId.isEmpty()) {
                    openLiveInSlot(id, preview.channel, preview.slot, preview.extra);
                }
            }
        }
    }

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
        clearLiveForDevice(id);
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
    resume_on_reconnect_.clear();
    if (shutdown_) {
        return;
    }
    shutdown_ = true;
    clearAllLive();

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
