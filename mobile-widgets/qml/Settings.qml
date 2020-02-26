// SPDX-License-Identifier: GPL-2.0
import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import org.kde.kirigami 2.4 as Kirigami
import org.subsurfacedivelog.mobile 1.0

TemplatePage {
	objectName: "Settings"
	id: settingsPage
	property alias defaultCylinderModel: defaultCylinderBox.model
	property alias defaultCylinderIndex: defaultCylinderBox.currentIndex

	title: qsTr("Settings")

	property real gridWidth: settingsPage.width - Kirigami.Units.gridUnit
	property var describe: [qsTr("Undefined"),
		qsTr("Incorrect username/password combination"),
		qsTr("Credentials need to be verified"),
		qsTr("Credentials verified"),
		qsTr("No cloud mode")]

	Column {
		width: gridWidth

		TemplateSection {
			id: sectionGeneral
			title: qsTr("General settings")
			isExpanded: true

			GridLayout {
				id: cloudSetting
				visible: sectionGeneral.isExpanded
				columns: 3

				TemplateLabel {
					text: qsTr("Cloud status")
					font.pointSize: subsurfaceTheme.headingPointSize
					font.weight: Font.Light
					Layout.topMargin: Kirigami.Units.largeSpacing
					Layout.bottomMargin: Kirigami.Units.largeSpacing / 2
					Layout.columnSpan: 3
				}
				TemplateLabel {
					text: qsTr("Email")
					Layout.preferredWidth: gridWidth * 0.15
				}
				TemplateLabel {
					text: Backend.cloud_verification_status === Enums.CS_NOCLOUD ? qsTr("Not applicable") : PrefCloudStorage.cloud_storage_email
					Layout.preferredWidth: gridWidth * 0.60
				}
				TemplateButton {
					id: changeCloudSettings
					text: qsTr("Change")
					onClicked: {
						Backend.cloud_verification_status = Enums.CS_UNKNOWN
						manager.startPageText  = qsTr("Starting...");
					}
				}
				TemplateLabel {
					text: qsTr("Status")
					Layout.preferredWidth: gridWidth * 0.15
					Layout.preferredHeight: Kirigami.Units.gridUnit * 1.5
				}
				TemplateLabel {
					text: describe[Backend.cloud_verification_status]
					Layout.preferredWidth: gridWidth * 0.60
					Layout.preferredHeight: Kirigami.Units.gridUnit * 1.5
				}
			}
			TemplateLine {
				visible: sectionGeneral.isExpanded
			}
			GridLayout {
				id: defaultCylinder
				visible: sectionGeneral.isExpanded
				columns: 2
				width: parent.width

				TemplateLabel {
					text: qsTr("Default Cylinder")
					font.pointSize: subsurfaceTheme.headingPointSize
					font.weight: Font.Light
					Layout.topMargin: Kirigami.Units.largeSpacing
					Layout.bottomMargin: Kirigami.Units.largeSpacing / 2
					Layout.columnSpan: 2
				}
				TemplateLabel {
					text: qsTr("Cylinder:")
				}
				TemplateComboBox {
					id: defaultCylinderBox
					Layout.fillWidth: true
					onActivated: {
						// the entry for 'no default cylinder' is known to be the top, but its text
						// is possibly translated so check against the index
						if (currentIndex === 0)
							PrefEquipment.default_cylinder = ""
						else
							PrefEquipment.default_cylinder = defaultCylinderBox.currentText
					}
				}
			}
			TemplateLine {
				visible: sectionGeneral.isExpanded
			}
			GridLayout {
				id: divecomputers
				visible: sectionGeneral.isExpanded
				columns: 2
				TemplateLabel {
					text: qsTr("Dive computers")
					font.pointSize: subsurfaceTheme.headingPointSize
					font.weight: Font.Light
					Layout.topMargin: Kirigami.Units.largeSpacing
					Layout.bottomMargin: Kirigami.Units.largeSpacing
					Layout.columnSpan: 2
				}
				TemplateLabel {
					text: qsTr("Forget remembered dive computers")
					Layout.preferredWidth: gridWidth * 0.75
				}
				TemplateButton {
					id: forgetDCButton
					text: qsTr("Forget")
					enabled: PrefDiveComputer.vendor1 !== ""
					onClicked: {
						PrefDiveComputer.vendor1 = PrefDiveComputer.product1 = PrefDiveComputer.device1 = ""
						PrefDiveComputer.vendor2 = PrefDiveComputer.product2 = PrefDiveComputer.device2 = ""
						PrefDiveComputer.vendor3 = PrefDiveComputer.product3 = PrefDiveComputer.device3 = ""
						PrefDiveComputer.vendor4 = PrefDiveComputer.product4 = PrefDiveComputer.device4 = ""
						PrefDiveComputer.vendor = PrefDiveComputer.product = PrefDiveComputer.device = ""
					}
				}
			}
		}

		TemplateSection {
			id: sectionTheme
			title: qsTr("Theme")

			GridLayout {
				visible: sectionTheme.isExpanded
				columns: 2

				TemplateComboBox {
					editable: false
					Layout.columnSpan: 2
					currentIndex: (subsurfaceTheme.currentTheme === "Blue") ? 0 :
								  (subsurfaceTheme.currentTheme === "Pink") ? 1 : 2
					model: ListModel {
						   ListElement {text: qsTr("Blue")}
						   ListElement {text: qsTr("Pink")}
						   ListElement {text: qsTr("Dark")}
					}
					onActivated:  {
						subsurfaceTheme.currentTheme = currentIndex === 0 ? "Blue" : currentIndex === 1 ? "Pink" : "Dark"
					}
				}

				Rectangle {
					color: 	subsurfaceTheme.backgroundColor
					width: rootItem.width / 2 - 20
					height: subsurfaceTheme.regularPointSize + 10

					TemplateLabel {
						text: qsTr("background")
						color: subsurfaceTheme.textColor
						colorBackground: parent.color
						anchors.horizontalCenter: parent.horizontalCenter
						anchors.verticalCenter: parent.verticalCenter
					}
				}
				Rectangle {
					color: subsurfaceTheme.backgroundColor
					width: rootItem.width / 2 - 60
					height: subsurfaceTheme.regularPointSize + 10

					TemplateLabel {
						text: qsTr("text")
						color: subsurfaceTheme.textColor
						colorBackground: parent.color
						anchors.horizontalCenter: parent.horizontalCenter
						anchors.verticalCenter: parent.verticalCenter
					}
				}

				Rectangle {
					border.width: 2
					border.color: "black"
					color: 	subsurfaceTheme.primaryColor
					width: rootItem.width / 2 - 20
					height: subsurfaceTheme.regularPointSize + 10

					TemplateLabel {
						text: qsTr("primary")
						color: subsurfaceTheme.primaryTextColor
						colorBackground: parent.color
						anchors.horizontalCenter: parent.horizontalCenter
						anchors.verticalCenter: parent.verticalCenter
					}
				}
				Rectangle {
					border.width: 2
					border.color: "black"
					color: subsurfaceTheme.primaryTextColor
					width: rootItem.width / 2 - 60
					height: subsurfaceTheme.regularPointSize + 10

					TemplateLabel {
						text: qsTr("primary text")
						color: subsurfaceTheme.primaryColor
						colorBackground: parent.color
						anchors.horizontalCenter: parent.horizontalCenter
						anchors.verticalCenter: parent.verticalCenter
					}
				}

				Rectangle {
					border.width: 2
					border.color: "black"
					color: 	subsurfaceTheme.darkerPrimaryColor
					width: rootItem.width / 2 - 20
					height: subsurfaceTheme.regularPointSize + 10

					TemplateLabel {
						text: qsTr("darker primary")
						color: subsurfaceTheme.darkerPrimaryTextColor
						colorBackground: parent.color
						anchors.horizontalCenter: parent.horizontalCenter
						anchors.verticalCenter: parent.verticalCenter
					}
				}
				Rectangle {
					border.width: 2
					border.color: "black"
					color: subsurfaceTheme.darkerPrimaryTextColor
					width: rootItem.width / 2 - 60
					height: subsurfaceTheme.regularPointSize + 10

					TemplateLabel {
						text: qsTr("darker primary text")
						color: subsurfaceTheme.darkerPrimaryColor
						colorBackground: parent.color
						anchors.horizontalCenter: parent.horizontalCenter
						anchors.verticalCenter: parent.verticalCenter
					}
				}

				Rectangle {
					border.width: 2
					border.color: "black"
					color: 	subsurfaceTheme.lightPrimaryColor
					width: rootItem.width / 2 - 20
					height: subsurfaceTheme.regularPointSize + 10

					TemplateLabel {
						text: qsTr("light primary")
						color: subsurfaceTheme.lightPrimaryTextColor
						colorBackground: parent.color
						anchors.horizontalCenter: parent.horizontalCenter
						anchors.verticalCenter: parent.verticalCenter
					}
				}
				Rectangle {
					border.width: 2
					border.color: "black"
					color: subsurfaceTheme.lightPrimaryTextColor
					width: rootItem.width / 2 - 60
					height: subsurfaceTheme.regularPointSize + 10

					TemplateLabel {
						text: qsTr("light primary text")
						color: subsurfaceTheme.lightPrimaryColor
						colorBackground: parent.color
						anchors.horizontalCenter: parent.horizontalCenter
						anchors.verticalCenter: parent.verticalCenter
					}
				}


				TemplateLabel {
					text: ""
				}
				Rectangle {
					border.width: 2
					border.color: "black"
					color: subsurfaceTheme.secondaryTextColor
					width: rootItem.width / 2 - 60
					height: subsurfaceTheme.regularPointSize + 10

					TemplateLabel {
						text: qsTr("secondary text")
						color: subsurfaceTheme.primaryColor
						colorBackground: parent.color
						anchors.horizontalCenter: parent.horizontalCenter
						anchors.verticalCenter: parent.verticalCenter
					}
				}

				Rectangle {
					border.width: 2
					border.color: "black"
					color: 	subsurfaceTheme.drawerColor
					width: rootItem.width / 2 - 20
					height: subsurfaceTheme.regularPointSize + 10

					TemplateLabel {
						text: qsTr("drawer")
						color: subsurfaceTheme.textColor
						colorBackground: parent.color
						anchors.horizontalCenter: parent.horizontalCenter
						anchors.verticalCenter: parent.verticalCenter
					}
				}
				TemplateLabel {
					text: ""
				}
				Row {
					Layout.preferredWidth: gridWidth * 0.8
					Layout.columnSpan: 2
					spacing: Kirigami.Units.largeSpacing
					TemplateButton {
						text: qsTr("smaller")
						enabled: subsurfaceTheme.currentScale !== 0.85
						onClicked: {
							subsurfaceTheme.currentScale = 0.85
						}
					}
					TemplateButton {
						text: qsTr("regular")
						enabled: subsurfaceTheme.currentScale !== 1.0
						onClicked: {
							subsurfaceTheme.currentScale = 1.0
						}
					}
					TemplateButton {
						text: qsTr("larger")
						enabled: subsurfaceTheme.currentScale !== 1.15
						onClicked: {
							subsurfaceTheme.currentScale = 1.15
						}
					}
				}
				Rectangle {
				}
			}
		}
		TemplateSection {
			id: sectionUnits
			title: qsTr("Units")

			Row {
				visible: sectionUnits.isExpanded

				TemplateRadioButton {
					text: qsTr("Metric")
					checked: (Backend.unit_system === Enums.METRIC)
					onClicked: {
						Backend.unit_system = Enums.METRIC
						manager.changesNeedSaving()
						manager.refreshDiveList()
					}
				}
				TemplateRadioButton {
					text: qsTr("Imperial")
					checked:  (Backend.unit_system === Enums.IMPERIAL)
					onClicked: {
						Backend.unit_system = Enums.IMPERIAL
						manager.changesNeedSaving()
						manager.refreshDiveList()
					}
				}
				TemplateRadioButton {
					text: qsTr("Personalize")
					checked:  (Backend.unit_system === Enums.PERSONALIZE)
					onClicked: {
						Backend.unit_system = Enums.PERSONALIZE
						manager.changesNeedSaving()
						manager.refreshDiveList()
					}
				}
			}

			GridLayout {
				visible: sectionUnits.isExpanded
				enabled: (Backend.unit_system === Enums.PERSONALIZE)
				columns: 3

				ButtonGroup { id: unitDepth }
				ButtonGroup { id: unitPressure }
				ButtonGroup { id: unitVolume }
				ButtonGroup { id: unitTemperature }
				ButtonGroup { id: unitWeight }

				TemplateLabel {
					text: qsTr("Depth")
					font.bold: (Backend.unit_system === Enums.PERSONALIZE)
				}
				TemplateRadioButton {
					text: qsTr("meters")
					checked: (Backend.length === Enums.METERS)
					ButtonGroup.group: unitDepth
					onClicked: {
						Backend.length = Enums.METERS
					}
				}
				TemplateRadioButton {
					text: qsTr("feet")
					checked: (Backend.length === Enums.FEET)
					ButtonGroup.group: unitDepth
					onClicked: {
						Backend.length = Enums.FEET
					}
				}
				TemplateLabel {
					text: qsTr("Pressure")
					font.bold: (Backend.unit_system === Enums.PERSONALIZE)
				}
				TemplateRadioButton {
					text: qsTr("bar")
					checked: (Backend.pressure === Enums.BAR)
					ButtonGroup.group: unitPressure
					onClicked: {
						Backend.pressure = Enums.BAR
					}
				}
				TemplateRadioButton {
					text: qsTr("psi")
					checked: (Backend.pressure === Enums.PSI)
					ButtonGroup.group: unitPressure
					onClicked: {
						Backend.pressure = Enums.PSI
					}
				}
				TemplateLabel {
					text: qsTr("Volume")
					font.bold: (Backend.unit_system === Enums.PERSONALIZE)
				}
				TemplateRadioButton {
					text: qsTr("liter")
					checked: (Backend.volume === Enums.LITER)
					ButtonGroup.group: unitVolume
					onClicked: {
						Backend.volume = Enums.LITER
					}
				}
				TemplateRadioButton {
					text: qsTr("cuft")
					checked: (Backend.volume === Enums.CUFT)
					ButtonGroup.group: unitVolume
					onClicked: {
						Backend.volume = Enums.CUFT
					}
				}
				TemplateLabel {
					text: qsTr("Temperature")
					font.bold: (Backend.unit_system === Enums.PERSONALIZE)
				}
				TemplateRadioButton {
					text: qsTr("celsius")
					checked: (Backend.temperature === Enums.CELSIUS)
					ButtonGroup.group: unitTemperature
					onClicked: {
						Backend.temperature = Enums.CELSIUS
					}
				}
				TemplateRadioButton {
					text: qsTr("fahrenheit")
					checked: (Backend.temperature === Enums.FAHRENHEIT)
					ButtonGroup.group: unitTemperature
					onClicked: {
						Backend.temperature = Enums.FAHRENHEIT
					}
				}
				TemplateLabel {
					text: qsTr("Weight")
					font.bold: (Backend.unit_system === Enums.PERSONALIZE)
				}
				TemplateRadioButton {
					text: qsTr("kg")
					checked: (Backend.weight === Enums.KG)
					ButtonGroup.group: unitWeight
					onClicked: {
						Backend.weight = Enums.KG
					}
				}
				TemplateRadioButton {
					text: qsTr("lbs")
					checked: (Backend.weight === Enums.LBS)
					ButtonGroup.group: unitWeight
					onClicked: {
						Backend.weight = Enums.LBS
					}
				}
			}
		}

		TemplateSection {
			id: sectionAdvanced
			title: qsTr("Advanced")

			GridLayout {
				id: gpsPrefs
				visible: sectionAdvanced.isExpanded
				columns: 2

				TemplateLabel {
					text: qsTr("GPS location service")
					font.pointSize: subsurfaceTheme.headingPointSize
					font.weight: Font.Light
					Layout.topMargin: Kirigami.Units.largeSpacing
					Layout.bottomMargin: Kirigami.Units.largeSpacing / 2
					Layout.columnSpan: 2
				}

				TemplateLabel {
					text: qsTr("Distance threshold (meters)")
					Layout.preferredWidth: gridWidth * 0.75
				}

				TemplateTextField {
					id: distanceThreshold
					text: PrefLocationService.distance_threshold
					Layout.preferredWidth: gridWidth * 0.25
					onEditingFinished: {
						PrefLocationService.distance_threshold = distanceThreshold.text
					}
				}

				TemplateLabel {
					text: qsTr("Time threshold (minutes)")
					Layout.preferredWidth: gridWidth * 0.75
				}

				TemplateTextField {
					id: timeThreshold
					text: PrefLocationService.time_threshold / 60
					Layout.preferredWidth: gridWidth * 0.25
					onEditingFinished: {
						PrefLocationService.time_threshold = timeThreshold.text * 60
					}
				}
			}
			TemplateLine {
				visible: sectionAdvanced.isExpanded
			}
			GridLayout {
				id: whichBluetoothDevices
				visible: sectionAdvanced.isExpanded
				columns: 2
				TemplateLabel {
					text: qsTr("Bluetooth")
					font.pointSize: subsurfaceTheme.headingPointSize
					font.weight: Font.Light
					Layout.topMargin: Kirigami.Units.largeSpacing
					Layout.bottomMargin: Kirigami.Units.largeSpacing / 2
					Layout.columnSpan: 2
				}

				TemplateLabel {
					text: qsTr("Temporarily show all bluetooth devices \neven if not recognized as dive computers.\nPlease report DCs that need this setting")
					Layout.preferredWidth: gridWidth * 0.75
				}
				SsrfSwitch {
					id: nonDCButton
					checked: manager.showNonDiveComputers
					Layout.preferredWidth: gridWidth * 0.25
					onClicked: {
						manager.showNonDiveComputers = checked
					}
				}
			}
			TemplateLine {
				visible: sectionAdvanced.isExpanded
			}
			GridLayout {
				id: display
				visible: sectionAdvanced.isExpanded
				columns: 2
				TemplateLabel {
					text: qsTr("Display")
					font.pointSize: subsurfaceTheme.headingPointSize
					font.weight: Font.Light
					Layout.topMargin: Kirigami.Units.largeSpacing
					Layout.bottomMargin: Kirigami.Units.largeSpacing / 2
					Layout.columnSpan: 2
				}
				TemplateLabel {
					text: qsTr("Show only one column in Portrait mode")
					Layout.preferredWidth: gridWidth * 0.75
				}
				SsrfSwitch {
					id: singleColumnButton
					checked: PrefDisplay.singleColumnPortrait
					Layout.preferredWidth: gridWidth * 0.25
					onClicked: {
						PrefDisplay.singleColumnPortrait = checked
					}
				}
			}

			TemplateLine {
				visible: sectionAdvanced.isExpanded
			}
			GridLayout {
				id: developer
				visible: sectionAdvanced.isExpanded
				columns: 2
				TemplateLabel {
					text: qsTr("Developer")
					font.pointSize: subsurfaceTheme.headingPointSize
					font.weight: Font.Light
					Layout.topMargin: Kirigami.Units.largeSpacing
					Layout.bottomMargin: Kirigami.Units.largeSpacing / 2
					Layout.columnSpan: 2
				}

				TemplateLabel {
					text: qsTr("Display Developer menu")
					Layout.preferredWidth: gridWidth * 0.75
				}
				SsrfSwitch {
					id: developerButton
					checked: PrefDisplay.show_developer
					Layout.preferredWidth: gridWidth * 0.25
					onClicked: {
						PrefDisplay.show_developer = checked
					}
				}
			}
		}
	}
}
