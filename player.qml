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

    VideoItem {
        id: video
        anchors.fill: parent
        surface: videoSurface
    }

    ColumnLayout {
        id: leftColumn
        anchors.top: parent.top
        anchors.left: parent.left
        z: 1
        opacity: 0.25

        Button {
            text: "Record"
            onClicked: pipeline.startRecording()
        }

        Text {
            id: status
            text: "Stopped"
            horizontalAlignment: Text.AlignLeft
            textFormat: Text.PlainText

            Connections {
                target: pipeline
                onRecordingStarting: status.text = "Starting"
                onRecordingStarted: status.text = "Started"
                onRecordingStopping: status.text = "Stopping"
                onRecordingStopped: status.text = "Stopped"
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
        z: 1
        opacity: 0.25

        Button {
            text: "Close"
            onClicked: window.close()
        }

        Text {
            id: available
            horizontalAlignment: Text.AlignRight
            textFormat: Text.PlainText

            Connections {
                target: storageMonitor
                onStorageAvailable: available.text = text
            }
        }
    }

    ColumnLayout {
        id: dateTimeColumn
        anchors.top: parent.top
        anchors.right: rightColumn.left
        z: 1
        opacity: 0.25

        Text {
            id: dateTime
            textFormat: Text.PlainText
            horizontalAlignment: Text.AlignRight
        }

        Timer {
            interval: 500
            repeat: true
            running: true
            triggeredOnStart: true

            onTriggered: dateTime.text = new Date().toLocaleString()
        }
    }
}
