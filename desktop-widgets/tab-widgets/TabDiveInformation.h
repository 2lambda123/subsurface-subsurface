// SPDX-License-Identifier: GPL-2.0
#ifndef TAB_DIVE_INFORMATION_H
#define TAB_DIVE_INFORMATION_H

#include "TabBase.h"
#include "core/subsurface-qt/DiveListNotifier.h"

namespace Ui {
	class TabDiveInformation;
};

class TabDiveInformation : public TabBase {
	Q_OBJECT
public:
	TabDiveInformation(QWidget *parent = 0);
	~TabDiveInformation();
	void updateData() override;
	void clear() override;
private slots:
	void divesChanged(dive_trip *trip, const QVector<dive *> &dives, DiveField field);
private:
	Ui::TabDiveInformation *ui;
	void updateProfile();
	void updateWhen();
};

#endif
