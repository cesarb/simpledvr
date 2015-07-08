#include "timer.h"

#include <QTimerEvent>

RecordingTimer::RecordingTimer(QObject *parent)
    : QObject(parent), timerId(startTimer(1000, Qt::VeryCoarseTimer))
{
}

void RecordingTimer::addSecs(qint64 secs)
{
    if (!stopTime.isValid())
        stopTime = QDateTime::currentDateTime();

    stopTime = stopTime.addSecs(secs);

    updateTimer();
}

void RecordingTimer::updateTimer()
{
    auto remaining = QDateTime::currentDateTime().secsTo(stopTime);
    if (remaining <= 0) {
        stopTime = QDateTime();
        emit timeRemaining(QString());
        return;
    }

    emit timeRemaining(QStringLiteral("%1:%2").arg(remaining / 60, 2, 10, QLatin1Char('0')).arg(remaining % 60, 2, 10, QLatin1Char('0')));
}

void RecordingTimer::timerEvent(QTimerEvent *event)
{
    if (event->timerId() != timerId)
        return;

    if (!stopTime.isValid())
        return;

    updateTimer();
    if (!stopTime.isValid())
        emit stopRecording();
}
