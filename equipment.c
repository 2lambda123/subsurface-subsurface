/* equipment.c */
/* creates the UI for the equipment page -
 * controlled through the following interfaces:
 * 
 * void show_dive_equipment(struct dive *dive)
 * void flush_dive_equipment_changes(struct dive *dive)
 *
 * called from gtk-ui:
 * GtkWidget *equipment_widget(void)
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>

#include "dive.h"
#include "display.h"
#include "display-gtk.h"
#include "divelist.h"

struct cylinder_widget {
	int index, changed;
	const char *name;
	GtkWidget *hbox;
	GtkComboBox *description;
	GtkSpinButton *size, *pressure;
	GtkWidget *o2, *gasmix_button;
};

static struct cylinder_widget gtk_cylinder[MAX_CYLINDERS];

static void set_cylinder_spinbuttons(struct cylinder_widget *cylinder, int ml, int mbar)
{
	double volume, pressure;

	volume = ml / 1000.0;
	pressure = mbar / 1000.0;
	if (mbar) {
		if (output_units.volume == CUFT) {
			volume /= 28.3168466;	/* Liters to cuft */
			volume *= pressure / 1.01325;
		}
		if (output_units.pressure == PSI) {
			pressure *= 14.5037738;	/* Bar to PSI */
		}
	}

	gtk_spin_button_set_value(cylinder->size, volume);
	gtk_spin_button_set_value(cylinder->pressure, pressure);
}

static void cylinder_cb(GtkComboBox *combo_box, gpointer data)
{
	GtkTreeIter iter;
	GtkTreeModel *model = gtk_combo_box_get_model(combo_box);
	GValue value1 = {0, }, value2 = {0,};
	struct cylinder_widget *cylinder = data;
	cylinder_t *cyl = current_dive->cylinder + cylinder->index;

	/* Did the user set it to some non-standard value? */
	if (!gtk_combo_box_get_active_iter(combo_box, &iter)) {
		cylinder->changed = 1;
		return;
	}

	/*
	 * We get "change" signal callbacks just because we set
	 * the description by hand. Whatever. So ignore them if
	 * they are no-ops.
	 */
	if (!cylinder->changed && cyl->type.description) {
		int same;
		char *desc = gtk_combo_box_get_active_text(combo_box);

		same = !strcmp(desc, cyl->type.description);
		g_free(desc);
		if (same)
			return;
	}
	cylinder->changed = 1;

	gtk_tree_model_get_value(model, &iter, 1, &value1);
	gtk_tree_model_get_value(model, &iter, 2, &value2);

	set_cylinder_spinbuttons(cylinder, g_value_get_int(&value1), g_value_get_int(&value2));
}

/*
 * The gtk_tree_model_foreach() interface is bad. It could have
 * returned whether the callback ever returned true
 */
static int found_match = 0;

static gboolean match_cylinder(GtkTreeModel *model,
				GtkTreePath *path,
				GtkTreeIter *iter,
				gpointer data)
{
	const char *name;
	struct cylinder_widget *cylinder = data;
	GValue value = {0, };

	gtk_tree_model_get_value(model, iter, 0, &value);
	name = g_value_get_string(&value);
	if (strcmp(cylinder->name, name))
		return FALSE;
	gtk_combo_box_set_active_iter(cylinder->description, iter);
	found_match = 1;
	return TRUE;
}

static void add_cylinder(struct cylinder_widget *cylinder, const char *desc, int ml, int mbar)
{
	GtkTreeModel *model;

	found_match = 0;
	model = gtk_combo_box_get_model(cylinder->description);
	cylinder->name = desc;
	gtk_tree_model_foreach(model, match_cylinder, cylinder);

	if (!found_match) {
		GtkListStore *store = GTK_LIST_STORE(model);
		GtkTreeIter iter;

		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter,
			0, desc,
			1, ml,
			2, mbar,
			-1);
		gtk_combo_box_set_active_iter(cylinder->description, &iter);
	}
}

static void show_cylinder(cylinder_t *cyl, struct cylinder_widget *cylinder)
{
	const char *desc;
	int ml, mbar;
	double o2;

	/* Don't show uninitialized cylinder widgets */
	if (!cylinder->description)
		return;

	desc = cyl->type.description;
	if (!desc)
		desc = "";
	ml = cyl->type.size.mliter;
	mbar = cyl->type.workingpressure.mbar;
	add_cylinder(cylinder, desc, ml, mbar);

	set_cylinder_spinbuttons(cylinder, cyl->type.size.mliter, cyl->type.workingpressure.mbar);
	o2 = cyl->gasmix.o2.permille / 10.0;
	gtk_widget_set_sensitive(cylinder->o2, !!o2);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cylinder->gasmix_button), !!o2);
	if (!o2)
		o2 = 21.0;
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(cylinder->o2), o2);
}

void show_dive_equipment(struct dive *dive)
{
	int i;

	for (i = 0; i < MAX_CYLINDERS; i++)
		show_cylinder(dive->cylinder + i, gtk_cylinder+i);
}

static GtkWidget *create_spinbutton(GtkWidget *vbox, const char *name, double min, double max, double incr)
{
	GtkWidget *frame, *hbox, *button;

	frame = gtk_frame_new(name);
	gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, TRUE, 0);

	hbox = gtk_hbox_new(FALSE, 3);
	gtk_container_add(GTK_CONTAINER(frame), hbox);

	button = gtk_spin_button_new_with_range(min, max, incr);
	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, TRUE, 0);

	gtk_spin_button_set_update_policy(GTK_SPIN_BUTTON(button), GTK_UPDATE_IF_VALID);

	return button;
}

static void fill_cylinder_info(struct cylinder_widget *cylinder, cylinder_t *cyl, const char *desc, double volume, double pressure, int o2)
{
	int mbar, ml;

	if (output_units.pressure == PSI)
		pressure /= 14.5037738;

	if (pressure && output_units.volume == CUFT) {
		volume *= 28.3168466;	/* CUFT to liter */
		volume /= pressure / 1.01325;
	}

	ml = volume * 1000 + 0.5;
	mbar = pressure * 1000 + 0.5;

	if (o2 < 211)
		o2 = 0;
	cyl->type.description = desc;
	cyl->type.size.mliter = ml;
	cyl->type.workingpressure.mbar = mbar;
	cyl->gasmix.o2.permille = o2;

	/*
	 * Also, insert it into the model if it doesn't already exist
	 */
	add_cylinder(cylinder, desc, ml, mbar);
}

static void record_cylinder_changes(cylinder_t *cyl, struct cylinder_widget *cylinder)
{
	const gchar *desc;
	GtkComboBox *box;
	double volume, pressure;
	int o2;

	/* Ignore uninitialized cylinder widgets */
	box = cylinder->description;
	if (!box)
		return;

	desc = gtk_combo_box_get_active_text(box);
	volume = gtk_spin_button_get_value(cylinder->size);
	pressure = gtk_spin_button_get_value(cylinder->pressure);
	o2 = gtk_spin_button_get_value(GTK_SPIN_BUTTON(cylinder->o2))*10 + 0.5;
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(cylinder->gasmix_button)))
		o2 = 0;
	fill_cylinder_info(cylinder, cyl, desc, volume, pressure, o2);
}

void flush_dive_equipment_changes(struct dive *dive)
{
	/* We do nothing: we require the "Ok" button press */
}

static void apply_cb(GtkButton *button, gpointer data)
{
	int i;
	struct dive *dive = current_dive;

	if (!dive)
		return;

	for (i = 0; i < MAX_CYLINDERS; i++)
		record_cylinder_changes(dive->cylinder+i, gtk_cylinder+i);
	mark_divelist_changed(TRUE);
	flush_divelist(dive);
}

static void cancel_cb(GtkButton *button, gpointer data)
{
	struct dive *dive = current_dive;

	if (!dive)
		return;

	show_dive_equipment(current_dive);
}

/*
 * We hardcode the most common standard cylinders,
 * we should pick up any other names from the dive
 * logs directly.
 */
static struct tank_info {
	const char *name;
	int size;	/* cuft if < 1000, otherwise mliter */
	int psi;	/* If zero, size is in mliter */
} tank_info[100] = {
	/* Need an empty entry for the no-cylinder case */
	{ "", 0, 0 },

	/* Size-only metric cylinders */
	{ "10.0 l", 10000 },
	{ "11.1 l", 11100 },

	/* Most common AL cylinders */
	{ "AL50",   50, 3000 },
	{ "AL63",   63, 3000 },
	{ "AL72",   72, 3000 },
	{ "AL80",   80, 3000 },
	{ "AL100", 100, 3300 },

	/* Somewhat common LP steel cylinders */
	{ "LP85",   85, 2640 },
	{ "LP95",   95, 2640 },
	{ "LP108", 108, 2640 },
	{ "LP121", 121, 2640 },

	/* Somewhat common HP steel cylinders */
	{ "HP65",   65, 3442 },
	{ "HP80",   80, 3442 },
	{ "HP100", 100, 3442 },
	{ "HP119", 119, 3442 },
	{ "HP130", 130, 3442 },

	/* We'll fill in more from the dive log dynamically */
	{ NULL, }
};

static void fill_tank_list(GtkListStore *store)
{
	GtkTreeIter iter;
	struct tank_info *info = tank_info;

	while (info->name) {
		int size = info->size;
		int psi = info->psi;
		int mbar = 0, ml = size;

		/* Is it in cuft and psi? */
		if (psi) {
			double bar = 0.0689475729 * psi;
			double airvolume = 28316.8466 * size;
			double atm = bar / 1.01325;

			ml = airvolume / atm + 0.5;
			mbar = bar*1000 + 0.5;
		}

		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter,
			0, info->name,
			1, ml,
			2, mbar,
			-1);
		info++;
	}
}

static void nitrox_cb(GtkToggleButton *button, gpointer data)
{
	struct cylinder_widget *cylinder = data;
	int state;

	state = gtk_toggle_button_get_active(button);
	gtk_widget_set_sensitive(cylinder->o2, state);
}

static void cylinder_widget(int nr, GtkListStore *model)
{
	struct cylinder_widget *cylinder;
	GtkWidget *frame, *hbox, *hbox2;
	GtkWidget *widget;
	char buffer[80];

	cylinder = gtk_cylinder + nr;
	cylinder->index = nr;

	hbox = gtk_hbox_new(FALSE, 3);
	cylinder->hbox = hbox;

	snprintf(buffer, sizeof(buffer), "Cylinder %d", nr+1);
	frame = gtk_frame_new(buffer);
	gtk_box_pack_start(GTK_BOX(hbox), frame, FALSE, TRUE, 0);

	hbox2 = gtk_hbox_new(FALSE, 3);
	gtk_container_add(GTK_CONTAINER(frame), hbox2);

	widget = gtk_combo_box_entry_new_with_model(GTK_TREE_MODEL(model), 0);
	gtk_box_pack_start(GTK_BOX(hbox2), widget, FALSE, TRUE, 0);

	cylinder->description = GTK_COMBO_BOX(widget);
	g_signal_connect(widget, "changed", G_CALLBACK(cylinder_cb), cylinder);

	widget = create_spinbutton(hbox, "Size", 0, 200, 0.1);
	cylinder->size = GTK_SPIN_BUTTON(widget);

	widget = create_spinbutton(hbox, "Pressure", 0, 5000, 1);
	cylinder->pressure = GTK_SPIN_BUTTON(widget);

	widget = create_spinbutton(hbox, "Nitrox", 21, 100, 0.1);
	cylinder->o2 = widget;
	cylinder->gasmix_button = gtk_check_button_new();
	gtk_box_pack_start(GTK_BOX(gtk_widget_get_parent(cylinder->o2)),
		cylinder->gasmix_button, FALSE, FALSE, 3);
	g_signal_connect(cylinder->gasmix_button, "toggled", G_CALLBACK(nitrox_cb), cylinder);

	gtk_spin_button_set_range(GTK_SPIN_BUTTON(cylinder->o2), 21.0, 100.0);
}

static GtkListStore *create_tank_size_model(void)
{
	GtkListStore *model;

	model = gtk_list_store_new(3,
		G_TYPE_STRING,		/* Tank name */
		G_TYPE_INT,		/* Tank size in mliter */
		G_TYPE_INT,		/* Tank working pressure in mbar */
		-1);

	fill_tank_list(model);
	return model;
}

GtkWidget *equipment_widget(void)
{
	int i;
	GtkWidget *vbox, *hbox;
	GtkWidget *apply, *cancel;
	GtkListStore *model;

	vbox = gtk_vbox_new(FALSE, 3);

	model = create_tank_size_model();

	/* Create all MAX_CYLINDER gtk widgets */
	for (i = 0; i < MAX_CYLINDERS; i++)
		cylinder_widget(i, model);

	/* But only connect two of them right now to the frame vbox */
	for (i = 0; i < 2; i++) {
		struct cylinder_widget *cylinder = gtk_cylinder+i;
		gtk_box_pack_start(GTK_BOX(vbox), cylinder->hbox, FALSE, TRUE, 0);
	}

	hbox = gtk_hbox_new(TRUE, 3);
	gtk_box_pack_end(GTK_BOX(vbox), hbox, TRUE, FALSE, 0);

	apply = gtk_button_new_from_stock(GTK_STOCK_APPLY);
	cancel = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	gtk_box_pack_start(GTK_BOX(hbox), apply, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), cancel, FALSE, FALSE, 0);

	g_signal_connect(apply, "clicked", G_CALLBACK(apply_cb), NULL);
	g_signal_connect(cancel, "clicked", G_CALLBACK(cancel_cb), NULL);

	return vbox;
}
