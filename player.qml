import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.1
import QtQuick.Window 2.2
import QtGStreamer 1.0

Window {
    id: window
    title: "Simple DVR"

    visible: true
    visibility: "Maximized"
    //visibility: "FullScreen"

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
    }
}

