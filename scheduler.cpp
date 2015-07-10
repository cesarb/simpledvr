#include "scheduler.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QStandardPaths>

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

void ScheduledRecording::read(const QJsonObject &json)
{
    setStartTime(QDateTime::fromString(json.value("start").toString(), Qt::ISODate));
    setStopTime(QDateTime::fromString(json.value("stop").toString(), Qt::ISODate));
}

void ScheduledRecording::write(QJsonObject &json) const
{
    json.insert("start", getStartTime().toString(Qt::ISODate));
    json.insert("stop", getStopTime().toString(Qt::ISODate));
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

ScheduledRecording *RecordingScheduler::newChild()
{
    auto child = new ScheduledRecording(this);
    connect(child, &ScheduledRecording::destroyed, this, &RecordingScheduler::removeChild);
    connect(child, &ScheduledRecording::startRecordingUntil, this, &RecordingScheduler::removeChild);
    connect(child, &ScheduledRecording::startRecordingUntil, this, &RecordingScheduler::startRecordingUntil);
    return child;
}

void RecordingScheduler::addSchedule(const QDateTime &startTime, const QDateTime &stopTime)
{
    auto child = newChild();
    child->setStartTime(startTime);
    child->setStopTime(stopTime);

    auto row = items.size();
    beginInsertRows(QModelIndex(), row, row);
    items.append(child);
    endInsertRows();
    save();
}

void RecordingScheduler::removeSchedule(int row)
{
    if (row < 0 || row >= items.size())
        return;

    beginRemoveRows(QModelIndex(), row, row);
    items.takeAt(row)->deleteLater();
    endRemoveRows();
    save();
}

void RecordingScheduler::removeChild()
{
    auto row = items.indexOf(static_cast<ScheduledRecording *>(sender()));
    removeSchedule(row);
}

QString RecordingScheduler::scheduleFileName()
{
    auto path = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation);
    return path + QStringLiteral("/simpledvr.json");
}

void RecordingScheduler::load()
{
    QFile file(scheduleFileName());
    if (!file.open(QFile::ReadOnly))
        return;

    auto jsonDocument = QJsonDocument::fromJson(file.readAll());
    auto jsonItems = jsonDocument.array();
    for (auto jsonItem : jsonItems) {
        auto child = newChild();
        child->read(jsonItem.toObject());
        items.append(child);
    }
}

void RecordingScheduler::save()
{
    QFile file(scheduleFileName());
    if (items.isEmpty()) {
        file.remove();
        return;
    }

    QJsonArray jsonItems;
    for (auto item : items) {
        QJsonObject jsonItem;
        item->write(jsonItem);
        jsonItems.append(jsonItem);
    }

    QJsonDocument jsonDocument(jsonItems);
    file.open(QFile::WriteOnly);
    file.write(jsonDocument.toJson());
    file.close();
}
