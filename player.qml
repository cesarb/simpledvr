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
        z: scheduleZ
        opacity: scheduleOpacity

        model: recordingScheduler
        delegate: Text {
            text: display
            color: "#ffffff"
            style: Text.Outline
            styleColor: "#000000"
            horizontalAlignment: Text.AlignLeft
            textFormat: Text.PlainText
        }
    }
}
