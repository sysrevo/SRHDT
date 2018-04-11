import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Dialogs 1.2
import QtQuick.Window 2.3
import QtQuick.Layouts 1.3
import QtQuick.Controls.Styles 1.4
import data.backend 1.0

ApplicationWindow {
	id: win_predict
	width: 600
	height: 480

	property string imgPath: "image://colors/red"

	FileDialog {
		id: file_data
		title: "Please choose the data file"
		selectMultiple: false
		selectFolder: false
		selectExisting: true

		onAccepted: {
			backend.read(fileUrl)
		}
	}

	FileDialog {
		id: file_predict
		title: "Please choose the data file"
		selectMultiple: false
		selectFolder: false
		selectExisting: true

		onAccepted: {
			backend.predict(fileUrl)
		}
	}

	FileDialog {
		id: file_predict_low
		title: "Please choose the data file"
		selectMultiple: false
		selectFolder: false
		selectExisting: true

		onAccepted: {
			backend.predictWithLowRes(fileUrl)
		}
	}

	FileDialog {
		id: file_img_save
		title: "Please choose the data file"
		selectMultiple: false
		selectFolder: false
		selectExisting: false

		onAccepted: {
			backend.savePredicted(fileUrl)
		}
	}

	Connections {
		target: backend
		onImagePredicted: {
			console.log("onImagePredicted")
			imgPath += "r"
		}
		onImageSaved: {
			console.log("onImageSaved")
		}
		onDataRead: {
			console.log("onDataRead")
		}
	}

	Column {
		anchors.centerIn: parent
		spacing: 2

		Rectangle {
			width: 300
			height: 300
			border.width: 2
			border.color: "black"
			
			Image {
				id: img_pred
				anchors.centerIn: parent
				width: parent.width - 3
				height: parent.height - 3
				fillMode: Image.PreserveAspectFit
				source: imgPath
				cache: false
			}
		}

		Row {
			Button {
				id: btn_read
				text: "Read data"
				onClicked: {
					file_data.open()
				}
			}
			Label {
				text: "Current data file: " + file_data.fileUrl
			}
		}

		Button {
			id: btn_predict
			text: "Predict"
			onClicked: {
				file_predict.open()
			}
		}
		
		Button {
			id: btn_predict_low
			text: "Predict"
			onClicked: {
				file_predict_low.open()
			}
		}

		Button {
			id: btn_save_img
			text: "Save"
			onClicked: {
				file_img_save.open()
			}
		}
	}
}