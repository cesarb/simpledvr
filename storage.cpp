#include "storage.h"

#include <QStandardPaths>
#include <QStorageInfo>
#include <QTimerEvent>

StorageMonitor::StorageMonitor(QObject *parent)
    : QObject(parent), timerId(startTimer(1000, Qt::VeryCoarseTimer))
{
}

void StorageMonitor::timerEvent(QTimerEvent *event)
{
    if (event->timerId() != timerId)
        return;

    auto path = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation);
    auto available = (double)QStorageInfo(path).bytesAvailable() / (1024 * 1024 * 1024);
    emit storageAvailable(QString::number(available, 'f', 2) + " GiB");
}
