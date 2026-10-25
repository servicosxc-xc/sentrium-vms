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
    void shutdown();

signals:
    void connectFinished(
        const QString& id,
        bool success,
        int channelCount,
        const QString& diagnostic);
    void disconnectFinished(
        const QString& id,
        bool success,
        const QString& diagnostic);

private:
    sentrium::intelbras::Adapter adapter_;
    std::unordered_map<std::string, sentrium::device::Session> sessions_;
};
