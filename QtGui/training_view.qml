import QtQuick 2.6
import QtQuick.Window 2.2
import QtQuick.Controls 2.0
import QtQuick.Dialogs 1.0
import data.backend 1.0

Window {
    id: train_view

    BackEnd {
        id: backend
    }

	Dialog {
        id: settings_dialog
        title: "Settings"
        standardButtons: StandardButton.Ok | StandardButton.Cancel

        onButtonClicked: {
            if (clickedButton==StandardButton.Ok) {
                console.log("Accepted " + clickedButton)
            } else {
                console.log("Rejected" + clickedButton)
            }
        }
    }

    FileDialog {
        id: file
        title: "Please choose the directory of training images"
        selectFolder: true
        selectExisting: true
        onAccepted: {
            console.log("You chose: " + file.fileUrls)
			Qt.createComponent("train_settings_dialog.qml").createObject(train_view, {});
			backend.Train(file.fileUrls)
        }
        onRejected: {
            console.log("Canceled")
        }
    }

    Button {
        text: "train"
        onClicked: {
			settings_dialog.open()
			file.open()
		}
    }
}
