// SPDX-License-Identifier: GPL-2.0
#include "testrenumber.h"
#include "core/dive.h"
#include "core/divelist.h"
#include "core/file.h"
#include <QTextStream>

void TestRenumber::setup()
{
	QCOMPARE(parse_file(SUBSURFACE_TEST_DATA "/dives/test47.xml", &dive_table), 0);
	process_loaded_dives();
	dive_table.preexisting = dive_table.nr;
}

void TestRenumber::testMerge()
{
	struct dive_table table = { 0 };
	QCOMPARE(parse_file(SUBSURFACE_TEST_DATA "/dives/test47b.xml", &table), 0);
	process_imported_dives(&table, false);
	QCOMPARE(dive_table.nr, 1);
	QCOMPARE(unsaved_changes(), 1);
	mark_divelist_changed(false);
	dive_table.preexisting = dive_table.nr;
}

void TestRenumber::testMergeAndAppend()
{
	struct dive_table table = { 0 };
	QCOMPARE(parse_file(SUBSURFACE_TEST_DATA "/dives/test47c.xml", &table), 0);
	process_imported_dives(&table, false);
	QCOMPARE(dive_table.nr, 2);
	QCOMPARE(unsaved_changes(), 1);
	struct dive *d = get_dive(1);
	QVERIFY(d != NULL);
	if (d)
		QCOMPARE(d->number, 2);
}

QTEST_GUILESS_MAIN(TestRenumber)
