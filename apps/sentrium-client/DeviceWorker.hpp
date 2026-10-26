#pragma once

#include <sentrium/intelbras.hpp>

#include <QObject>
#include <QString>

#include <unordered_map>

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
    std::unordered_map<std::string, sentrium::device::Session> sessions_;
};
