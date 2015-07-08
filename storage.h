#ifndef STORAGEMONITOR_H
#define STORAGEMONITOR_H

#include <QObject>

class StorageMonitor : public QObject
{
    Q_OBJECT
public:
    explicit StorageMonitor(QObject *parent = nullptr);

signals:
    void storageAvailable(const QString &text);

public slots:

protected:
    virtual void timerEvent(QTimerEvent *event) Q_DECL_OVERRIDE;

private:
    int timerId;
};

#endif // STORAGEMONITOR_H
