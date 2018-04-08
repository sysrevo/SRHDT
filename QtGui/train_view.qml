import QtQuick 2.6
import QtQuick.Controls 2.0
import QtQuick.Dialogs 1.0
import data.backend 1.0

Item {
    id: training_view

    BackEnd {
        id: train_backend
    }

    FileDialog {
        id: file
        title: "Please choose the directory of training images"
        selectFolder: true
        selectExisting: true
        onAccepted: {
            console.log("You chose: " + file.fileUrls)
        }
        onRejected: {
            console.log("Canceled")
        }
    }

    Button {
        text: "start"
        onClicked: file.open()
    }
}
