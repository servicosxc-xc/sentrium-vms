#pragma once

#include <sentrium/intelbras.hpp>

#include <QObject>
#include <QString>

#include <unordered_map>
#include <unordered_set>

class DeviceWorker final : public QObject {
    Q_OBJECT

public:
    explicit DeviceWorker(QObject* parent = nullptr);

public slots:
    void connectDevice(
        const QString& id,
        const QString& host,
        quint16 port,
        const QString& username,
        const QString& password);
    void disconnectDevice(const QString& id);
    void startLive(int slot, quint64 token, const QString& id,
                   int channel, quint64 nativeWindow, bool extra);
    void stopLive(int slot);
    void startRegistrationServer(const QString& bindAddress, quint16 port);
    void stopRegistrationServer();
    void registerAutoDevice(
        const QString& id, const QString& registrationId,
        const QString& username, const QString& password);
    void unregisterAutoDevice(const QString& id);
    void shutdown();

signals:
    void connectFinished(
        const QString& id,
        bool success,
        int channelCount,
        const QString& diagnostic);
    void liveStarted(int slot, quint64 token, const QString& id,
                     bool success, const QString& diagnostic);
    void registrationServerChanged(bool listening, const QString& diagnostic);
    void registrationReceived(
        const QString& registrationId, const QString& peer, quint16 port, bool disconnected);
    void registrationLost(const QString& id);
    // Sanitized operational observation. Never include registration ID, peer or credentials.
    void registrationObservation(const QString& message);
    void disconnectFinished(
        const QString& id,
        bool success,
        const QString& diagnostic);

private slots:
    void onRegistrationReceived(
        const QString& registrationId, const QString& peer, quint16 port, bool disconnected);

private:
    struct RegisteredTarget {
        QString registrationId;
        QString username;
        QString password;
    };

    bool listening_ = false;
    std::unordered_map<std::string, RegisteredTarget> registered_;
    sentrium::intelbras::Adapter adapter_;
    struct LiveEntry {
        QString deviceId;
        sentrium::device::Session handle;
    };
    std::unordered_map<int, LiveEntry> live_sessions_;
    void stopLiveForDevice(const QString& id);
    void stopAllLive();
    std::unordered_map<std::string, sentrium::device::Session> sessions_;
    // A failed logout is retried before accepting a new registration.
    std::unordered_set<std::string> stale_registration_sessions_;
};
