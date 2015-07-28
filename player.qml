import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.1
import QtQuick.Window 2.2
import QtGStreamer 1.0

Window {
    id: window
    title: "Simple DVR"

    visible: true
    //visibility: "Maximized"
    visibility: "FullScreen"

    width: 640
    height: 480

    property real controlZ: 1
    property real controlOpacity: 0.25

    property real scheduleZ: 1
    property real scheduleOpacity: 0.75
    property bool scheduleVisible: false

    VideoItem {
        id: video
        anchors.fill: parent
        surface: videoSurface
    }

    ColumnLayout {
        id: leftColumn
        anchors.top: parent.top
        anchors.left: parent.left
        z: controlZ
        opacity: controlOpacity

        Button {
            text: "Record"
            onClicked: pipeline.startRecording()
        }

        Button {
            text: "Schedule"
            onClicked: scheduleVisible = !scheduleVisible
        }

        Button {
            text: "+"
            onClicked: recordingTimer.addSecs(60)
        }

        Text {
            id: timeRemaining
            color: "#ffffff"
            style: Text.Outline
            styleColor: "#000000"
            horizontalAlignment: Text.AlignLeft
            textFormat: Text.PlainText

            Connections {
                target: recordingTimer
                onTimeRemaining: timeRemaining.text = text
            }
        }

        Button {
            text: "-"
            onClicked: recordingTimer.addSecs(-60)
        }

        Text {
            id: status
            color: "#ffffff"
            style: Text.Outline
            styleColor: "#000000"
            horizontalAlignment: Text.AlignLeft
            textFormat: Text.PlainText

            Connections {
                target: pipeline
                onRecordingStarting: status.text = "Starting"
                onRecordingStarted: status.text = "Recording"
                onRecordingStopping: status.text = "Stopping"
                onRecordingStopped: status.text = ""
            }
        }

        Button {
            text: "Stop"
            onClicked: pipeline.stopRecording()
        }
    }

    ColumnLayout {
        id: rightColumn
        anchors.top: parent.top
        anchors.right: parent.right
        z: controlZ
        opacity: controlOpacity

        Button {
            text: "Close"
            onClicked: window.close()
        }

        Text {
            id: available
            color: "#ffffff"
            style: Text.Outline
            styleColor: "#000000"
            horizontalAlignment: Text.AlignRight
            textFormat: Text.PlainText

            Connections {
                target: storageMonitor
                onStorageAvailable: available.text = text
            }
        }

        Button {
            text: "Full Screen"
            onClicked: if (window.visibility == Window.FullScreen) { window.showMaximized() } else { window.showFullScreen() }
        }
    }

    ColumnLayout {
        id: centerColumn
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        z: controlZ
        opacity: controlOpacity

        Text {
            id: dateTime
            color: "#ffffff"
            style: Text.Outline
            styleColor: "#000000"
            horizontalAlignment: Text.AlignCenter
            textFormat: Text.PlainText

            Timer {
                interval: 500
                repeat: true
                running: true
                triggeredOnStart: true

                onTriggered: parent.text = new Date().toLocaleString()
            }
        }
    }

    ListView {
        id: scheduleList
        anchors.top: centerColumn.bottom
        anchors.left: leftColumn.right
        anchors.right: rightColumn.left
        anchors.bottom: scheduleForm.top
        z: scheduleZ
        opacity: scheduleOpacity
        visible: scheduleVisible
        enabled: scheduleVisible

        model: recordingScheduler
        delegate: Row {
            spacing: 5

            Text {
                anchors.verticalCenter: parent.verticalCenter
                text: display
                color: "#ffffff"
                style: Text.Outline
                styleColor: "#000000"
                horizontalAlignment: Text.AlignLeft
                textFormat: Text.PlainText
            }

            Button {
                anchors.verticalCenter: parent.verticalCenter
                text: "Remove"
                onClicked: recordingScheduler.removeSchedule(index)
            }
        }
    }

    ScheduleForm {
        id: scheduleForm
        anchors.left: leftColumn.right
        anchors.right: rightColumn.left
        anchors.bottom: parent.bottom
        z: scheduleZ
        opacity: scheduleOpacity
        visible: scheduleVisible
        enabled: scheduleVisible

        addButton.onClicked: addSchedule()

        function addSchedule() {
            var start = new Date();
            start.setTime(startDate.getTime() + startHour * 60 * 60 * 1000 + startMinute * 60 * 1000);
            var stop = new Date();
            stop.setTime(start.getTime() + durationHours * 60 * 60 * 1000 + durationMinutes * 60 * 1000);
            recordingScheduler.addSchedule(start, stop);
        }
    }
}
