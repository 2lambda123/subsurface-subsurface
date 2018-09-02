// SPDX-License-Identifier: GPL-2.0
#include "qPrefPrivate.h" // Include first, to make prefs mutable
#include "qPrefTechnicalDetails.h"



static const QString group = QStringLiteral("TecDetails");

qPrefTechnicalDetails::qPrefTechnicalDetails(QObject *parent) : QObject(parent)
{
}

qPrefTechnicalDetails *qPrefTechnicalDetails::instance()
{
	static qPrefTechnicalDetails *self = new qPrefTechnicalDetails;
	return self;
}


void qPrefTechnicalDetails::loadSync(bool doSync)
{
	disk_calcalltissues(doSync);
	disk_calcceiling(doSync);
	disk_calcceiling3m(doSync);
	disk_calcndltts(doSync);
	disk_dcceiling(doSync);
	disk_display_deco_mode(doSync);
	disk_display_unused_tanks(doSync);
	disk_ead(doSync);
	disk_gfhigh(doSync);
	disk_gflow(doSync);
	disk_gf_low_at_maxdepth(doSync);
	disk_hrgraph(doSync);
	disk_mod(doSync);
	disk_modpO2(doSync);
	disk_percentagegraph(doSync);
	disk_redceiling(doSync);
	disk_rulergraph(doSync);
	disk_show_average_depth(doSync);
	disk_show_ccr_sensors(doSync);
	disk_show_ccr_setpoint(doSync);
	disk_show_icd(doSync);
	disk_show_pictures_in_profile(doSync);
	disk_show_sac(doSync);
	disk_show_scr_ocpo2(doSync);
	disk_tankbar(doSync);
	disk_vpmb_conservatism(doSync);
	disk_zoomed_plot(doSync);
}

HANDLE_PREFERENCE_BOOL(TechnicalDetails, "calcalltissues", calcalltissues);

HANDLE_PREFERENCE_BOOL(TechnicalDetails, "calcceiling", calcceiling);

HANDLE_PREFERENCE_BOOL(TechnicalDetails, "calcceiling3m", calcceiling3m);

HANDLE_PREFERENCE_BOOL(TechnicalDetails, "calcndltts", calcndltts);

HANDLE_PREFERENCE_BOOL(TechnicalDetails, "dcceiling", dcceiling);

HANDLE_PREFERENCE_ENUM(TechnicalDetails, deco_mode, "display_deco_mode", display_deco_mode);

HANDLE_PREFERENCE_BOOL(TechnicalDetails, "display_unused_tanks", display_unused_tanks);

HANDLE_PREFERENCE_BOOL(TechnicalDetails, "ead", ead);

HANDLE_PREFERENCE_INT(TechnicalDetails, "gfhigh", gfhigh);

HANDLE_PREFERENCE_INT(TechnicalDetails, "gflow", gflow);

HANDLE_PREFERENCE_BOOL(TechnicalDetails, "gf_low_at_maxdepth", gf_low_at_maxdepth);

HANDLE_PREFERENCE_BOOL(TechnicalDetails, "hrgraph", hrgraph);

HANDLE_PREFERENCE_BOOL(TechnicalDetails, "mod", mod);

HANDLE_PREFERENCE_DOUBLE(TechnicalDetails, "modpO2", modpO2);

HANDLE_PREFERENCE_BOOL(TechnicalDetails, "percentagegraph", percentagegraph);

HANDLE_PREFERENCE_BOOL(TechnicalDetails, "redceiling", redceiling);

HANDLE_PREFERENCE_BOOL(TechnicalDetails, "RulerBar", rulergraph);

HANDLE_PREFERENCE_BOOL(TechnicalDetails, "show_average_depth", show_average_depth);

HANDLE_PREFERENCE_BOOL(TechnicalDetails, "show_ccr_sensors", show_ccr_sensors);

HANDLE_PREFERENCE_BOOL(TechnicalDetails, "show_ccr_setpoint", show_ccr_setpoint);

HANDLE_PREFERENCE_BOOL(TechnicalDetails, "show_icd", show_icd);

HANDLE_PREFERENCE_BOOL(TechnicalDetails, "show_pictures_in_profile", show_pictures_in_profile);

HANDLE_PREFERENCE_BOOL(TechnicalDetails, "show_sac", show_sac);

HANDLE_PREFERENCE_BOOL(TechnicalDetails, "show_scr_ocpo2", show_scr_ocpo2);

HANDLE_PREFERENCE_BOOL(TechnicalDetails, "tankbar", tankbar);

HANDLE_PREFERENCE_INT(TechnicalDetails, "vpmb_conservatism", vpmb_conservatism);

HANDLE_PREFERENCE_BOOL(TechnicalDetails, "zoomed_plot", zoomed_plot);
