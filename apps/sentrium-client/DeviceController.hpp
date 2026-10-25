#pragma once

#include <sentrium/device.hpp>

#include <QObject>
#include <QThread>
#include <QVariantList>

#include <string>
#include <vector>

class DeviceWorker;

class DeviceController final : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList devices READ devices NOTIFY devicesChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)

public:
    explicit DeviceController(QObject* parent = nullptr);
    ~DeviceController() override;

    QVariantList devices() const;
    QString lastError() const { return last_error_; }

    Q_INVOKABLE void addAndConnect(
        const QString& displayName,
        const QString& host,
        int port,
        const QString& username,
        const QString& password);
    Q_INVOKABLE void connectDevice(const QString& id);
    Q_INVOKABLE void disconnectDevice(const QString& id);
    Q_INVOKABLE void removeDevice(const QString& id);

    void shutdown();

signals:
    void devicesChanged();
    void lastErrorChanged();
    void auditEvent(const QString& eventId, const QString& message);

    void requestConnect(
        const QString& id,
        const QString& host,
        quint16 port,
        const QString& username,
        const QString& password);
    void requestDisconnect(const QString& id);

private slots:
    void onConnectFinished(
        const QString& id,
        bool success,
        int channelCount,
        const QString& diagnostic);
    void onDisconnectFinished(
        const QString& id,
        bool success,
        const QString& diagnostic);

private:
    struct Record {
        sentrium::device::Device device;
        std::string username;
        std::string password;
        bool pending_remove = false;
    };

    Record* find(const QString& id);
    const Record* find(const QString& id) const;
    void setError(const QString& error);
    void erase(const QString& id);

    QThread worker_thread_;
    DeviceWorker* worker_ = nullptr;
    std::vector<Record> records_;
    QString last_error_;
    bool shutdown_ = false;
};
