// SPDX-License-Identifier: GPL-2.0
#include "preferences_defaults.h"
#include "ui_preferences_defaults.h"
#include "core/dive.h"
#include "core/settings/qPrefGeneral.h"
#include "core/settings/qPrefDisplay.h"
#include "core/settings/qPrefCloudStorage.h"
#include "core/settings/qPrefDiveComputer.h"

#include <QFileDialog>
#include <QProcess>
#include <QMessageBox>

PreferencesDefaults::PreferencesDefaults(): AbstractPreferencesWidget(tr("General"), QIcon(":preferences-other-icon"), 0 ), ui(new Ui::PreferencesDefaults())
{
	ui->setupUi(this);
}

PreferencesDefaults::~PreferencesDefaults()
{
	delete ui;
}

void PreferencesDefaults::on_chooseFile_clicked()
{
	QFileInfo fi(system_default_filename());
	QString choosenFileName = QFileDialog::getOpenFileName(this, tr("Open default log file"), fi.absolutePath(), tr("Subsurface files") + " (*.ssrf *.xml)");

	if (!choosenFileName.isEmpty())
		ui->defaultfilename->setText(choosenFileName);
}

void PreferencesDefaults::on_btnUseDefaultFile_toggled(bool toggle)
{
	if (toggle) {
		ui->defaultfilename->setText(system_default_filename());
		ui->defaultfilename->setEnabled(false);
	} else {
		ui->defaultfilename->setEnabled(true);
	}
}

void PreferencesDefaults::on_localDefaultFile_toggled(bool toggle)
{
	ui->defaultfilename->setEnabled(toggle);
	ui->btnUseDefaultFile->setEnabled(toggle);
	ui->chooseFile->setEnabled(toggle);
}

void PreferencesDefaults::on_resetRememberedDCs_clicked()
{
	qPrefDiveComputer::set_vendor1(QString());
	qPrefDiveComputer::set_vendor2(QString());
	qPrefDiveComputer::set_vendor3(QString());
	qPrefDiveComputer::set_vendor4(QString());
}

void PreferencesDefaults::on_resetSettings_clicked()
{
	// apparently this button was never hooked up?
}

void PreferencesDefaults::refreshSettings()
{
	ui->font->setCurrentFont(qPrefDisplay::divelist_font());
	ui->fontsize->setValue(qPrefDisplay::font_size());
	ui->defaultfilename->setText(qPrefGeneral::default_filename());
	ui->noDefaultFile->setChecked(qPrefGeneral::default_file_behavior() == NO_DEFAULT_FILE);
	ui->cloudDefaultFile->setChecked(qPrefGeneral::default_file_behavior() == CLOUD_DEFAULT_FILE);
	ui->localDefaultFile->setChecked(qPrefGeneral::default_file_behavior() == LOCAL_DEFAULT_FILE);

	ui->displayinvalid->setChecked(qPrefDisplay::display_invalid_dives());
	ui->velocitySlider->setValue(qPrefDisplay::animation_speed());
	ui->btnUseDefaultFile->setChecked(qPrefGeneral::use_default_file());

	if (qPrefCloudStorage::cloud_verification_status() == qPrefCloudStorage::CS_VERIFIED) {
		ui->cloudDefaultFile->setEnabled(true);
	} else {
		if (ui->cloudDefaultFile->isChecked())
			ui->noDefaultFile->setChecked(true);
		ui->cloudDefaultFile->setEnabled(false);
	}

	ui->defaultfilename->setEnabled(qPrefGeneral::default_file_behavior() == LOCAL_DEFAULT_FILE);
	ui->btnUseDefaultFile->setEnabled(qPrefGeneral::default_file_behavior() == LOCAL_DEFAULT_FILE);
	ui->chooseFile->setEnabled(qPrefGeneral::default_file_behavior() == LOCAL_DEFAULT_FILE);

	ui->extraEnvironmentalDefault->setChecked(prefs.extraEnvironmentalDefault);
}

void PreferencesDefaults::syncSettings()
{
	auto general = qPrefGeneral::instance();
	general->set_default_filename(ui->defaultfilename->text());
	general->set_use_default_file(ui->btnUseDefaultFile->isChecked());
	if (ui->noDefaultFile->isChecked())
		general->set_default_file_behavior(NO_DEFAULT_FILE);
	else if (ui->localDefaultFile->isChecked())
		general->set_default_file_behavior(LOCAL_DEFAULT_FILE);
	else if (ui->cloudDefaultFile->isChecked())
		general->set_default_file_behavior(CLOUD_DEFAULT_FILE);

	qPrefDisplay::set_divelist_font(ui->font->currentFont().toString());
	qPrefDisplay::set_font_size(ui->fontsize->value());
	qPrefDisplay::set_display_invalid_dives(ui->displayinvalid->isChecked());
	qPrefDisplay::set_animation_speed(ui->velocitySlider->value());
	qPrefGeneral::set_extraEnvironmentalDefault(ui->extraEnvironmentalDefault->isChecked());
}
