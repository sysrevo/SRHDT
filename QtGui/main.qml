import QtQuick 2.6
import QtQuick.Controls 2.0
import QtQuick.Dialogs 1.0
import QtQuick.Layouts 1.3

ApplicationWindow {
    id: root
    width: 300
    height: 480
    visible: true

    property variant win
	
	Column {
		anchors.centerIn: parent
		spacing: 3
		Button {
			text: qsTr("Train")

			onClicked: {
				var component = Qt.createComponent("/train.qml")
				if( component.status == Component.Error )
					console.debug("Error:"+ component.errorString())
				var window = component.createObject(root)
				window.show()
			}
		}

		Button {
			text: qsTr("Predict")

			onClicked: {
				var component = Qt.createComponent("/predict.qml")
				if( component.status == Component.Error )
					console.debug("Error:"+ component.errorString())
				var window = component.createObject(root)
				window.show()
			}
		}
	}
    
}
