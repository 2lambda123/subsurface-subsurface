// SPDX-License-Identifier: GPL-2.0
#ifndef PREFERENCES_DEFAULTS_H
#define PREFERENCES_DEFAULTS_H

#include "abstractpreferenceswidget.h"
#include "core/pref.h"

namespace Ui {
	class PreferencesDefaults;
}

class PreferencesDefaults : public AbstractPreferencesWidget {
	Q_OBJECT
public:
	PreferencesDefaults();
	~PreferencesDefaults();
	void refreshSettings() override;
	void syncSettings() override;
public slots:
	void on_chooseFile_clicked();
	void on_btnUseDefaultFile_toggled(bool toggled);
	void on_localDefaultFile_toggled(bool toggled);
	void on_resetSettings_clicked();

private:
	Ui::PreferencesDefaults *ui;
};


#endif
