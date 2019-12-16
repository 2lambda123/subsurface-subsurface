// SPDX-License-Identifier: GPL-2.0
#ifndef PLANNERSHARED_H
#define PLANNERSHARED_H
#include <QObject>
#include "core/pref.h"

// This is a shared class (mobile/desktop), and contains the core of the diveplanner
// without UI entanglement.
// It make variables and functions available to QML, these are referenced directly
// in the desktop version
//
// The mobile diveplanner shows all diveplans, but the editing functionality is
// limited to keep the UI simpler.

class plannerShared: public QObject {
	Q_OBJECT

	// Ascend/Descend data, converted to meter/feet depending on user selection
	// Settings these will automatically update the corresponding qPrefDivePlanner
	// Variables
	Q_PROPERTY(int ascratelast6m READ ascratelast6m WRITE set_ascratelast6m NOTIFY ascratelast6mChanged);
	Q_PROPERTY(int ascratestops READ ascratestops WRITE set_ascratestops NOTIFY ascratestopsChanged);
	Q_PROPERTY(int ascrate50 READ ascrate50 WRITE set_ascrate50 NOTIFY ascrate50Changed);
	Q_PROPERTY(int ascrate75 READ ascrate75 WRITE set_ascrate75 NOTIFY ascrate75Changed);
	Q_PROPERTY(int descrate READ descrate WRITE set_descrate NOTIFY descrateChanged);

	// Planning data, no conversion but different origins
	Q_PROPERTY(deco_mode planner_deco_mode READ planner_deco_mode WRITE set_planner_deco_mode NOTIFY planner_deco_modeChanged);
	Q_PROPERTY(bool dobailout READ dobailout WRITE set_dobailout NOTIFY dobailoutChanged);
	Q_PROPERTY(int reserve_gas READ reserve_gas WRITE set_reserve_gas NOTIFY reserve_gasChanged);
	Q_PROPERTY(bool safetystop READ safetystop WRITE set_safetystop NOTIFY safetystopChanged);
	Q_PROPERTY(int gflow READ gflow WRITE set_gflow NOTIFY gflowChanged);
	Q_PROPERTY(int gfhigh READ gfhigh WRITE set_gfhigh NOTIFY gfhighChanged);
	Q_PROPERTY(int vpmb_conservatism READ vpmb_conservatism WRITE set_vpmb_conservatism NOTIFY vpmb_conservatismChanged);

public:
	static plannerShared *instance();

	// Ascend/Descend data, converted to meter/feet depending on user selection
	static int ascratelast6m();
	static int ascratestops();
	static int ascrate50();
	static int ascrate75();
	static int descrate();

	// Planning data, no conversion but different origins
	static deco_mode planner_deco_mode();
	static bool dobailout();
	static int reserve_gas();
	static bool safetystop();
	static int gflow();
	static int gfhigh();
	static int vpmb_conservatism();

public slots:
	// Ascend/Descend data, converted to meter/feet depending on user selection
	static void set_ascratelast6m(int value);
	static void set_ascratestops(int value);
	static void set_ascrate50(int value);
	static void set_ascrate75(int value);
	static void set_descrate(int value);

	// Planning data, no conversion but different origins
	static void set_planner_deco_mode(deco_mode value);
	static void set_dobailout(bool value);
	static void set_reserve_gas(int value);
	static void set_safetystop(bool value);
	static void set_gflow(int value);
	static void set_gfhigh(int value);
	static void set_vpmb_conservatism(int value);

signals:
	// Ascend/Descend data, converted to meter/feet depending on user selection
	void ascratelast6mChanged(int value);
	void ascratestopsChanged(int value);
	void ascrate50Changed(int value);
	void ascrate75Changed(int value);
	void descrateChanged(int value);

	// Planning data, no conversion but different origins
	void planner_deco_modeChanged(deco_mode value);
	void dobailoutChanged(bool value);
	void reserve_gasChanged(int value);
	void safetystopChanged(bool value);
	void gflowChanged(int value);
	void gfhighChanged(int value);
	void vpmb_conservatismChanged(int value);

private:
	plannerShared() {}
};

#endif // PLANNERSHARED_H
