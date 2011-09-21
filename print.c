#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <gtk/gtk.h>

#include "dive.h"
#include "display.h"
#include "display-gtk.h"

/*
 * You know what? Maybe somebody can do a real Pango layout thing.
 * This is hacky.
 */
static void show_dive_text(struct dive *dive, cairo_t *cr, double w, double h, PangoFontDescription *font)
{
	int len, width, height, maxwidth, maxheight;
	PangoLayout *layout;
	struct tm *tm;
	char buffer[1024], divenr[20];

	maxwidth = w * PANGO_SCALE;
	maxheight = h * PANGO_SCALE * 0.9;

	layout = pango_cairo_create_layout(cr);
	pango_layout_set_font_description(layout, font);
	pango_layout_set_width(layout, maxwidth);
	pango_layout_set_height(layout, maxheight);
	pango_layout_set_wrap(layout, PANGO_WRAP_WORD_CHAR);

	*divenr = 0;
	if (dive->number)
		snprintf(divenr, sizeof(divenr), "Dive #%d - ", dive->number);


	tm = gmtime(&dive->when);
	len = snprintf(buffer, sizeof(buffer),
		"<span size=\"large\">"
		"%s%s, %s %d, %d   %d:%02d"
		"</span>",
		divenr,
		weekday(tm->tm_wday),
		monthname(tm->tm_mon),
		tm->tm_mday, tm->tm_year + 1900,
		tm->tm_hour, tm->tm_min);

	pango_layout_set_justify(layout, 1);
	pango_layout_set_ellipsize(layout, PANGO_ELLIPSIZE_END);
	pango_layout_set_alignment(layout, PANGO_ALIGN_LEFT);

	pango_layout_set_markup(layout, buffer, len);
	pango_layout_get_size(layout, &width, &height);

	cairo_move_to(cr, 0, 0);
	pango_cairo_show_layout(cr, layout);

	/*
	 * This is still problematic: a long dive location will clash
	 * with the depth/duration information. Need to mask that or
	 * create a box or something.
	 */
	snprintf(buffer, sizeof(buffer),
		"<span size=\"small\">"
		"Max depth: %d ft\n"
		"Duration: %d:%02d"
		"</span>",
		to_feet(dive->maxdepth),
		dive->duration.seconds / 60,
		dive->duration.seconds % 60);

	pango_layout_set_alignment(layout, PANGO_ALIGN_RIGHT);
	pango_layout_set_markup(layout, buffer, -1);

	cairo_move_to(cr, 0, 0);
	pango_cairo_show_layout(cr, layout);

	len = snprintf(buffer, sizeof(buffer), "%s\n\n%s",
		dive->location ? : "",
		dive->notes ? : "");

	maxheight -= height;
	pango_layout_set_height(layout, maxheight);
	pango_layout_set_attributes(layout, NULL);
	pango_layout_set_alignment(layout, PANGO_ALIGN_LEFT);
	pango_layout_set_text(layout, buffer, len);

	cairo_move_to(cr, 0, height / (double) PANGO_SCALE);
	pango_cairo_show_layout(cr, layout);

	g_object_unref(layout);
}

static void show_dive_profile(struct dive *dive, cairo_t *cr, double w, double h)
{
	struct graphics_context gc = {
		.printer = 1,
		.cr = cr
	};
	cairo_save(cr);
	plot(&gc, w, h, dive);
	cairo_restore(cr);
}

static void print(int divenr, cairo_t *cr, double x, double y, double w, double h, PangoFontDescription *font)
{
	struct dive *dive;

	dive = get_dive(divenr);
	if (!dive)
		return;
	cairo_save(cr);
	cairo_translate(cr, x, y);

	/* Plus 5% on all sides */
	cairo_translate(cr, w/20, h/20);
	w *= 0.9; h *= 0.9;

	/* We actually want to scale the text and the lines now */
	cairo_scale(cr, 0.5, 0.5);

	/* Dive plot in the upper two thirds - note the scaling */
	show_dive_profile(dive, cr, w*2, h*1.33);

	/* Dive information in the lower third */
	cairo_translate(cr, 0, h*1.33);

	show_dive_text(dive, cr, w*2, h*0.67, font);

	cairo_restore(cr);
}

static void draw_page(GtkPrintOperation *operation,
			GtkPrintContext *context,
			gint page_nr,
			gpointer user_data)
{
	int nr;
	cairo_t *cr;
	double w, h;
	PangoFontDescription *font;

	cr = gtk_print_context_get_cairo_context(context);
	font = pango_font_description_from_string("Sans");

	w = gtk_print_context_get_width(context)/2;
	h = gtk_print_context_get_height(context)/3;

	nr = page_nr*6;
	print(nr+0, cr, 0,   0, w, h, font);
	print(nr+1, cr, w,   0, w, h, font);
	print(nr+2, cr, 0,   h, w, h, font);
	print(nr+3, cr, w,   h, w, h, font);
	print(nr+4, cr, 0, 2*h, w, h, font);
	print(nr+5, cr, w, 2*h, w, h, font);

	pango_font_description_free(font);
}

static void begin_print(GtkPrintOperation *operation, gpointer user_data)
{
}

static GtkPrintSettings *settings = NULL;

void do_print(void)
{
	int pages;
	GtkPrintOperation *print;
	GtkPrintOperationResult res;

	repaint_dive();
	print = gtk_print_operation_new();
	if (settings != NULL)
		gtk_print_operation_set_print_settings(print, settings);
	pages = (dive_table.nr + 5) / 6;
	gtk_print_operation_set_n_pages(print, pages);
	g_signal_connect(print, "begin_print", G_CALLBACK(begin_print), NULL);
	g_signal_connect(print, "draw_page", G_CALLBACK(draw_page), NULL);
	res = gtk_print_operation_run(print, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG,
					 GTK_WINDOW(main_window), NULL);
	if (res == GTK_PRINT_OPERATION_RESULT_APPLY) {
		if (settings != NULL)
			g_object_unref(settings);
		settings = g_object_ref(gtk_print_operation_get_print_settings(print));
	}
	g_object_unref(print);
}
