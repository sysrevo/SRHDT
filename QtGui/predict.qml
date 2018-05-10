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
    property string lastFileUrl: ""

	FileDialog {
		id: file_data
		title: "Please choose the data file"
		selectMultiple: false
		selectFolder: false
		selectExisting: true

		onAccepted: {
			backend.read(fileUrl)
			file_data.fileUrl = fileUrl
			file_pred_bicubic.fileUrl = fileUrl
			file_pred.fileUrl = fileUrl
			file_img_save.fileUrl = fileUrl
		}
	}

	FileDialog {
		id: file_pred_bicubic
		title: "Please choose the data file"
		nameFilters: ["Image files(*.jpg *.png)"]
		selectMultiple: false
		selectFolder: false
		selectExisting: true

		onAccepted: {
			backend.predictBicubic(fileUrl)
			lastFileUrl = fileUrl
		}
	}

	FileDialog {
		id: file_pred
		title: "Please choose the data file"
		nameFilters: ["Image files(*.jpg *.png)"]
		selectMultiple: false
		selectFolder: false
		selectExisting: true

		onAccepted: {
			backend.predict(fileUrl)
			lastFileUrl = fileUrl
		}
	}

	FileDialog {
		id: file_img_save
		title: "Please choose the data file"
		nameFilters: ["Image files(*.png)"]
		selectMultiple: false
		selectFolder: false
		selectExisting: false

		onAccepted: {
			backend.savePredicted(fileUrl)
			lastFileUrl = fileUrl
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
				file_pred.open()
			}
		}
		
		Button {
			id: btn_predict_low
			text: "Predict Bicubic"
			onClicked: {
				file_pred_bicubic.open()
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