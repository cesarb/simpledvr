#ifndef RECORDINGSCHEDULER_H
#define RECORDINGSCHEDULER_H

#include <QAbstractListModel>
#include <QDateTime>
#include <QList>
#include <QObject>
#include <QTimer>

class ScheduledRecording : public QObject
{
    Q_OBJECT
public:
    explicit ScheduledRecording(QObject *parent = nullptr);

    QDateTime getStartTime() const { return startTime; }
    QDateTime getStopTime() const { return stopTime; }

    void setStartTime(const QDateTime &startTime) { this->startTime = startTime; updateTimer(); }
    void setStopTime(const QDateTime &stopTime) { this->stopTime = stopTime; }

signals:
    void startRecordingUntil(const QDateTime &stopTime);

public slots:

private:
    void updateTimer();
    void timeout();

    QDateTime startTime, stopTime;
    QTimer timer;
};

class RecordingScheduler : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit RecordingScheduler(QObject *parent = nullptr);

    enum Roles {
        StartTimeRole = Qt::UserRole + 1,
        StopTimeRole,
    };

    virtual QHash<int, QByteArray> roleNames() const Q_DECL_OVERRIDE;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    virtual Qt::ItemFlags flags(const QModelIndex & index) const Q_DECL_OVERRIDE;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) Q_DECL_OVERRIDE;

signals:
    void startRecordingUntil(const QDateTime &stopTime);

public slots:

private:
    ScheduledRecording *insertChild();
    void removeChild();

    void load();
    void save();

    QList<ScheduledRecording *> items;
};

#endif // RECORDINGSCHEDULER_H
