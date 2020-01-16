// SPDX-License-Identifier: GPL-2.0
import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.11


Item {
	property alias title: myLabel.text
	width: parent.width -20
	height: myLabel.height + 1

	Label {
		id: myLabel
		color: subsurfaceTheme.textColor
		font.pointSize: subsurfaceTheme.titlePointSize
		font.bold: true
		anchors.horizontalCenter: parent.horizontalCenter
		text: "ups"
	}
	TemplateLine {
		anchors.top: myLabel.bottom
		anchors.left: parent.left
		anchors.right: parent.right
	}
}

