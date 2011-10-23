/* profile.c */
/* creates all the necessary data for drawing the dive profile 
 * uses cairo to draw it
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#include "dive.h"
#include "display.h"
#include "divelist.h"

int selected_dive = 0;

typedef enum { STABLE, SLOW, MODERATE, FAST, CRAZY } velocity_t;
/* Plot info with smoothing, velocity indication
 * and one-, two- and three-minute minimums and maximums */
struct plot_info {
	int nr;
	int maxtime;
	int meandepth, maxdepth;
	int minpressure, maxpressure;
	int endpressure; /* start pressure better be max pressure */
	int mintemp, maxtemp;
	struct plot_data {
		unsigned int same_cylinder:1;
		unsigned int cylinderindex;
		int sec;
		/* pressure[0] is sensor pressure
		 * pressure[1] is interpolated pressure */
		int pressure[2];
		int temperature;
		/* Depth info */
		int depth;
		int smoothed;
		velocity_t velocity;
		struct plot_data *min[3];
		struct plot_data *max[3];
		int avg[3];
	} entry[];
};
#define SENSOR_PR 0
#define INTERPOLATED_PR 1
#define SENSOR_PRESSURE(_entry) (_entry)->pressure[SENSOR_PR]
#define INTERPOLATED_PRESSURE(_entry) (_entry)->pressure[INTERPOLATED_PR]

/* convert velocity to colors */
typedef struct { double r, g, b; } rgb_t;
static const rgb_t rgb[] = {
	[STABLE]   = {0.0, 0.4, 0.0},
	[SLOW]     = {0.4, 0.8, 0.0},
	[MODERATE] = {0.8, 0.8, 0.0},
	[FAST]     = {0.8, 0.5, 0.0},
	[CRAZY]    = {1.0, 0.0, 0.0},
};

#define plot_info_size(nr) (sizeof(struct plot_info) + (nr)*sizeof(struct plot_data))

/* Scale to 0,0 -> maxx,maxy */
#define SCALEX(gc,x)  (((x)-gc->leftx)/(gc->rightx-gc->leftx)*gc->maxx)
#define SCALEY(gc,y)  (((y)-gc->topy)/(gc->bottomy-gc->topy)*gc->maxy)
#define SCALE(gc,x,y) SCALEX(gc,x),SCALEY(gc,y)

static void move_to(struct graphics_context *gc, double x, double y)
{
	cairo_move_to(gc->cr, SCALE(gc, x, y));
}

static void line_to(struct graphics_context *gc, double x, double y)
{
	cairo_line_to(gc->cr, SCALE(gc, x, y));
}

static void set_source_rgba(struct graphics_context *gc, double r, double g, double b, double a)
{
	/*
	 * For printers, we still honor 'a', but ignore colors
	 * for now. Black is white and white is black
	 */
	if (gc->printer) {
		double sum = r+g+b;
		if (sum > 0.8)
			r = g = b = 0;
		else
			r = g = b = 1;
	}
	cairo_set_source_rgba(gc->cr, r, g, b, a);
}

void set_source_rgb(struct graphics_context *gc, double r, double g, double b)
{
	set_source_rgba(gc, r, g, b, 1);
}

#define ROUND_UP(x,y) ((((x)+(y)-1)/(y))*(y))

/*
 * When showing dive profiles, we scale things to the
 * current dive. However, we don't scale past less than
 * 30 minutes or 90 ft, just so that small dives show
 * up as such.
 * we also need to add 180 seconds at the end so the min/max
 * plots correctly
 */
static int get_maxtime(struct plot_info *pi)
{
	int seconds = pi->maxtime;
	/* min 30 minutes, rounded up to 5 minutes, with at least 2.5 minutes to spare */
	return MAX(30*60, ROUND_UP(seconds+150, 60*5));
}

static int get_maxdepth(struct plot_info *pi)
{
	unsigned mm = pi->maxdepth;
	/* Minimum 30m, rounded up to 10m, with at least 3m to spare */
	return MAX(30000, ROUND_UP(mm+3000, 10000));
}

typedef struct {
	int size;
	double r,g,b;
	double hpos, vpos;
} text_render_options_t;

#define RIGHT (-1.0)
#define CENTER (-0.5)
#define LEFT (0.0)

#define TOP (1)
#define MIDDLE (0)
#define BOTTOM (-1)

static void plot_text(struct graphics_context *gc, const text_render_options_t *tro,
		      double x, double y, const char *fmt, ...)
{
	cairo_t *cr = gc->cr;
	cairo_font_extents_t fe;
	cairo_text_extents_t extents;
	double dx, dy;
	char buffer[80];
	va_list args;

	va_start(args, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);

	cairo_set_font_size(cr, tro->size);
	cairo_font_extents(cr, &fe);
	cairo_text_extents(cr, buffer, &extents);
	dx = tro->hpos * extents.width + extents.x_bearing;
	dy = tro->vpos * extents.height + fe.descent;

	move_to(gc, x, y);
	cairo_rel_move_to(cr, dx, dy);

	cairo_text_path(cr, buffer);
	set_source_rgb(gc, 0, 0, 0);
	cairo_stroke(cr);

	move_to(gc, x, y);
	cairo_rel_move_to(cr, dx, dy);

	set_source_rgb(gc, tro->r, tro->g, tro->b);
	cairo_show_text(cr, buffer);
}

static void plot_one_event(struct graphics_context *gc, struct plot_info *pi, struct event *event, const text_render_options_t *tro)
{
	int i, depth = 0;
	int x,y;

	for (i = 0; i < pi->nr; i++) {
		struct plot_data *data = pi->entry + i;
		if (event->time.seconds < data->sec)
			break;
		depth = data->depth;
	}
	/* draw a little tirangular marker and attach tooltip */
	x = SCALEX(gc, event->time.seconds);
	y = SCALEY(gc, depth);
	set_source_rgba(gc, 1.0, 1.0, 0.1, 0.8);
	cairo_move_to(gc->cr, x-15, y+6);
	cairo_line_to(gc->cr, x-3  , y+6);
	cairo_line_to(gc->cr, x-9, y-6);
	cairo_line_to(gc->cr, x-15, y+6);
	cairo_stroke_preserve(gc->cr);
	cairo_fill(gc->cr);
	set_source_rgba(gc, 0.0, 0.0, 0.0, 0.8);
	cairo_move_to(gc->cr, x-9, y-3);
	cairo_line_to(gc->cr, x-9, y+1);
	cairo_move_to(gc->cr, x-9, y+4);
	cairo_line_to(gc->cr, x-9, y+4);
	cairo_stroke(gc->cr);
	attach_tooltip(x-15, y-6, 12, 12, event->name);
}

static void plot_events(struct graphics_context *gc, struct plot_info *pi, struct dive *dive)
{
	static const text_render_options_t tro = {14, 1.0, 0.2, 0.2, CENTER, TOP};
	struct event *event = dive->events;

	if (gc->printer)
		return;

	while (event) {
		plot_one_event(gc, pi, event, &tro);
		event = event->next;
	}
}

static void render_depth_sample(struct graphics_context *gc, struct plot_data *entry, const text_render_options_t *tro)
{
	int sec = entry->sec, decimals;
	double d;

	d = get_depth_units(entry->depth, &decimals, NULL);

	plot_text(gc, tro, sec, entry->depth, "%.*f", decimals, d);
}

static void plot_text_samples(struct graphics_context *gc, struct plot_info *pi)
{
	static const text_render_options_t deep = {14, 1.0, 0.2, 0.2, CENTER, TOP};
	static const text_render_options_t shallow = {14, 1.0, 0.2, 0.2, CENTER, BOTTOM};
	int i;

	for (i = 0; i < pi->nr; i++) {
		struct plot_data *entry = pi->entry + i;

		if (entry->depth < 2000)
			continue;

		if (entry == entry->max[2])
			render_depth_sample(gc, entry, &deep);

		if (entry == entry->min[2])
			render_depth_sample(gc, entry, &shallow);
	}
}

static void plot_depth_text(struct graphics_context *gc, struct plot_info *pi)
{
	int maxtime, maxdepth;

	/* Get plot scaling limits */
	maxtime = get_maxtime(pi);
	maxdepth = get_maxdepth(pi);

	gc->leftx = 0; gc->rightx = maxtime;
	gc->topy = 0; gc->bottomy = maxdepth;

	plot_text_samples(gc, pi);
}

static void plot_smoothed_profile(struct graphics_context *gc, struct plot_info *pi)
{
	int i;
	struct plot_data *entry = pi->entry;

	set_source_rgba(gc, 1, 0.2, 0.2, 0.20);
	move_to(gc, entry->sec, entry->smoothed);
	for (i = 1; i < pi->nr; i++) {
		entry++;
		line_to(gc, entry->sec, entry->smoothed);
	}
	cairo_stroke(gc->cr);
}

static void plot_minmax_profile_minute(struct graphics_context *gc, struct plot_info *pi,
				int index, double a)
{
	int i;
	struct plot_data *entry = pi->entry;

	set_source_rgba(gc, 1, 0.2, 1, a);
	move_to(gc, entry->sec, entry->min[index]->depth);
	for (i = 1; i < pi->nr; i++) {
		entry++;
		line_to(gc, entry->sec, entry->min[index]->depth);
	}
	for (i = 1; i < pi->nr; i++) {
		line_to(gc, entry->sec, entry->max[index]->depth);
		entry--;
	}
	cairo_close_path(gc->cr);
	cairo_fill(gc->cr);
}

static void plot_minmax_profile(struct graphics_context *gc, struct plot_info *pi)
{
	if (gc->printer)
		return;
	plot_minmax_profile_minute(gc, pi, 2, 0.1);
	plot_minmax_profile_minute(gc, pi, 1, 0.1);
	plot_minmax_profile_minute(gc, pi, 0, 0.1);
}

static void plot_depth_profile(struct graphics_context *gc, struct plot_info *pi)
{
	int i;
	cairo_t *cr = gc->cr;
	int sec, depth;
	struct plot_data *entry;
	int maxtime, maxdepth, marker;

	/* Get plot scaling limits */
	maxtime = get_maxtime(pi);
	maxdepth = get_maxdepth(pi);

	/* Time markers: every 5 min */
	gc->leftx = 0; gc->rightx = maxtime;
	gc->topy = 0; gc->bottomy = 1.0;
	for (i = 5*60; i < maxtime; i += 5*60) {
		move_to(gc, i, 0);
		line_to(gc, i, 1);
	}

	/* Depth markers: every 30 ft or 10 m*/
	gc->leftx = 0; gc->rightx = 1.0;
	gc->topy = 0; gc->bottomy = maxdepth;
	switch (output_units.length) {
	case METERS: marker = 10000; break;
	case FEET: marker = 9144; break;	/* 30 ft */
	}

	set_source_rgba(gc, 1, 1, 1, 0.5);
	for (i = marker; i < maxdepth; i += marker) {
		move_to(gc, 0, i);
		line_to(gc, 1, i);
	}
	cairo_stroke(cr);

	/* Show mean depth */
	if (! gc->printer) {
		set_source_rgba(gc, 1, 0.2, 0.2, 0.40);
		move_to(gc, 0, pi->meandepth);
		line_to(gc, 1, pi->meandepth);
		cairo_stroke(cr);
	}

	gc->leftx = 0; gc->rightx = maxtime;

	/*
	 * These are good for debugging text placement etc,
	 * but not for actual display..
	 */
	if (0) {
		plot_smoothed_profile(gc, pi);
		plot_minmax_profile(gc, pi);
	}

	set_source_rgba(gc, 1, 0.2, 0.2, 0.80);

	/* Do the depth profile for the neat fill */
	gc->topy = 0; gc->bottomy = maxdepth;
	set_source_rgba(gc, 1, 0.2, 0.2, 0.20);

	entry = pi->entry;
	move_to(gc, 0, 0);
	for (i = 0; i < pi->nr; i++, entry++)
		line_to(gc, entry->sec, entry->depth);
	cairo_close_path(gc->cr);
	if (gc->printer) {
		set_source_rgba(gc, 1, 1, 1, 0.2);
		cairo_fill_preserve(cr);
		set_source_rgb(gc, 1, 1, 1);
		cairo_stroke(cr);
		return;
	}
	cairo_fill(gc->cr);

	/* Now do it again for the velocity colors */
	entry = pi->entry;
	for (i = 1; i < pi->nr; i++) {
		entry++;
		sec = entry->sec;
		/* we want to draw the segments in different colors
		 * representing the vertical velocity, so we need to
		 * chop this into short segments */
		rgb_t color = rgb[entry->velocity];
		depth = entry->depth;
		set_source_rgb(gc, color.r, color.g, color.b);
		move_to(gc, entry[-1].sec, entry[-1].depth);
		line_to(gc, sec, depth);
		cairo_stroke(cr);
	}
}

static int setup_temperature_limits(struct graphics_context *gc, struct plot_info *pi)
{
	int maxtime, mintemp, maxtemp, delta;

	/* Get plot scaling limits */
	maxtime = get_maxtime(pi);
	mintemp = pi->mintemp;
	maxtemp = pi->maxtemp;

	gc->leftx = 0; gc->rightx = maxtime;
	/* Show temperatures in roughly the lower third, but make sure the scale
	   is at least somewhat reasonable */
	delta = maxtemp - mintemp;
	if (delta > 3000) { /* more than 3K in fluctuation */
		gc->topy = maxtemp + delta*2;
		gc->bottomy = mintemp - delta/2;
	} else {
		gc->topy = maxtemp + 1500 + delta*2;
		gc->bottomy = mintemp - delta/2;
	}

	return maxtemp > mintemp;
}

static void plot_single_temp_text(struct graphics_context *gc, int sec, int mkelvin)
{
	int deg;
	const char *unit;
	static const text_render_options_t tro = {12, 0.2, 0.2, 1.0, LEFT, TOP};
	temperature_t temperature = { mkelvin };

	if (output_units.temperature == FAHRENHEIT) {
		deg = to_F(temperature);
		unit = UTF8_DEGREE "F";
	} else {
		deg = to_C(temperature);
		unit = UTF8_DEGREE "C";
	}
	plot_text(gc, &tro, sec, temperature.mkelvin, "%d%s", deg, unit);
}

static void plot_temperature_text(struct graphics_context *gc, struct plot_info *pi)
{
	int i;
	int last = 0, sec = 0;
	int last_temperature = 0, last_printed_temp = 0;

	if (!setup_temperature_limits(gc, pi))
		return;

	for (i = 0; i < pi->nr; i++) {
		struct plot_data *entry = pi->entry+i;
		int mkelvin = entry->temperature;

		if (!mkelvin)
			continue;
		last_temperature = mkelvin;
		sec = entry->sec;
		if (sec < last + 300)
			continue;
		last = sec;
		plot_single_temp_text(gc,sec,mkelvin);
		last_printed_temp = mkelvin;
	}
	/* it would be nice to print the end temperature, if it's different */
	if (abs(last_temperature - last_printed_temp) > 500)
		plot_single_temp_text(gc, sec, last_temperature);
}

static void plot_temperature_profile(struct graphics_context *gc, struct plot_info *pi)
{
	int i;
	cairo_t *cr = gc->cr;
	int last = 0;

	if (!setup_temperature_limits(gc, pi))
		return;

	set_source_rgba(gc, 0.2, 0.2, 1.0, 0.8);
	for (i = 0; i < pi->nr; i++) {
		struct plot_data *entry = pi->entry + i;
		int mkelvin = entry->temperature;
		int sec = entry->sec;
		if (!mkelvin) {
			if (!last)
				continue;
			mkelvin = last;
		}
		if (last)
			line_to(gc, sec, mkelvin);
		else
			move_to(gc, sec, mkelvin);
		last = mkelvin;
	}
	cairo_stroke(cr);
}

/* gets both the actual start and end pressure as well as the scaling factors */
static int get_cylinder_pressure_range(struct graphics_context *gc, struct plot_info *pi)
{
	gc->leftx = 0;
	gc->rightx = get_maxtime(pi);

	gc->bottomy = 0; gc->topy = pi->maxpressure * 1.5;
	return pi->maxpressure != 0;
}

static void plot_pressure_helper(struct graphics_context *gc, struct plot_info *pi, int type)
{
	int i;
	int lift_pen = FALSE;

	for (i = 0; i < pi->nr; i++) {
		int mbar;
		struct plot_data *entry = pi->entry + i;

		mbar = entry->pressure[type];
		if (!entry->same_cylinder)
			lift_pen = TRUE;
		if (!mbar) {
			lift_pen = TRUE;
			continue;
		}
		if (lift_pen) {
			if (i > 0 && entry->same_cylinder) {
				/* if we have a previous event from the same tank,
				 * draw at least a short line .
				 * This uses the implementation detail that the
				 * type is either 0 or 1 */
				int prev_pr;
				prev_pr = (entry-1)->pressure[type] ? : (entry-1)->pressure[1 - type];
				move_to(gc, (entry-1)->sec, prev_pr);
				line_to(gc, entry->sec, mbar);
			} else
				move_to(gc, entry->sec, mbar);
			lift_pen = FALSE;
		}
		else
			line_to(gc, entry->sec, mbar);
	}
	cairo_stroke(gc->cr);

}

static void plot_cylinder_pressure(struct graphics_context *gc, struct plot_info *pi)
{
	if (!get_cylinder_pressure_range(gc, pi))
		return;

	/* first plot the pressure readings we have from the dive computer */
	set_source_rgba(gc, 0.2, 1.0, 0.2, 0.80);
	plot_pressure_helper(gc, pi, SENSOR_PR);

	/* then, in a different color, the interpolated values */
	set_source_rgba(gc, 1.0, 1.0, 0.2, 0.80);
	plot_pressure_helper(gc, pi, INTERPOLATED_PR);
}

static int mbar_to_PSI(int mbar)
{
	pressure_t p = {mbar};
	return to_PSI(p);
}

static void plot_pressure_value(struct graphics_context *gc, int mbar, int sec,
				int xalign, int yalign)
{
	int pressure;
	const char *unit;

	switch (output_units.pressure) {
	case PASCAL:
		pressure = mbar * 100;
		unit = "pascal";
		break;
	case BAR:
		pressure = (mbar + 500) / 1000;
		unit = "bar";
		break;
	case PSI:
		pressure = mbar_to_PSI(mbar);
		unit = "psi";
		break;
	}
	text_render_options_t tro = {10, 0.2, 1.0, 0.2, xalign, yalign};
	plot_text(gc, &tro, sec, mbar, "%d %s", pressure, unit);
}

static void plot_cylinder_pressure_text(struct graphics_context *gc, struct plot_info *pi)
{
	int i;
	int mbar, cyl;
	int seen_cyl[MAX_CYLINDERS] = { FALSE, };
	int last_pressure[MAX_CYLINDERS] = { 0, };
	int last_time[MAX_CYLINDERS] = { 0, };
	struct plot_data *entry;

	if (!get_cylinder_pressure_range(gc, pi))
		return;

	/* only loop over the actual events from the dive computer */
	for (i = 2; i < pi->nr - 2; i++) {
		entry = pi->entry + i;

		if (!entry->same_cylinder) {
			cyl = entry->cylinderindex;
			if (!seen_cyl[cyl]) {
				mbar = SENSOR_PRESSURE(entry) ? : INTERPOLATED_PRESSURE(entry);
				plot_pressure_value(gc, mbar, entry->sec, LEFT, BOTTOM);
				seen_cyl[cyl] = TRUE;
			}
			if (i > 2) {
				/* remember the last pressure and time of
				 * the previous cylinder */
				cyl = (entry - 1)->cylinderindex;
				last_pressure[cyl] =
					SENSOR_PRESSURE(entry - 1) ? : INTERPOLATED_PRESSURE(entry - 1);
				last_time[cyl] = (entry - 1)->sec;
			}
		}
	}
	cyl = entry->cylinderindex;
	last_pressure[cyl] = SENSOR_PRESSURE(entry) ? : INTERPOLATED_PRESSURE(entry);
	last_time[cyl] = entry->sec;

	for (cyl = 0; cyl < MAX_CYLINDERS; cyl++) {
		if (last_time[cyl]) {
			plot_pressure_value(gc, last_pressure[cyl], last_time[cyl], CENTER, TOP);
		}
	}
}

static void analyze_plot_info_minmax_minute(struct plot_data *entry, struct plot_data *first, struct plot_data *last, int index)
{
	struct plot_data *p = entry;
	int time = entry->sec;
	int seconds = 90*(index+1);
	struct plot_data *min, *max;
	int avg, nr;

	/* Go back 'seconds' in time */
	while (p > first) {
		if (p[-1].sec < time - seconds)
			break;
		p--;
	}

	/* Then go forward until we hit an entry past the time */
	min = max = p;
	avg = p->depth;
	nr = 1;
	while (++p < last) {
		int depth = p->depth;
		if (p->sec > time + seconds)
			break;
		avg += depth;
		nr ++;
		if (depth < min->depth)
			min = p;
		if (depth > max->depth)
			max = p;
	}
	entry->min[index] = min;
	entry->max[index] = max;
	entry->avg[index] = (avg + nr/2) / nr;
}

static void analyze_plot_info_minmax(struct plot_data *entry, struct plot_data *first, struct plot_data *last)
{
	analyze_plot_info_minmax_minute(entry, first, last, 0);
	analyze_plot_info_minmax_minute(entry, first, last, 1);
	analyze_plot_info_minmax_minute(entry, first, last, 2);
}

static velocity_t velocity(int speed)
{
	velocity_t v;

	if (speed < -304) /* ascent faster than -60ft/min */
		v = CRAZY;
	else if (speed < -152) /* above -30ft/min */
		v = FAST;
	else if (speed < -76) /* -15ft/min */
		v = MODERATE;
	else if (speed < -25) /* -5ft/min */
		v = SLOW;
	else if (speed < 25) /* very hard to find data, but it appears that the recommendations
				for descent are usually about 2x ascent rate; still, we want 
				stable to mean stable */
		v = STABLE;
	else if (speed < 152) /* between 5 and 30ft/min is considered slow */
		v = SLOW;
	else if (speed < 304) /* up to 60ft/min is moderate */
		v = MODERATE;
	else if (speed < 507) /* up to 100ft/min is fast */
		v = FAST;
	else /* more than that is just crazy - you'll blow your ears out */
		v = CRAZY;

	return v;
}
static struct plot_info *analyze_plot_info(struct plot_info *pi)
{
	int i;
	int nr = pi->nr;

	/* Do pressure min/max based on the non-surface data */
	for (i = 0; i < nr; i++) {
		struct plot_data *entry = pi->entry+i;
		int pressure = SENSOR_PRESSURE(entry) ? : INTERPOLATED_PRESSURE(entry);
		int temperature = entry->temperature;

		if (pressure) {
			if (!pi->minpressure || pressure < pi->minpressure)
				pi->minpressure = pressure;
			if (pressure > pi->maxpressure)
				pi->maxpressure = pressure;
		}

		if (temperature) {
			if (!pi->mintemp || temperature < pi->mintemp)
				pi->mintemp = temperature;
			if (temperature > pi->maxtemp)
				pi->maxtemp = temperature;
		}
	}

	/* Smoothing function: 5-point triangular smooth */
	for (i = 2; i < nr; i++) {
		struct plot_data *entry = pi->entry+i;
		int depth;

		if (i < nr-2) {
			depth = entry[-2].depth + 2*entry[-1].depth + 3*entry[0].depth + 2*entry[1].depth + entry[2].depth;
			entry->smoothed = (depth+4) / 9;
		}
		/* vertical velocity in mm/sec */
		/* Linus wants to smooth this - let's at least look at the samples that aren't FAST or CRAZY */
		if (entry[0].sec - entry[-1].sec) {
			entry->velocity = velocity((entry[0].depth - entry[-1].depth) / (entry[0].sec - entry[-1].sec));
                        /* if our samples are short and we aren't too FAST*/
			if (entry[0].sec - entry[-1].sec < 15 && entry->velocity < FAST) {
				int past = -2;
				while (i+past > 0 && entry[0].sec - entry[past].sec < 15)
					past--;
				entry->velocity = velocity((entry[0].depth - entry[past].depth) / 
							(entry[0].sec - entry[past].sec));
			}
		} else
			entry->velocity = STABLE;
	}

	/* One-, two- and three-minute minmax data */
	for (i = 0; i < nr; i++) {
		struct plot_data *entry = pi->entry +i;
		analyze_plot_info_minmax(entry, pi->entry, pi->entry+nr);
	}
	
	return pi;
}

/*
 * simple structure to track the beginning and end tank pressure as
 * well as the integral of depth over time spent while we have no
 * pressure reading from the tank */
typedef struct pr_track_struct pr_track_t;
struct pr_track_struct {
	int start;
	int end;
	int t_start;
	int t_end;
	double pressure_time;
	pr_track_t *next;
};

static pr_track_t *pr_track_alloc(int start, int t_start) {
	pr_track_t *pt = malloc(sizeof(pr_track_t));
	pt->start = start;
	pt->t_start = t_start;
	pt->end = 0;
	pt->t_end = 0;
	pt->pressure_time = 0.0;
	pt->next = NULL;
	return pt;
}

/* poor man's linked list */
static pr_track_t *list_last(pr_track_t *list)
{
	pr_track_t *tail = list;
	if (!tail)
		return NULL;
	while (tail->next) {
		tail = tail->next;
	}
	return tail;
}

static pr_track_t *list_add(pr_track_t *list, pr_track_t *element)
{
	pr_track_t *tail = list_last(list);
	if (!tail)
		return element;
	tail->next = element;
	return list;
}

static void list_free(pr_track_t *list)
{
	if (!list)
		return;
	list_free(list->next);
	free(list);
}

static void fill_missing_tank_pressures(struct dive *dive, struct plot_info *pi,
					pr_track_t **track_pr)
{
	pr_track_t *list = NULL;
	pr_track_t *nlist = NULL;
	double pt, magic;
	int cyl, i;
	struct plot_data *entry;
	int cur_pr[MAX_CYLINDERS];

	for (cyl = 0; cyl < MAX_CYLINDERS; cyl++) {
		cur_pr[cyl] = track_pr[cyl]->start;
	}
	for (i = 0; i < dive->samples; i++) {
		entry = pi->entry + i + 2;
		if (SENSOR_PRESSURE(entry)) {
			cur_pr[entry->cylinderindex] = SENSOR_PRESSURE(entry);
		} else {
			if(!list || list->t_end < entry->sec) {
				nlist = track_pr[entry->cylinderindex];
				list = NULL;
				while (nlist && nlist->t_start <= entry->sec) {
					list = nlist;
					nlist = list->next;
				}
				/* there may be multiple segments - so
				 * let's assemble the length */
				nlist = list;
				pt = list->pressure_time;
				while (!nlist->end) {
					nlist = nlist->next;
					if (!nlist) {
						/* oops - we have no end pressure,
						 * so this means this is a tank without
						 * gas consumption information */
						break;
					}
					pt += nlist->pressure_time;
				}
				if (!nlist) {
					/* just continue without calculating
					 * interpolated values */
					list = NULL;
					continue;
				}
				magic = (nlist->end - cur_pr[entry->cylinderindex]) / pt;				}
			if (pt != 0.0) {
				double cur_pt = (entry->sec - (entry-1)->sec) *
					(1 + entry->depth / 10000.0);
				INTERPOLATED_PRESSURE(entry) =
					cur_pr[entry->cylinderindex] + cur_pt * magic;
				cur_pr[entry->cylinderindex] = INTERPOLATED_PRESSURE(entry);
			}
		}
	}
}

/*
 * Create a plot-info with smoothing and ranged min/max
 *
 * This also makes sure that we have extra empty events on both
 * sides, so that you can do end-points without having to worry
 * about it.
 */
static struct plot_info *create_plot_info(struct dive *dive)
{
	int cylinderindex = -1;
	int lastdepth, lastindex;
	int i, nr = dive->samples + 4, sec, cyl;
	size_t alloc_size = plot_info_size(nr);
	struct plot_info *pi;
	pr_track_t *track_pr[MAX_CYLINDERS] = {NULL, };
	pr_track_t *pr_track, *current;
	gboolean missing_pr = FALSE;
	struct plot_data *entry;

	pi = malloc(alloc_size);
	if (!pi)
		return pi;
	memset(pi, 0, alloc_size);
	pi->nr = nr;
	sec = 0;
	lastindex = 0;
	lastdepth = -1;
	for (cyl = 0; cyl < MAX_CYLINDERS; cyl++) /* initialize the start pressures */
		track_pr[cyl] = pr_track_alloc(dive->cylinder[cyl].start.mbar, -1);
	current = track_pr[dive->sample[0].cylinderindex];
	for (i = 0; i < dive->samples; i++) {
		int depth;
		struct sample *sample = dive->sample+i;

		entry = pi->entry + i + 2;
		sec = entry->sec = sample->time.seconds;
		depth = entry->depth = sample->depth.mm;
		entry->same_cylinder = sample->cylinderindex == cylinderindex;
		entry->cylinderindex = cylinderindex = sample->cylinderindex;
		SENSOR_PRESSURE(entry) = sample->cylinderpressure.mbar;
		/* track the segments per cylinder and their pressure/time integral */
		if (!entry->same_cylinder) {
			current->end = SENSOR_PRESSURE(entry-1);
			current->t_end = (entry-1)->sec;
			current = pr_track_alloc(SENSOR_PRESSURE(entry), entry->sec);
			track_pr[cylinderindex] = list_add(track_pr[cylinderindex], current);
		} else { /* same cylinder */
			if ((!SENSOR_PRESSURE(entry) && SENSOR_PRESSURE(entry-1)) ||
				(SENSOR_PRESSURE(entry) && !SENSOR_PRESSURE(entry-1))) {
				/* transmitter changed its working status */
				current->end = SENSOR_PRESSURE(entry-1);
				current->t_end = (entry-1)->sec;
				current = pr_track_alloc(SENSOR_PRESSURE(entry), entry->sec);
				track_pr[cylinderindex] =
					list_add(track_pr[cylinderindex], current);
			}
		}
		/* finally, do the discrete integration to get the SAC rate equivalent */
		current->pressure_time += (entry->sec - (entry-1)->sec) *
						(1 + entry->depth / 10000.0);
		missing_pr |= !SENSOR_PRESSURE(entry);
		entry->temperature = sample->temperature.mkelvin;

		if (depth || lastdepth)
			lastindex = i+2;

		lastdepth = depth;
		if (depth > pi->maxdepth)
			pi->maxdepth = depth;
	}
	current->t_end = entry->sec;
	for (cyl = 0; cyl < MAX_CYLINDERS; cyl++) { /* initialize the end pressures */
		int pr = dive->cylinder[cyl].end.mbar;
		if (pr && track_pr[cyl]) {
			pr_track = list_last(track_pr[cyl]);
			pr_track->end = pr;
		}
	}
	if (lastdepth)
		lastindex = i + 2;
	/* Fill in the last two entries with empty values but valid times */
	i = dive->samples + 2;
	pi->entry[i].sec = sec + 20;
	pi->entry[i+1].sec = sec + 40;
	pi->nr = lastindex+1;
	pi->maxtime = pi->entry[lastindex].sec;

	pi->endpressure = pi->minpressure = dive->cylinder[0].end.mbar;
	pi->maxpressure = dive->cylinder[0].start.mbar;

	pi->meandepth = dive->meandepth.mm;

	if (missing_pr) {
		fill_missing_tank_pressures(dive, pi, track_pr);
	}
	for (cyl = 0; cyl < MAX_CYLINDERS; cyl++)
		list_free(track_pr[cyl]);
	return analyze_plot_info(pi);
}

void plot(struct graphics_context *gc, cairo_rectangle_int_t *drawing_area, struct dive *dive)
{
	struct plot_info *pi = create_plot_info(dive);

	cairo_translate(gc->cr, drawing_area->x, drawing_area->y);
	cairo_set_line_width(gc->cr, 2);
	cairo_set_line_cap(gc->cr, CAIRO_LINE_CAP_ROUND);
	cairo_set_line_join(gc->cr, CAIRO_LINE_JOIN_ROUND);

	/*
	 * We can use "cairo_translate()" because that doesn't
	 * scale line width etc. But the actual scaling we need
	 * do set up ourselves..
	 *
	 * Snif. What a pity.
	 */
	gc->maxx = (drawing_area->width - 2*drawing_area->x);
	gc->maxy = (drawing_area->height - 2*drawing_area->y);

	/* Temperature profile */
	plot_temperature_profile(gc, pi);

	/* Cylinder pressure plot */
	plot_cylinder_pressure(gc, pi);

	/* Depth profile */
	plot_depth_profile(gc, pi);
	plot_events(gc, pi, dive);

	/* Text on top of all graphs.. */
	plot_temperature_text(gc, pi);
	plot_depth_text(gc, pi);
	plot_cylinder_pressure_text(gc, pi);

	/* Bounding box last */
	gc->leftx = 0; gc->rightx = 1.0;
	gc->topy = 0; gc->bottomy = 1.0;

	set_source_rgb(gc, 1, 1, 1);
	move_to(gc, 0, 0);
	line_to(gc, 0, 1);
	line_to(gc, 1, 1);
	line_to(gc, 1, 0);
	cairo_close_path(gc->cr);
	cairo_stroke(gc->cr);

	free(pi);
}
