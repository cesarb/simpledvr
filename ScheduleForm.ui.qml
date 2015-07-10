import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.1

Row {
    property alias startDate: calendar.selectedDate
    property alias startHour: startHour.value
    property alias startMinute: startMinute.value
    property alias durationHours: durationHours.value
    property alias durationMinutes: durationMinutes.value
    property alias addButton: addButton

    spacing: 5

    Calendar {
        id: calendar
    }

    ColumnLayout {
        spacing: 10

        ColumnLayout {
            Text {
                text: "Start:"
                color: "#ffffff"
                style: Text.Outline
                styleColor: "#000000"
                textFormat: Text.PlainText
                horizontalAlignment: Text.AlignLeft
            }
            RowLayout {
                SpinBox {
                    id: startHour
                    activeFocusOnPress: false
                    minimumValue: 0
                    maximumValue: 23
                }
                Text {
                    text: ":"
                    color: "#ffffff"
                    style: Text.Outline
                    styleColor: "#000000"
                    textFormat: Text.PlainText
                    horizontalAlignment: Text.AlignHCenter
                }
                SpinBox {
                    id: startMinute
                    activeFocusOnPress: false
                    minimumValue: 0
                    maximumValue: 59
                }
            }
        }

        ColumnLayout {
            Text {
                text: "Duration:"
                color: "#ffffff"
                style: Text.Outline
                styleColor: "#000000"
                textFormat: Text.PlainText
                horizontalAlignment: Text.AlignLeft
            }
            RowLayout {
                SpinBox {
                    id: durationHours
                    activeFocusOnPress: false
                    minimumValue: 0
                    maximumValue: 23
                }
                Text {
                    text: ":"
                    color: "#ffffff"
                    style: Text.Outline
                    styleColor: "#000000"
                    textFormat: Text.PlainText
                    horizontalAlignment: Text.AlignHCenter
                }
                SpinBox {
                    id: durationMinutes
                    activeFocusOnPress: false
                    minimumValue: 0
                    maximumValue: 59
                }
            }
        }

        Button {
            id: addButton
            text: "Add"
        }
    }
}
