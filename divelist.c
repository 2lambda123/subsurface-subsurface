#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "divelist.h"
#include "dive.h"
#include "display.h"

/*
 * The dive list has the dive data in both string format (for showing)
 * and in "raw" format (for sorting purposes)
 */
enum {
	DIVE_INDEX = 0,
	DIVE_DATESTR,		/* "Thu Jun 17th, 2011" or whatever */
	DIVE_DATE,		/* time_t: dive->when */
	DIVE_DEPTHSTR,		/* "67" in ft or whatever */
	DIVE_DEPTH,		/* int: dive->maxdepth in mm */
	DIVE_DURATIONSTR,	/* "47" in minutes */
	DIVE_DURATION,		/* int: in seconds */
	DIVE_TEMPSTR,		/* "78" in fahrenheit or whatever */
	DIVE_TEMP,		/* int: in mkelvin */
	DIVE_NITROXSTR,		/* "32.5" in percent */
	DIVE_NITROX,		/* int: in permille */
	DIVE_SACSTR,		/* "0.49" in cuft/min */
	DIVE_SAC,		/* int: in ml/min or something */
	DIVELIST_COLUMNS
};


static void selection_cb(GtkTreeSelection *selection, GtkTreeModel *model)
{
	GtkTreeIter iter;
	GValue value = {0, };

	if (!gtk_tree_selection_get_selected(selection, NULL, &iter))
		return;

	gtk_tree_model_get_value(model, &iter, DIVE_INDEX, &value);
	selected_dive = g_value_get_int(&value);
	repaint_dive();
}

static void get_temp(struct dive *dive, int *val, char **str)
{
	int value = dive->watertemp.mkelvin;
	char buffer[80];

	*val = value;
	*str = "";
	if (value) {
		double deg;
		switch (output_units.temperature) {
		case CELSIUS:
			deg = mkelvin_to_C(value);
			break;
		case FAHRENHEIT:
			deg = mkelvin_to_F(value);
			break;
		default:
			return;
		}
		snprintf(buffer, sizeof(buffer), "%.1f", deg);
		*str = strdup(buffer);
	}
}

static void get_nitrox(struct dive *dive, int *val, char **str)
{
	int value = dive->cylinder[0].gasmix.o2.permille;
	char buffer[80];

	*val = value;
	*str = "";
	if (value) {
		snprintf(buffer, sizeof(buffer), "%.1f", value/10.0);
		*str = strdup(buffer);
	}
}

static void get_sac(struct dive *dive, int *val, char **str)
{
	*val = 0;
	*str = "";
}

static gboolean set_one_dive(GtkTreeModel *model,
			     GtkTreePath *path,
			     GtkTreeIter *iter,
			     gpointer data)
{
	int len;
	GValue value = {0, };
	struct dive *dive;
	char buffer[256], *datestr, *depth, *duration;
	struct tm *tm;
	int integer, frac;
	int temp, nitrox, sac;
	char *tempstr, *nitroxstr, *sacstr;

	/* Get the dive number */
	gtk_tree_model_get_value(model, iter, DIVE_INDEX, &value);
	dive = get_dive(g_value_get_int(&value));
	if (!dive)
		return TRUE;

	tm = gmtime(&dive->when);
	len = snprintf(buffer, sizeof(buffer),
		"%02d.%02d.%02d %02d:%02d",
		tm->tm_mday, tm->tm_mon+1, tm->tm_year % 100,
		tm->tm_hour, tm->tm_min);
	datestr = malloc(len+1);
	memcpy(datestr, buffer, len+1);

	switch (output_units.length) {
	unsigned int depth;
	case METERS:
		depth = (dive->maxdepth.mm + 49) / 100;
		integer = depth / 10;
		frac = depth % 10;
		if (integer < 20)
			break;
		frac = -1;
		/* Rounding? */
		break;
	case FEET:
		integer = to_feet(dive->maxdepth);
		frac = -1;
	}
	len = snprintf(buffer, sizeof(buffer),
	               "%d", integer);
	if (frac >= 0)
		len += snprintf(buffer+len, sizeof(buffer)-len,
			".%d", frac);
	depth = malloc(len + 1);
	memcpy(depth, buffer, len+1);

	len = snprintf(buffer, sizeof(buffer),
	               "%d", dive->duration.seconds / 60);
	duration = malloc(len + 1);
	memcpy(duration, buffer, len+1);

	get_temp(dive, &temp, &tempstr);
	get_nitrox(dive, &nitrox, &nitroxstr);
	get_sac(dive, &sac, &sacstr);

	/*
	 * We only set the fields that changed: the strings.
	 * The core data itself is unaffected by units
	 */
	gtk_list_store_set(GTK_LIST_STORE(model), iter,
		DIVE_DATESTR, datestr,
		DIVE_DEPTHSTR, depth,
		DIVE_DURATIONSTR, duration,
		DIVE_TEMPSTR, tempstr,
		DIVE_TEMP, temp,
		DIVE_NITROXSTR, nitroxstr,
		DIVE_NITROX, nitrox,
		DIVE_SACSTR, sacstr,
		DIVE_NITROX, sac,
		-1);

	return FALSE;
}

void update_dive_list_units(struct DiveList *dive_list)
{
	const char *unit;
	GtkTreeModel *model = GTK_TREE_MODEL(dive_list->model);

	switch (output_units.length) {
	case METERS:
		unit = "m";
		break;
	case FEET:
		unit = "ft";
		break;
	}
	gtk_tree_view_column_set_title(dive_list->depth, unit);

	gtk_tree_model_foreach(model, set_one_dive, NULL);
}

static void fill_dive_list(struct DiveList *dive_list)
{
	int i;
	GtkTreeIter iter;
	GtkListStore *store;

	store = GTK_LIST_STORE(dive_list->model);

	for (i = 0; i < dive_table.nr; i++) {
		struct dive *dive = dive_table.dives[i];

		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter,
			DIVE_INDEX, i,
			DIVE_DATESTR, "date",
			DIVE_DATE, dive->when,
			DIVE_DEPTHSTR, "depth",
			DIVE_DEPTH, dive->maxdepth,
			DIVE_DURATIONSTR, "duration",
			DIVE_DURATION, dive->duration.seconds,
			DIVE_TEMPSTR, "temp",
			DIVE_TEMP, dive->watertemp.mkelvin,
			DIVE_NITROXSTR, "21.0",
			DIVE_NITROX, dive->cylinder[0].gasmix.o2,
			DIVE_SACSTR, "sac",
			DIVE_SAC, 0,
			-1);
	}

	update_dive_list_units(dive_list);
}

void dive_list_update_dives(struct DiveList dive_list)
{
	gtk_list_store_clear(GTK_LIST_STORE(dive_list.model));
	fill_dive_list(&dive_list);
	repaint_dive();
}

struct DiveList dive_list_create(void)
{
	struct DiveList    dive_list;
	GtkTreeSelection  *selection;
	GtkCellRenderer   *renderer;
	GtkTreeViewColumn *col;

	dive_list.model = gtk_list_store_new(DIVELIST_COLUMNS,
				G_TYPE_INT,			/* index */
				G_TYPE_STRING, G_TYPE_INT,	/* Date */
	                        G_TYPE_STRING, G_TYPE_INT, 	/* Depth */
	                        G_TYPE_STRING, G_TYPE_INT,	/* Duration */
				G_TYPE_STRING, G_TYPE_INT,	/* Temperature */
				G_TYPE_STRING, G_TYPE_INT,	/* Nitrox */
				G_TYPE_STRING, G_TYPE_INT	/* SAC */
				);
	dive_list.tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(dive_list.model));
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(dive_list.tree_view));

	gtk_tree_selection_set_mode(GTK_TREE_SELECTION(selection), GTK_SELECTION_BROWSE);
	gtk_widget_set_size_request(dive_list.tree_view, 200, 100);

	renderer = gtk_cell_renderer_text_new();
	dive_list.date = col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(col, "Date");
	gtk_tree_view_column_set_sort_column_id(col, DIVE_DATE);
	gtk_tree_view_column_set_resizable (col, TRUE);
	gtk_tree_view_column_pack_start(col, renderer, TRUE);
	gtk_tree_view_column_add_attribute(col, renderer, "text", DIVE_DATESTR);
	gtk_tree_view_append_column(GTK_TREE_VIEW(dive_list.tree_view), col);

	renderer = gtk_cell_renderer_text_new();
	dive_list.depth = col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(col, "ft");
	gtk_tree_view_column_set_sort_column_id(col, DIVE_DEPTH);
	gtk_tree_view_column_pack_start(col, renderer, FALSE);
	gtk_tree_view_column_add_attribute(col, renderer, "text", DIVE_DEPTHSTR);
	gtk_tree_view_append_column(GTK_TREE_VIEW(dive_list.tree_view), col);
	gtk_object_set(GTK_OBJECT(renderer), "alignment", PANGO_ALIGN_RIGHT, NULL);
	gtk_cell_renderer_set_alignment(GTK_CELL_RENDERER(renderer), 1.0, 0.5);

	renderer = gtk_cell_renderer_text_new();
	dive_list.duration = col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(col, "min");
	gtk_tree_view_column_set_sort_column_id(col, DIVE_DURATION);
	gtk_tree_view_column_pack_start(col, renderer, FALSE);
	gtk_tree_view_column_add_attribute(col, renderer, "text", DIVE_DURATIONSTR);
	gtk_tree_view_append_column(GTK_TREE_VIEW(dive_list.tree_view), col);
	gtk_object_set(GTK_OBJECT(renderer), "alignment", PANGO_ALIGN_RIGHT, NULL);
	gtk_cell_renderer_set_alignment(GTK_CELL_RENDERER(renderer), 1.0, 0.5);

	renderer = gtk_cell_renderer_text_new();
	dive_list.temperature = col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(col, "deg");
	gtk_tree_view_column_set_sort_column_id(col, DIVE_TEMP);
	gtk_tree_view_column_pack_start(col, renderer, FALSE);
	gtk_tree_view_column_add_attribute(col, renderer, "text", DIVE_TEMPSTR);
	gtk_tree_view_append_column(GTK_TREE_VIEW(dive_list.tree_view), col);
	gtk_object_set(GTK_OBJECT(renderer), "alignment", PANGO_ALIGN_RIGHT, NULL);
	gtk_cell_renderer_set_alignment(GTK_CELL_RENDERER(renderer), 1.0, 0.5);

	renderer = gtk_cell_renderer_text_new();
	dive_list.nitrox = col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(col, "EAD");
	gtk_tree_view_column_set_sort_column_id(col, DIVE_NITROX);
	gtk_tree_view_column_pack_start(col, renderer, FALSE);
	gtk_tree_view_column_add_attribute(col, renderer, "text", DIVE_NITROXSTR);
	gtk_tree_view_append_column(GTK_TREE_VIEW(dive_list.tree_view), col);
	gtk_object_set(GTK_OBJECT(renderer), "alignment", PANGO_ALIGN_RIGHT, NULL);
	gtk_cell_renderer_set_alignment(GTK_CELL_RENDERER(renderer), 1.0, 0.5);

	renderer = gtk_cell_renderer_text_new();
	dive_list.sac = col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(col, "SAC");
	gtk_tree_view_column_set_sort_column_id(col, DIVE_SAC);
	gtk_tree_view_column_pack_start(col, renderer, FALSE);
	gtk_tree_view_column_add_attribute(col, renderer, "text", DIVE_SACSTR);
	gtk_tree_view_append_column(GTK_TREE_VIEW(dive_list.tree_view), col);
	gtk_object_set(GTK_OBJECT(renderer), "alignment", PANGO_ALIGN_RIGHT, NULL);
	gtk_cell_renderer_set_alignment(GTK_CELL_RENDERER(renderer), 1.0, 0.5);

	fill_dive_list(&dive_list);

	g_object_set(G_OBJECT(dive_list.tree_view), "headers-visible", TRUE,
					  "search-column", 0,
					  "rules-hint", TRUE,
					  NULL);

	g_signal_connect(selection, "changed", G_CALLBACK(selection_cb), dive_list.model);

	dive_list.container_widget = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(dive_list.container_widget),
		               GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(dive_list.container_widget), dive_list.tree_view);

	return dive_list;
}
