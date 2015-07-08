#ifndef RECORDINGTIMER_H
#define RECORDINGTIMER_H

#include <QDateTime>
#include <QObject>

class RecordingTimer : public QObject
{
    Q_OBJECT
public:
    explicit RecordingTimer(QObject *parent = nullptr);

signals:
    void timeRemaining(const QString &text);
    void stopRecording();

public slots:
    void addSecs(qint64 secs);
    void clear();

protected:
    virtual void timerEvent(QTimerEvent *event) Q_DECL_OVERRIDE;

private:
    void updateTimer();

    int timerId;
    QDateTime stopTime;
};

#endif // RECORDINGTIMER_H
