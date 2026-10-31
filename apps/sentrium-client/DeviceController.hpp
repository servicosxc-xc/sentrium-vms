#pragma once

#include <sentrium/device.hpp>

#include <QObject>
#include <QPointer>
#include <QThread>
#include <QVariantList>

#include <string>
#include <vector>

class DeviceWorker;
class QWindow;

class DeviceController final : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList devices READ devices NOTIFY devicesChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)
    Q_PROPERTY(bool registrationListening READ registrationListening NOTIFY registrationStatusChanged)
    Q_PROPERTY(QString registrationStatus READ registrationStatus NOTIFY registrationStatusChanged)
    Q_PROPERTY(QString registrationLastEvent READ registrationLastEvent NOTIFY registrationEventsChanged)
    Q_PROPERTY(int registrationEventCount READ registrationEventCount NOTIFY registrationEventsChanged)

public:
    explicit DeviceController(QObject* parent = nullptr);
    ~DeviceController() override;

    QVariantList devices() const;
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
    // First native video proof: one preview in a separate OS window.
    Q_INVOKABLE void openLive(const QString& id, int channel, bool extra = true);
    Q_INVOKABLE void removeDevice(const QString& id);

    void shutdown();

signals:
    void devicesChanged();
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
    void requestStartLive(const QString& id, int channel, quint64 nativeWindow, bool extra);
    void requestStopLive(const QString& id);
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
    void onLiveStarted(const QString& id, bool success, const QString& diagnostic);

private:
    struct Record {
        sentrium::device::Device device;
        std::string username;
        std::string password;
        bool pending_remove = false;
        bool auto_registration = false;
        std::string registration_id;
    };

    Record* find(const QString& id);
    const Record* find(const QString& id) const;
    void setError(const QString& error);
    void erase(const QString& id);

    QThread worker_thread_;
    QPointer<QWindow> live_window_;
    QString live_device_id_;
    DeviceWorker* worker_ = nullptr;
    std::vector<Record> records_;
    QString last_error_;
    QString registration_status_ = QStringLiteral("Servidor de registro desativado");
    QString registration_last_event_ = QStringLiteral("Nenhum registro recebido");
    int registration_event_count_ = 0;
    bool registration_listening_ = false;
    bool shutdown_ = false;
};
