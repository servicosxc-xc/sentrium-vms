#pragma once

#include <sentrium/device.hpp>

#include <QObject>
#include <QPointer>
#include <QThread>
#include <QVariantList>
#include <QStringList>

#include <array>
#include <string>
#include <unordered_map>
#include <vector>

class DeviceWorker;
class QWindow;

class DeviceController final : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList devices READ devices NOTIFY devicesChanged)
    Q_PROPERTY(QStringList deviceGroups READ deviceGroups NOTIFY devicesChanged)
    Q_PROPERTY(QVariantList mosaicSlots READ mosaicSlots NOTIFY mosaicSlotsChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)
    Q_PROPERTY(bool registrationListening READ registrationListening NOTIFY registrationStatusChanged)
    Q_PROPERTY(QString registrationStatus READ registrationStatus NOTIFY registrationStatusChanged)
    Q_PROPERTY(QString registrationLastEvent READ registrationLastEvent NOTIFY registrationEventsChanged)
    Q_PROPERTY(int registrationEventCount READ registrationEventCount NOTIFY registrationEventsChanged)

public:
    explicit DeviceController(QObject* parent = nullptr);
    ~DeviceController() override;

    QVariantList devices() const;
    QStringList deviceGroups() const { return device_groups_; }
    QVariantList mosaicSlots() const;
    QString lastError() const { return last_error_; }
    bool registrationListening() const { return registration_listening_; }
    QString registrationStatus() const { return registration_status_; }
    QString registrationLastEvent() const { return registration_last_event_; }
    int registrationEventCount() const { return registration_event_count_; }

    Q_INVOKABLE void addAndConnect(
        const QString& displayName,
        const QString& host,
        int port,
        const QString& username,
        const QString& password);
    // Experimental bridge: a separate, operator-owned Cloud tunnel exposes a
    // local TCP endpoint. NetSDK authenticates through that endpoint.
    // This does NOT implement or manage the Cloud/P2P tunnel.
    Q_INVOKABLE void addCloudBridgeAndConnect(
        const QString& displayName,
        int localPort,
        const QString& username,
        const QString& password);

    Q_INVOKABLE bool addAutoRegistration(
        const QString& displayName, const QString& registrationId,
        const QString& username, const QString& password);
    Q_INVOKABLE void startRegistrationServer(const QString& bindAddress, int port);
    Q_INVOKABLE void stopRegistrationServer();
    Q_INVOKABLE void connectDevice(const QString& id);
    Q_INVOKABLE void disconnectDevice(const QString& id);
    // Native NetSDK preview embedded in Qt Quick WindowContainer mosaic.
    Q_INVOKABLE QWindow* liveSurface(int slot);
    Q_INVOKABLE void openLive(const QString& id, int channel, bool extra = true);
    Q_INVOKABLE void openLiveInSlot(
        const QString& id, int channel, int slot, bool extra = true);
    Q_INVOKABLE void clearLiveSlot(int slot);
    Q_INVOKABLE void swapLiveSlots(int sourceSlot, int targetSlot);
    Q_INVOKABLE void closeDeviceChannels(const QString& id);
    Q_INVOKABLE void clearAllLive();
    Q_INVOKABLE void setMosaicCapacity(int capacity);
    Q_INVOKABLE void removeDevice(const QString& id);
    // Operator-defined organizational metadata only; does not touch NetSDK.
    Q_INVOKABLE bool createDeviceGroup(const QString& name);
    Q_INVOKABLE bool setDeviceGroup(const QString& id, const QString& name);

    void shutdown();

signals:
    void devicesChanged();
    void mosaicSlotsChanged();
    void lastErrorChanged();
    void registrationStatusChanged();
    void registrationEventsChanged();
    void auditEvent(const QString& eventId, const QString& message);

    void requestConnect(
        const QString& id,
        const QString& host,
        quint16 port,
        const QString& username,
        const QString& password);
    void requestDisconnect(const QString& id);
    void requestStartLive(int slot, quint64 token, const QString& id,
                          int channel, quint64 nativeWindow, bool extra);
    void requestStopLive(int slot);
    void requestStartRegistrationServer(const QString& bindAddress, quint16 port);
    void requestStopRegistrationServer();
    void requestRegisterAutoDevice(
        const QString& id, const QString& registrationId,
        const QString& username, const QString& password);
    void requestUnregisterAutoDevice(const QString& id);

private slots:
    void onRegistrationServerChanged(bool listening, const QString& diagnostic);
    void onRegistrationLost(const QString& id);
    void onRegistrationObservation(const QString& message);
    void onConnectFinished(
        const QString& id,
        bool success,
        int channelCount,
        const QString& diagnostic);
    void onDisconnectFinished(
        const QString& id,
        bool success,
        const QString& diagnostic);
    void onLiveStarted(int slot, quint64 token, const QString& id,
                       bool success, const QString& diagnostic);

private:
    struct Record {
        sentrium::device::Device device;
        std::string username;
        std::string password;
        bool pending_remove = false;
        bool auto_registration = false;
        std::string registration_id;
        QString group_name;
    };

    Record* find(const QString& id);
    const Record* find(const QString& id) const;
    void setError(const QString& error);
    void erase(const QString& id);
    void clearLiveForDevice(const QString& id);

    struct ResumeSlot {
        int slot = -1;
        int channel = -1;
        bool extra = true;
    };
    std::unordered_map<std::string, std::vector<ResumeSlot>> resume_on_reconnect_;

    struct MosaicTile {
        QString deviceId;
        QString deviceName;
        QString channelName;
        QString status;
        int channel = -1;
        bool extra = true;
        quint64 token = 0;
    };

    QThread worker_thread_;
    static constexpr int max_mosaic_slots_ = 64;
    std::array<QPointer<QWindow>, max_mosaic_slots_> surfaces_{};
    std::array<MosaicTile, max_mosaic_slots_> tiles_{};
    int mosaic_capacity_ = 16;
    int next_slot_ = 0;
    quint64 preview_token_ = 0;
    DeviceWorker* worker_ = nullptr;
    std::vector<Record> records_;
    QStringList device_groups_;
    QString last_error_;
    QString registration_status_ = QStringLiteral("Servidor de registro desativado");
    QString registration_last_event_ = QStringLiteral("Nenhum registro recebido");
    int registration_event_count_ = 0;
    bool registration_listening_ = false;
    bool shutdown_ = false;
};
