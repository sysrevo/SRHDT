import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Dialogs 1.2
import QtQuick.Window 2.3
import QtQuick.Layouts 1.3
import QtQuick.Controls.Styles 1.4
import data.backend 1.0

ApplicationWindow {
    id: train
    width: 600
    height: 300

	Dialog {
		id: dlg_settings
		title: "Settings"
		standardButtons: StandardButton.Yes | StandardButton.Cancel
		
		FileDialog {
			id: filedlg_imgs
			title: "Please choose the directory of training images"
			selectMultiple: false
			selectFolder: true
		}

		Column {
			spacing: 2

			Row {
				Label { 
					text: "Patch size" 
				}
				TextField {
					id: in_patch_size
					text: "6" 
					width: 64
				}
			}

			Row {
				Label {
					text: "Overlapped pixels" 
				}
				TextField {
					id: in_overlap
					text: "4" 
					width: 64
				}
			}

			Row {
				Label { 
					text: "k" 
				}
				TextField {
					id: in_k
					text: "0.7" 
					width: 64
				}
			}

			Row {
				Label { 
					text: "lamda" 
				}
				TextField {
					id: in_lamda
					text: "0.01" 
					width: 64
				}
			}

			Row {
				Label { 
					text: "min patches per node" 
				}
				TextField {
					id: in_min_patches
					text: "1000" 
					width: 64
				}
			}

			Row {
				Label { 
					text: "number of tests" 
				}
				TextField {
					id: in_num_tests
					text: "36" 
					width: 64
				}
			}

			Row {
				Label {
					text: "number of layers" 
				}
				
				TextField {
					id: in_num_layers
					text: "4" 
					width: 64
				}
			}

			Row {
				Label {
					text: "number of tests" 
				}
				
				TextField {
					id: in_tests
					text: "36" 
					width: 64
				}
			}

			Row {
				Label {
					text: "data path" 
				}
				
				TextField {
					id: in_data_path
					text: filedlg_imgs.fileUrl
					width: 256
				}

				Button {
					text: "Select"
					onClicked: {
						filedlg_imgs.open()
					}
				}
			}
		}

		onYes: {
			btn_start.visible = false
			backend.train(in_data_path.text, 
				in_patch_size.text, 
				in_overlap.text, 
				in_k.text, 
				in_lamda.text, 
				in_min_patches.text, 
				in_tests.text
			)
		}
	}

	FileDialog {
		id: filedlg_data
		title: "Please select the data file"
		selectMultiple: false
		selectExisting: false

		onAccepted: {
			backend.write(fileUrl)
		}
	}

	Connections {
        target: backend
        onCompleted: {
			btn_save.visible = true
		}
    }
	
	Column {
		anchors.centerIn: parent
		Button {
			id: btn_start
			text: "Start training"
			onClicked: {
				dlg_settings.open()
			}
		}
		Label {
			text: "Current layer: " + backend.layer
		}
		Label {
			text: "Current number of samples: " + backend.numSamples
		}
		Label {
			text: "Current number of nodes: " + backend.numNodes
		}
		Label {
			text: "Current number of leaves: " + backend.numLeaves
		}
		
		Row {
			Label {
				text: "Current number of tests: " + backend.numTests
			}
			ProgressBar {
				value: backend.percentCurrentTest
			}
		}
		Button {
			id: btn_save
			visible: false
			text: "Save result"
			onClicked: {
				filedlg_data.open()
			}
		}
	}
}
