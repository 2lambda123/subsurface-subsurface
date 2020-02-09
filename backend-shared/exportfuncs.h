// SPDX-License-Identifier: GPL-2.0
#ifndef EXPORTFUNCS_H
#define EXPORTFUNCS_H

#include <QString>
#include <QFuture>

struct dive_site;

void exportProfile(QString filename, bool selected_only);
void export_TeX(const char *filename, bool selected_only, bool plain);
void export_depths(const char *filename, bool selected_only);
std::vector<const dive_site *> getDiveSitesToExport(bool selectedOnly);
QFuture<int> exportUsingStyleSheet(QString filename, bool doExport, int units, QString stylesheet, bool anonymize);

// prepareDivesForUploadDiveLog
// prepareDivesForUploadDiveShare

// WARNING
// exportProfile uses the UI and are therefore different between
// Desktop (UI) and Mobile (QML)
// In order to solve this difference, the actual implementations
// are done in
// desktop-widgets/divelogexportdialog.cpp and
// mobile-widgets/qmlmanager.cpp
void exportProfile(const struct dive *dive, const QString filename);

#endif // EXPORT_FUNCS_H
