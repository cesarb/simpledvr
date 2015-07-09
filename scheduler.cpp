#include "scheduler.h"

ScheduledRecording::ScheduledRecording(QObject *parent) : QObject(parent)
{
    timer.setTimerType(Qt::VeryCoarseTimer);
    connect(&timer, &QTimer::timeout, this, &ScheduledRecording::timeout);
}

void ScheduledRecording::updateTimer()
{
    timer.stop();

    auto remaining = QDateTime::currentDateTime().msecsTo(startTime);
    if (remaining > (24 * 60 * 60 * 1000))
        remaining = (24 * 60 * 60 * 1000);

    if (remaining >= 0)
        timer.start(remaining);
}

void ScheduledRecording::timeout()
{
    auto remaining = QDateTime::currentDateTime().msecsTo(startTime);
    if (remaining > 1000) {
        updateTimer();
        return;
    }

    emit startRecordingUntil(stopTime);
}

RecordingScheduler::RecordingScheduler(QObject *parent) : QAbstractListModel(parent)
{
    load();
}

QHash<int, QByteArray> RecordingScheduler::roleNames() const
{
    auto roles = QAbstractListModel::roleNames();
    roles.insert(StartTimeRole, "startTime");
    roles.insert(StopTimeRole, "stopTime");
    return roles;
}

int RecordingScheduler::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return items.size();
}

Qt::ItemFlags RecordingScheduler::flags(const QModelIndex & index) const
{
    return QAbstractListModel::flags(index) | Qt::ItemIsEditable;
}

QVariant RecordingScheduler::data(const QModelIndex &index, int role) const
{
    auto row = index.row();
    if (index.parent().isValid() || index.column() != 0 || row < 0 || row >= items.size())
        return QVariant();

    auto item = items.at(row);

    switch (role) {
    case Qt::DisplayRole:
    {
        auto startTime = item->getStartTime().toLocalTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
        auto stopTime = item->getStopTime().toLocalTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
        return startTime + QStringLiteral(" - ") + stopTime;
    }
    case StartTimeRole:
        return item->getStartTime();
    case StopTimeRole:
        return item->getStopTime();
    default:
        return QVariant();
    }
}

bool RecordingScheduler::setData(const QModelIndex &index, const QVariant &value, int role)
{
    auto row = index.row();
    if (index.parent().isValid() || index.column() != 0 || row < 0 || row >= items.size())
        return false;

    auto item = items.at(row);

    switch (role) {
    case StartTimeRole:
        item->setStartTime(value.toDateTime());
        break;
    case StopTimeRole:
        item->setStopTime(value.toDateTime());
        break;
    default:
        return false;
    }

    emit dataChanged(index, index);
    save();
    return true;
}

ScheduledRecording *RecordingScheduler::insertChild()
{
    auto child = new ScheduledRecording(this);
    connect(child, &ScheduledRecording::destroyed, this, &RecordingScheduler::removeChild);
    connect(child, &ScheduledRecording::startRecordingUntil, this, &RecordingScheduler::removeChild);
    connect(child, &ScheduledRecording::startRecordingUntil, this, &RecordingScheduler::startRecordingUntil);

    auto index = items.size();
    beginInsertRows(QModelIndex(), index, index);
    items.append(child);
    endInsertRows();

    return child;
}

void RecordingScheduler::removeChild()
{
    auto index = items.indexOf(static_cast<ScheduledRecording *>(sender()));
    if (index < 0)
        return;

    beginRemoveRows(QModelIndex(), index, index);
    items.takeAt(index)->deleteLater();
    endRemoveRows();

    save();
}

void RecordingScheduler::load()
{

}

void RecordingScheduler::save()
{

}
