// SPDX-License-Identifier: GPL-2.0
#define PREFERENCES_SOURCE 1 // Make prefs-structure mutable
#include "testqPrefUnits.h"

#include "core/pref.h"
#include "core/qthelper.h"
#include "core/settings/qPrefUnit.h"

#include <QTest>
#include <QSignalSpy>

void TestQPrefUnits::initTestCase()
{
	QCoreApplication::setOrganizationName("Subsurface");
	QCoreApplication::setOrganizationDomain("subsurface.hohndel.org");
	QCoreApplication::setApplicationName("SubsurfaceTestQPrefUnits");
}

void TestQPrefUnits::test_struct_get()
{
	// Test struct pref -> get func.

	auto tst = qPrefUnits::instance();

	prefs.coordinates_traditional = true;
	prefs.units.duration_units = units::MIXED;
	prefs.units.length = units::METERS;
	prefs.units.pressure = units::BAR;
	prefs.units.show_units_table = true;
	prefs.units.temperature = units::CELSIUS;
	prefs.units.vertical_speed_time = units::SECONDS;
	prefs.units.volume = units::LITER;
	prefs.units.weight = units::KG;

	QCOMPARE(tst->coordinates_traditional(), true);
	QCOMPARE(tst->duration_units(), units::MIXED);
	QCOMPARE(tst->length(), units::METERS);
	QCOMPARE(tst->pressure(), units::BAR);
	QCOMPARE(tst->show_units_table(), true);
	QCOMPARE(tst->temperature(), units::CELSIUS);
	QCOMPARE(tst->vertical_speed_time(), units::SECONDS);
	QCOMPARE(tst->volume(), units::LITER);
	QCOMPARE(tst->weight(), units::KG);
}

void TestQPrefUnits::test_set_struct()
{
	// Test set func -> struct pref

	auto tst = qPrefUnits::instance();

	tst->set_coordinates_traditional(false);
	tst->set_duration_units(units::MINUTES_ONLY);
	tst->set_length(units::FEET);
	tst->set_pressure(units::PSI);
	tst->set_show_units_table(false);
	tst->set_temperature(units::FAHRENHEIT);
	tst->set_vertical_speed_time(units::SECONDS);
	tst->set_volume(units::CUFT);
	tst->set_weight(units::LBS);

	QCOMPARE(prefs.coordinates_traditional, false);
	QCOMPARE(prefs.units.duration_units, units::MINUTES_ONLY);
	QCOMPARE(prefs.units.length, units::FEET);
	QCOMPARE(prefs.units.pressure, units::PSI);
	QCOMPARE(prefs.units.show_units_table, false);
	QCOMPARE(prefs.units.temperature, units::FAHRENHEIT);
	QCOMPARE(prefs.units.vertical_speed_time, units::SECONDS);
	QCOMPARE(prefs.units.volume, units::CUFT);
	QCOMPARE(prefs.units.weight, units::LBS);
}

void TestQPrefUnits::test_set_load_struct()
{
	// test set func -> load -> struct pref

	auto tst = qPrefUnits::instance();

	tst->set_coordinates_traditional(true);
	tst->set_duration_units(units::MINUTES_ONLY);
	tst->set_length(units::FEET);
	tst->set_pressure(units::PSI);
	tst->set_show_units_table(false);
	tst->set_temperature(units::FAHRENHEIT);
	tst->set_vertical_speed_time(units::MINUTES);
	tst->set_volume(units::CUFT);
	tst->set_weight(units::LBS);

	tst->sync();
	prefs.coordinates_traditional = false;
	prefs.units.duration_units = units::MIXED;
	prefs.units.length = units::METERS;
	prefs.units.pressure = units::BAR;
	prefs.units.show_units_table = true;
	prefs.units.temperature = units::CELSIUS;
	prefs.units.vertical_speed_time = units::SECONDS;
	prefs.units.volume = units::LITER;
	prefs.units.weight = units::KG;

	tst->load();
	QCOMPARE(prefs.coordinates_traditional, true);
	QCOMPARE(prefs.units.duration_units, units::MINUTES_ONLY);
	QCOMPARE(prefs.units.length, units::FEET);
	QCOMPARE(prefs.units.pressure, units::PSI);
	QCOMPARE(prefs.units.show_units_table, false);
	QCOMPARE(prefs.units.temperature, units::FAHRENHEIT);
	QCOMPARE(prefs.units.vertical_speed_time, units::MINUTES);
	QCOMPARE(prefs.units.volume, units::CUFT);
	QCOMPARE(prefs.units.weight, units::LBS);
}

void TestQPrefUnits::test_struct_disk()
{
	// test struct prefs -> disk

	auto tst = qPrefUnits::instance();

	prefs.coordinates_traditional = true;
	prefs.units.duration_units = units::MIXED;
	prefs.units.length = units::METERS;
	prefs.units.pressure = units::BAR;
	prefs.units.show_units_table = true;
	prefs.units.temperature = units::CELSIUS;
	prefs.units.vertical_speed_time = units::SECONDS;
	prefs.units.volume = units::LITER;
	prefs.units.weight = units::KG;

	tst->sync();
	prefs.coordinates_traditional = false;
	prefs.units.duration_units = units::MINUTES_ONLY;
	prefs.units.length = units::FEET;
	prefs.units.pressure = units::PSI;
	prefs.units.show_units_table = false;
	prefs.units.temperature = units::FAHRENHEIT;
	prefs.units.vertical_speed_time = units::MINUTES;
	prefs.units.volume = units::CUFT;
	prefs.units.weight = units::LBS;

	tst->load();
	QCOMPARE(prefs.coordinates_traditional, true);
	QCOMPARE(prefs.units.duration_units, units::MIXED);
	QCOMPARE(prefs.units.length, units::METERS);
	QCOMPARE(prefs.units.pressure, units::BAR);
	QCOMPARE(prefs.units.show_units_table, true);
	QCOMPARE(prefs.units.temperature, units::CELSIUS);
	QCOMPARE(prefs.units.vertical_speed_time, units::SECONDS);
	QCOMPARE(prefs.units.volume, units::LITER);
	QCOMPARE(prefs.units.weight, units::KG);
}

void TestQPrefUnits::test_multiple()
{
	// test multiple instances have the same information

	prefs.units.length = units::METERS;
	auto tst_direct = new qPrefUnits;

	prefs.units.pressure = units::BAR;
	auto tst = qPrefUnits::instance();

	QCOMPARE(tst->length(), tst_direct->length());
	QCOMPARE(tst->length(), units::METERS);
	QCOMPARE(tst->pressure(), tst_direct->pressure());
	QCOMPARE(tst->pressure(), units::BAR);
}

void TestQPrefUnits::test_unit_system()
{
	// test struct prefs <->  set/get unit_system

	auto tst = qPrefUnits::instance();

	tst->set_unit_system("metric");
	QCOMPARE(prefs.unit_system, METRIC);
	QCOMPARE(tst->unit_system(), QString("metric"));
	tst->set_unit_system("imperial");
	QCOMPARE(prefs.unit_system, IMPERIAL);
	QCOMPARE(tst->unit_system(), QString("imperial"));
	tst->set_unit_system("personalized");
	QCOMPARE(prefs.unit_system, PERSONALIZE);
	QCOMPARE(tst->unit_system(), QString("personalized"));

	prefs.unit_system = METRIC;
	QCOMPARE(tst->unit_system(), QString("metric"));
	prefs.unit_system = IMPERIAL;
	QCOMPARE(tst->unit_system(), QString("imperial"));
	prefs.unit_system = PERSONALIZE;
	QCOMPARE(tst->unit_system(), QString("personalized"));
}

#define TEST(METHOD, VALUE)      \
	QCOMPARE(METHOD, VALUE); \
	units->sync();           \
	units->load();           \
	QCOMPARE(METHOD, VALUE);

void TestQPrefUnits::test_oldPreferences()
{
	auto units = qPrefUnits::instance();

	units->set_length(units::METERS);
	units->set_pressure(units::BAR);
	units->set_volume(units::LITER);
	units->set_temperature(units::CELSIUS);
	units->set_weight(units::KG);
	units->set_unit_system(QStringLiteral("metric"));
	units->set_coordinates_traditional(false);
	units->set_vertical_speed_time(units::SECONDS);

	TEST(units->length(), units::METERS);
	TEST(units->pressure(), units::BAR);
	TEST(units->volume(), units::LITER);
	TEST(units->temperature(), units::CELSIUS);
	TEST(units->weight(), units::KG);
	TEST(units->vertical_speed_time(), units::SECONDS);
	TEST(units->unit_system(), QStringLiteral("metric"));
	TEST(units->coordinates_traditional(), false);

	units->set_length(units::FEET);
	units->set_pressure(units::PSI);
	units->set_volume(units::CUFT);
	units->set_temperature(units::FAHRENHEIT);
	units->set_weight(units::LBS);
	units->set_vertical_speed_time(units::MINUTES);
	units->set_unit_system(QStringLiteral("fake-metric-system"));
	units->set_coordinates_traditional(true);

	TEST(units->length(), units::FEET);
	TEST(units->pressure(), units::PSI);
	TEST(units->volume(), units::CUFT);
	TEST(units->temperature(), units::FAHRENHEIT);
	TEST(units->weight(), units::LBS);
	TEST(units->vertical_speed_time(), units::MINUTES);
	TEST(units->unit_system(), QStringLiteral("personalized"));
	TEST(units->coordinates_traditional(), true);
}

void TestQPrefUnits::test_signals()
{
	QSignalSpy spy1(qPrefUnits::instance(), SIGNAL(coordinates_traditionalChanged(bool)));
	QSignalSpy spy2(qPrefUnits::instance(), SIGNAL(duration_unitsChanged(int)));
	QSignalSpy spy3(qPrefUnits::instance(), SIGNAL(lengthChanged(int)));
	QSignalSpy spy4(qPrefUnits::instance(), SIGNAL(pressureChanged(int)));
	QSignalSpy spy5(qPrefUnits::instance(), SIGNAL(show_units_tableChanged(bool)));
	QSignalSpy spy6(qPrefUnits::instance(), SIGNAL(temperatureChanged(int)));
	QSignalSpy spy7(qPrefUnits::instance(), SIGNAL(vertical_speed_timeChanged(int)));
	QSignalSpy spy8(qPrefUnits::instance(), SIGNAL(volumeChanged(int)));
	QSignalSpy spy9(qPrefUnits::instance(), SIGNAL(weightChanged(int)));

	prefs.coordinates_traditional = true;
	qPrefUnits::set_coordinates_traditional(false);
	prefs.units.duration_units = units::MIXED;
	qPrefUnits::set_duration_units(units::MINUTES_ONLY);
	prefs.units.length = units::METERS;
	qPrefUnits::set_length(units::FEET);
	prefs.units.pressure = units::BAR;
	qPrefUnits::set_pressure(units::PSI);
	prefs.units.show_units_table = true;
	qPrefUnits::set_show_units_table(false);
	prefs.units.temperature = units::CELSIUS;
	qPrefUnits::set_temperature(units::FAHRENHEIT);
	prefs.units.vertical_speed_time = units::MINUTES;
	qPrefUnits::set_vertical_speed_time(units::SECONDS);
	prefs.units.volume = units::LITER;
	qPrefUnits::set_volume(units::CUFT);
	prefs.units.weight = units::KG;
	qPrefUnits::set_weight(units::LBS);

	QCOMPARE(spy1.count(), 1);
	QCOMPARE(spy2.count(), 1);
	QCOMPARE(spy3.count(), 1);
	QCOMPARE(spy4.count(), 1);
	QCOMPARE(spy5.count(), 1);
	QCOMPARE(spy6.count(), 1);
	QCOMPARE(spy7.count(), 1);
	QCOMPARE(spy9.count(), 1);
	QCOMPARE(spy9.count(), 1);

	QVERIFY(spy1.takeFirst().at(0).toBool() == false);
	QVERIFY(spy2.takeFirst().at(0).toInt() == units::MINUTES_ONLY);
	QVERIFY(spy3.takeFirst().at(0).toInt() == units::FEET);
	QVERIFY(spy4.takeFirst().at(0).toInt() == units::PSI);
	QVERIFY(spy5.takeFirst().at(0).toBool() == false);
	QVERIFY(spy6.takeFirst().at(0).toInt() == units::FAHRENHEIT);
	QVERIFY(spy7.takeFirst().at(0).toInt() == units::SECONDS);
	QVERIFY(spy8.takeFirst().at(0).toInt() == units::CUFT);
	QVERIFY(spy9.takeFirst().at(0).toInt() == units::LBS);
}


QTEST_MAIN(TestQPrefUnits)
