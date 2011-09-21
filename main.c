/* main.c */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <gconf/gconf-client.h>

#include "dive.h"
#include "divelist.h"

GConfClient *gconf;
struct units output_units;

#define GCONF_NAME(x) "/apps/subsurface/" #x

/* random helper functions, used here or elsewhere */
static int sortfn(const void *_a, const void *_b)
{
	const struct dive *a = *(void **)_a;
	const struct dive *b = *(void **)_b;

	if (a->when < b->when)
		return -1;
	if (a->when > b->when)
		return 1;
	return 0;
}

const char *weekday(int wday)
{
	static const char wday_array[7][4] = {
		"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
	};
	return wday_array[wday];
}

const char *monthname(int mon)
{
	static const char month_array[12][4] = {
		"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
	};
	return month_array[mon];
}

/*
 * This doesn't really report anything at all. We just sort the
 * dives, the GUI does the reporting
 */
void report_dives(void)
{
	int i;

	qsort(dive_table.dives, dive_table.nr, sizeof(struct dive *), sortfn);

	for (i = 1; i < dive_table.nr; i++) {
		struct dive **pp = &dive_table.dives[i-1];
		struct dive *prev = pp[0];
		struct dive *dive = pp[1];
		struct dive *merged;

		if (prev->when + prev->duration.seconds < dive->when)
			continue;

		merged = try_to_merge(prev, dive);
		if (!merged)
			continue;

		free(prev);
		free(dive);
		*pp = merged;
		dive_table.nr--;
		memmove(pp+1, pp+2, sizeof(*pp)*(dive_table.nr - i));

		/* Redo the new 'i'th dive */
		i--;
	}
}

static void parse_argument(const char *arg)
{
	const char *p = arg+1;

	do {
		switch (*p) {
		case 'v':
			verbose++;
			continue;
		default:
			fprintf(stderr, "Bad argument '%s'\n", arg);
			exit(1);
		}
	} while (*++p);
}

void update_dive(struct dive *new_dive)
{
	static struct dive *buffered_dive;
	struct dive *old_dive = buffered_dive;

	if (old_dive) {
		flush_dive_info_changes(old_dive);
		flush_dive_equipment_changes(old_dive);
		flush_divelist(old_dive);
	}
	if (new_dive) {
		show_dive_info(new_dive);
		show_dive_equipment(new_dive);
	}
	buffered_dive = new_dive;
}

void renumber_dives(int nr)
{
	int i;

	for (i = 0; i < dive_table.nr; i++) {
		struct dive *dive = dive_table.dives[i];
		dive->number = nr + i;
	}
}

int main(int argc, char **argv)
{
	int i;

	output_units = SI_units;
	parse_xml_init();

	init_ui(argc, argv);
	
	for (i = 1; i < argc; i++) {
		const char *a = argv[i];

		if (a[0] == '-') {
			parse_argument(a);
			continue;
		}
		GError *error = NULL;
		parse_xml_file(a, &error);
		
		if (error != NULL)
		{
			report_error(error);
			g_error_free(error);
			error = NULL;
		}
	}

	report_dives();
	dive_list_update_dives();

	run_ui();
	return 0;
}
