// SPDX-License-Identifier: GPL-2.0
import QtQuick 2.12
import QtQuick.Layouts 1.12
import org.kde.kirigami 2.4 as Kirigami
import org.subsurfacedivelog.mobile 1.0

Kirigami.ScrollablePage {
	objectName: "Settings"
	id: settingsPage
	property alias defaultCylinderModel: defaultCylinderBox.model
	property alias defaultCylinderIndex: defaultCylinderBox.currentIndex

	title: qsTr("Settings")
	background: Rectangle { color: subsurfaceTheme.backgroundColor }

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
					text: PrefCloudStorage.cloud_verification_status === Enums.CS_NOCLOUD ? qsTr("Not applicable") : PrefCloudStorage.cloud_storage_email
					Layout.preferredWidth: gridWidth * 0.60
				}
				SsrfButton {
					id: changeCloudSettings
					text: qsTr("Change")
					onClicked: {
						PrefCloudStorage.cloud_verification_status = Enums.CS_UNKNOWN
						manager.startPageText  = qsTr("Starting...");
					}
				}
				TemplateLabel {
					text: qsTr("Status")
					Layout.preferredWidth: gridWidth * 0.15
					Layout.preferredHeight: Kirigami.Units.gridUnit * 1.5
				}
				TemplateLabel {
					text: describe[PrefCloudStorage.cloud_verification_status]
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
				Layout.rightMargin: Kirigami.Units.gridUnit * 1.5

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
					Layout.preferredHeight: subsurfaceTheme.height * 2.5
					inputMethodHints: Qt.ImhNoPredictiveText
					Layout.fillWidth: true
					onActivated: {
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
				SsrfButton {
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
				id: themeSettings
				visible: sectionTheme.isExpanded
				columns: 3

				TemplateLabel {
					text: qsTr("Blue")
					rightPadding: Kirigami.Units.gridUnit
					Layout.preferredWidth: gridWidth * 0.15
				}
				Row {
					Layout.preferredWidth: gridWidth * 0.6
					Rectangle {
						id: blueRect
						color: subsurfaceTheme.blueBackgroundColor
						border.color: "black"
						width: sampleRegularBlue.width + Kirigami.Units.gridUnit
						height: Kirigami.Units.gridUnit * 2
						Text {
							id: sampleRegularBlue
							text: qsTr("regular text")
							font.pointSize: subsurfaceTheme.regularPointSize
							color: subsurfaceTheme.blueTextColor
							anchors {
								horizontalCenter: parent.horizontalCenter
								verticalCenter: parent.verticalCenter
							}
						}
					}
					Rectangle {
						color: subsurfaceTheme.bluePrimaryColor
						border.color: "black"
						width: sampleHighlightBlue.width + Kirigami.Units.gridUnit
						height: Kirigami.Units.gridUnit * 2
						Text {
							id: sampleHighlightBlue
							text: qsTr("Highlight")
							font.pointSize: subsurfaceTheme.regularPointSize
							color: subsurfaceTheme.bluePrimaryTextColor
							anchors {
								horizontalCenter: parent.horizontalCenter
								verticalCenter: parent.verticalCenter
							}
						}
					}
				}
				SsrfSwitch {
					id: blueButton
					Layout.preferredWidth: gridWidth * 0.25
					checked: subsurfaceTheme.currentTheme === "Blue"
					enabled: subsurfaceTheme.currentTheme !== "Blue"
					onClicked: {
						subsurfaceTheme.currentTheme = "Blue"
					}
				}

				TemplateLabel {
					id: pinkLabel
					text: qsTr("Pink")
					rightPadding: Kirigami.Units.gridUnit
					Layout.preferredWidth: gridWidth * 0.15
				}
				Row {
					Layout.preferredWidth: gridWidth * 0.6
					Rectangle {
						id: pinkRect
						color: subsurfaceTheme.pinkBackgroundColor
						border.color: "black"
						width: sampleRegularPink.width + Kirigami.Units.gridUnit
						height: Kirigami.Units.gridUnit * 2
						Text {
							id: sampleRegularPink
							text: qsTr("regular text")
							font.pointSize: subsurfaceTheme.regularPointSize
							color: subsurfaceTheme.pinkTextColor
							anchors {
								horizontalCenter: parent.horizontalCenter
								verticalCenter: parent.verticalCenter
							}
						}
					}
					Rectangle {
						color: subsurfaceTheme.pinkPrimaryColor
						border.color: "black"
						width: sampleHighlightPink.width + Kirigami.Units.gridUnit
						height: Kirigami.Units.gridUnit * 2
						Text {
							id: sampleHighlightPink
							text: qsTr("Highlight")
							font.pointSize: subsurfaceTheme.regularPointSize
							color: subsurfaceTheme.pinkPrimaryTextColor
							anchors {
								horizontalCenter: parent.horizontalCenter
								verticalCenter: parent.verticalCenter
							}
						}
					}
				}

				SsrfSwitch {
					id: pinkButton
					Layout.preferredWidth: gridWidth * 0.25
					checked: subsurfaceTheme.currentTheme === "Pink"
					enabled: subsurfaceTheme.currentTheme !== "Pink"
					onClicked: {
						subsurfaceTheme.currentTheme = "Pink"
					}
				}

				TemplateLabel {
					text: qsTr("Dark")
					rightPadding: Kirigami.Units.gridUnit
					Layout.preferredWidth: gridWidth * 0.15
				}
				Row {
					Layout.preferredWidth: gridWidth * 0.6
					Rectangle {
						id: blackRect
						color: subsurfaceTheme.darkBackgroundColor
						border.color: "black"
						width: sampleRegularDark.width + Kirigami.Units.gridUnit
						height: Kirigami.Units.gridUnit * 2
						Text {
							id: sampleRegularDark
							text: qsTr("regular text")
							font.pointSize: subsurfaceTheme.regularPointSize
							color: subsurfaceTheme.darkTextColor
							anchors {
								horizontalCenter: parent.horizontalCenter
								verticalCenter: parent.verticalCenter
							}
						}
					}
					Rectangle {
						color: subsurfaceTheme.darkPrimaryColor
						border.color: "black"
						width: sampleHighlightDark.width + Kirigami.Units.gridUnit
						height: Kirigami.Units.gridUnit * 2
						Text {
							id: sampleHighlightDark
							text: qsTr("Highlight")
							font.pointSize: subsurfaceTheme.regularPointSize
							color: subsurfaceTheme.darkPrimaryTextColor
							anchors {
								horizontalCenter: parent.horizontalCenter
								verticalCenter: parent.verticalCenter
							}
						}
					}
				}
				SsrfSwitch {
					id: darkButton
					Layout.preferredWidth: gridWidth * 0.25
					checked: subsurfaceTheme.currentTheme === "Dark"
					enabled: subsurfaceTheme.currentTheme !== "Dark"
					onClicked: {
						subsurfaceTheme.currentTheme = "Dark"
					}
				}
				TemplateLabel {
					text: qsTr("Scaling")
					font.pointSize: subsurfaceTheme.headingPointSize
					font.weight: Font.Light
					Layout.topMargin: Kirigami.Units.largeSpacing
					Layout.bottomMargin: Kirigami.Units.largeSpacing / 2
					Layout.columnSpan: 3
				}
				Row {
					Layout.preferredWidth: gridWidth * 0.8
					Layout.columnSpan: 3
					spacing: Kirigami.Units.largeSpacing
					SsrfButton {
						text: qsTr("smaller")
						enabled: subsurfaceTheme.currentScale !== 0.85
						onClicked: {
							subsurfaceTheme.currentScale = 0.85
						}
					}
					SsrfButton {
						text: qsTr("regular")
						enabled: subsurfaceTheme.currentScale !== 1.0
						onClicked: {
							subsurfaceTheme.currentScale = 1.0
						}
					}
					SsrfButton {
						text: qsTr("larger")
						enabled: subsurfaceTheme.currentScale !== 1.15
						onClicked: {
							subsurfaceTheme.currentScale = 1.15
						}
					}
				}
			}
		}

		TemplateSection {
			id: sectionUnits
			title: qsTr("Units")

			GridLayout {
				id: unit_system
				visible: sectionUnits.isExpanded
				columns: 2

				TemplateLabel {
					text: qsTr("Use Imperial Units")
					Layout.preferredWidth: gridWidth * 0.75
				}
				SsrfSwitch {
					id: imperialButton
					checked: PrefUnits.unit_system === Enums.IMPERIAL
					enabled: PrefUnits.unit_system === Enums.METRIC
					Layout.preferredWidth: gridWidth * 0.25
					onClicked: {
						PrefUnits.unit_system = Enums.IMPERIAL
						manager.changesNeedSaving()
						manager.refreshDiveList()
					}
				}
				TemplateLabel {
					text: qsTr("Use Metric Units")
					Layout.preferredWidth: gridWidth * 0.75
				}
				SsrfSwitch {
					id: metricButtton
					checked: PrefUnits.unit_system === Enums.METRIC
					enabled: PrefUnits.unit_system === Enums.IMPERIAL
					Layout.preferredWidth: gridWidth * 0.25
					onClicked: {
						PrefUnits.unit_system = Enums.METRIC
						manager.changesNeedSaving()
						manager.refreshDiveList()
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
				id: filterPrefs
				visible: sectionAdvanced.isExpanded
				columns: 2

				TemplateLabel {
					text: qsTr("Filter preferences")
					font.pointSize: subsurfaceTheme.headingPointSize
					font.weight: Font.Light
					Layout.topMargin: Kirigami.Units.largeSpacing
					Layout.bottomMargin: Kirigami.Units.largeSpacing / 2
					Layout.columnSpan: 2
				}
				TemplateLabel {
					text: qsTr("Include notes in full text filtering")
					Layout.preferredWidth: gridWidth * 0.75
				}

				SsrfSwitch {
					id: fullTextNotes
					checked: PrefGeneral.filterFullTextNotes
					Layout.preferredWidth: gridWidth * 0.25
					onClicked: {
						PrefGeneral.filterFullTextNotes = checked
					}
				}

				TemplateLabel {
					text: qsTr("Match filter case sensitive")
					Layout.preferredWidth: gridWidth * 0.75
				}

				SsrfSwitch {
					id: filterCaseSensitive
					checked: PrefGeneral.filterCaseSensitive
					Layout.preferredWidth: gridWidth * 0.25
					onClicked: {
						PrefGeneral.filterCaseSensitive = checked
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
