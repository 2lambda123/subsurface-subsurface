// SPDX-License-Identifier: GPL-2.0
import QtQuick 2.6
import QtQuick.Controls 2.2 as Controls
import QtQuick.Window 2.2
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.2
import org.kde.kirigami 2.4 as Kirigami
import org.subsurfacedivelog.mobile 1.0

Item {
	id: loginWindow
	height: outerLayout.height

	property string username: login.text;
	property string password: password.text;

	ColumnLayout {
		id: outerLayout
		width: loginWindow.width - Kirigami.Units.gridUnit // to ensure the full input fields are visible

		function goToNext() {
			for (var i = 0; i < children.length; ++i)
				if (children[i].focus) {
					children[i].nextItemInFocusChain().forceActiveFocus()
					break
				}
		}

		Keys.onReturnPressed: goToNext()
		Keys.onTabPressed: goToNext()

		onVisibleChanged: {
			if (visible) {
				Qt.inputMethod.show()
				login.forceActiveFocus()
			} else {
				Qt.inputMethod.hide()
			}
		}

		Kirigami.Heading {
			text: qsTr("Cloud credentials")
			level: headingLevel
			Layout.bottomMargin: Kirigami.Units.largeSpacing / 2
		}

		Controls.Label {
			text: qsTr("Email")
			visible: PrefCloudStorage.cloud_verification_status !== CloudStatus.CS_NEED_TO_VERIFY
			font.pointSize: subsurfaceTheme.smallPointSize
			color: subsurfaceTheme.secondaryTextColor
		}

		Controls.TextField {
			id: login
			text: PrefCloudStorage.cloud_storage_email
			visible: PrefCloudStorage.cloud_verification_status !== CloudStatus.CS_NEED_TO_VERIFY
			Layout.fillWidth: true
			inputMethodHints: Qt.ImhEmailCharactersOnly |
					  Qt.ImhNoAutoUppercase
		}

		Controls.Label {
			text: qsTr("Password")
			visible: PrefCloudStorage.cloud_verification_status !== CloudStatus.CS_NEED_TO_VERIFY
			font.pointSize: subsurfaceTheme.smallPointSize
			color: subsurfaceTheme.secondaryTextColor
		}

		Controls.TextField {
			id: password
			text: PrefCloudStorage.cloud_storage_password
			visible: PrefCloudStorage.cloud_verification_status !== CloudStatus.CS_NEED_TO_VERIFY
			echoMode: TextInput.PasswordEchoOnEdit
			inputMethodHints: Qt.ImhSensitiveData |
					  Qt.ImhHiddenText |
					  Qt.ImhNoAutoUppercase
			Layout.fillWidth: true
		}

		Controls.Label {
			text: qsTr("PIN")
			visible: PrefCloudStorage.cloud_verification_status === CloudStatus.CS_NEED_TO_VERIFY
		}
		Controls.TextField {
			id: pin
			text: ""
			Layout.fillWidth: true
			visible: PrefCloudStorage.cloud_verification_status === CloudStatus.CS_NEED_TO_VERIFY
		}

		RowLayout {
			Layout.fillWidth: true
			Layout.margins: Kirigami.Units.smallSpacing
			spacing: Kirigami.Units.smallSpacing
			visible: PrefCloudStorage.cloud_verification_status === CloudStatus.CS_NEED_TO_VERIFY
			SsrfButton {
				id: registerpin
				text: qsTr("Register")
				onClicked: {
					manager.saveCloudCredentials(login.text, password.text, pin.text)
				}
			}
			Controls.Label {
				text: ""  // Spacer between 2 button groups
				Layout.fillWidth: true
			}
			SsrfButton {
				id: cancelpin
				text: qsTr("Cancel")
				onClicked: {
					PrefCloudStorage.cloud_verification_status = CloudStatus.CS_UNKNOWN
					manager.startPageText = qsTr("Check credentials...");
				}
			}
		}

		RowLayout {
			Layout.fillWidth: true
			Layout.margins: Kirigami.Units.smallSpacing
			spacing: Kirigami.Units.smallSpacing
			visible: PrefCloudStorage.cloud_verification_status !== CloudStatus.CS_NEED_TO_VERIFY

			SsrfButton {
				id: signin_register_normal
				text: qsTr("Sign-in or Register")
				onClicked: {
					manager.saveCloudCredentials(login.text, password.text, "")
				}
			}
			Controls.Label {
				text: ""  // Spacer between 2 button groups
				Layout.fillWidth: true
			}
			SsrfButton {
				id: toNoCloudMode
				text: qsTr("No cloud mode")
				onClicked: {
					manager.setGitLocalOnly(true)
					PrefCloudStorage.cloud_auto_sync = false
					manager.oldStatus = PrefCloudStorage.cloud_verification_status
					PrefCloudStorage.cloud_verification_status = CloudStatus.CS_NOCLOUD
					manager.saveCloudCredentials("", "", "")
					manager.openNoCloudRepo()
				}
			}
		}
		SsrfButton {
			id: signin_forgot_password
			text: qsTr("Forgot password?")
			onClicked: {
				Qt.openUrlExternally("https://cloud.subsurface-divelog.org/passwordreset")
			}
		}
	}
}
