import QtQuick 2.0

Item {
    id : container
    property alias cell_color: rectangle.color
    signal clicked(color cell_color)

    width: 40; height: 25

    Rectangle {
        id : rectangle
        border.color: "white"
        anchors.fill: parent
    }

    MouseArea {
        anchors.fill: parent
        onClicked: container.clicked(container.cell_color)
    }

}