// SPDX-License-Identifier: GPL-2.0
#include "statsaxis.h"
#include "statscolors.h"
#include "statstranslations.h"
#include "statsvariables.h"
#include "zvalues.h"
#include "core/pref.h"
#include "core/subsurface-time.h"
#include <math.h> // for lrint
#include <numeric>
#include <QChart>
#include <QFontMetrics>
#include <QLocale>

// Define most constants for horizontal and vertical axes for more flexibility.
// Note: *Horizontal means that this is for the horizontal axis, so a vertical space.
static const double axisWidth = 1.0;
static const double axisTickWidth = 1.0;
static const double axisTickSizeHorizontal = 6.0;
static const double axisTickSizeVertical = 6.0;
static const double axisLabelSpaceHorizontal = 2.0;	// Space between axis or ticks and labels
static const double axisLabelSpaceVertical = 2.0;	// Space between axis or ticks and labels
static const double axisTitleSpaceHorizontal = 2.0;	// Space between labels and title
static const double axisTitleSpaceVertical = 2.0;	// Space between labels and title

StatsAxis::StatsAxis(QtCharts::QChart *chart, bool horizontal, bool labelsBetweenTicks) :
	QGraphicsLineItem(chart),
	chart(chart), horizontal(horizontal), labelsBetweenTicks(labelsBetweenTicks),
	labelFont(QFont()),	// make this configurable
	titleFont(QFont(labelFont.family(), labelFont.pointSize(), QFont::Bold, false)), // same as label, but bold
	size(1.0), zeroOnScreen(0.0), min(0.0), max(1.0)
{
	setPen(QPen(axisColor, axisWidth));
	setZValue(ZValues::axes);
}

StatsAxis::~StatsAxis()
{
}

std::pair<double, double> StatsAxis::minMax() const
{
	return { min, max };
}

void StatsAxis::setRange(double minIn, double maxIn)
{
	min = minIn;
	max = maxIn;
}

// Guess the number of tick marks based on example strings.
// We will use minimum and maximum values, which are not necessarily the
// maximum-size strings especially, when using proportional fonts or for
// categorical data. Therefore, try to err on the safe side by adding enough
// margins.
int StatsAxis::guessNumTicks(const std::vector<QString> &strings) const
{
	QFontMetrics fm(labelFont);
	int minSize = fm.height();
	for (const QString &s: strings) {
		QSize labelSize = fm.size(Qt::TextSingleLine, s);
		int needed = horizontal ? labelSize.width() : labelSize.height();
		if (needed > minSize)
			minSize = needed;
	}

	// Add space between labels
	if (horizontal)
		minSize = minSize * 3 / 2;
	else
		minSize *= 2;
	int numTicks = lrint(size / minSize);
	return std::max(numTicks, 2);
}

double StatsAxis::width() const
{
	if (horizontal)
		return 0.0;	// Only supported for vertical axes
	double labelWidth = 0.0;
	for (const Label &label: labels) {
		double w = label.label->boundingRect().width();
		if (w > labelWidth)
			labelWidth = w;
	}
	return labelWidth + axisLabelSpaceVertical +
	       QFontMetrics(titleFont).height() + axisTitleSpaceVertical +
	       (labelsBetweenTicks ? 0.0 : axisTickSizeVertical);
}

double StatsAxis::height() const
{
	if (!horizontal)
		return 0.0;	// Only supported for horizontal axes
	return QFontMetrics(labelFont).height() + axisLabelSpaceHorizontal +
	       QFontMetrics(titleFont).height() + axisTitleSpaceHorizontal +
	       (labelsBetweenTicks ? 0.0 : axisTickSizeHorizontal);
}

StatsAxis::Label::Label(const QString &name, double pos, QtCharts::QChart *chart, const QFont &font) :
	label(new QGraphicsSimpleTextItem(name, chart)),
	pos(pos)
{
	label->setBrush(QBrush(darkLabelColor));
	label->setFont(font);
	label->setZValue(ZValues::axes);
}

void StatsAxis::addLabel(const QString &label, double pos)
{
	labels.emplace_back(label, pos, chart, labelFont);
}

StatsAxis::Tick::Tick(double pos, QtCharts::QChart *chart) :
	item(new QGraphicsLineItem(chart)),
	pos(pos)
{
	item->setPen(QPen(axisColor, axisTickWidth));
	item->setZValue(ZValues::axes);
}

void StatsAxis::addTick(double pos)
{
	ticks.emplace_back(pos, chart);
}

// Map x (horizontal) or y (vertical) coordinate to or from screen coordinate
double StatsAxis::toScreen(double pos) const
{
	// Vertical is bottom-up
	return horizontal ? (pos - min) / (max - min) * size + zeroOnScreen
			  : (min - pos) / (max - min) * size + zeroOnScreen;
}

double StatsAxis::toValue(double pos) const
{
	// Vertical is bottom-up
	return horizontal ? (pos - zeroOnScreen) / size * (max - min) + min
			  : (zeroOnScreen - pos) / size * (max - min) + zeroOnScreen;
}

void StatsAxis::setSize(double sizeIn)
{
	size = sizeIn;
	updateLabels();
}

void StatsAxis::setPos(QPointF pos)
{
	if (horizontal) {
		zeroOnScreen = pos.x();
		double labelY = pos.y() + axisLabelSpaceHorizontal +
				(labelsBetweenTicks ? 0.0 : axisTickSizeHorizontal);
		double y = pos.y();
		for (Label &label: labels) {
			double x = toScreen(label.pos) - label.label->boundingRect().width() / 2.0;
			label.label->setPos(QPointF(x, labelY));
		}
		for (Tick &tick: ticks) {
			double x = toScreen(tick.pos);
			tick.item->setLine(x, y, x, y + axisTickSizeHorizontal);
		}
		setLine(zeroOnScreen, y, zeroOnScreen + size, y);
	} else {
		double fontHeight = QFontMetrics(labelFont).height();
		zeroOnScreen = pos.y();
		double x = pos.x();
		double labelX = x - axisLabelSpaceVertical -
				(labelsBetweenTicks ? 0.0 : axisTickSizeVertical);
		for (Label &label: labels) {
			double y = toScreen(label.pos) - fontHeight / 2.0;
			label.label->setPos(QPointF(labelX - label.label->boundingRect().width(), y));
		}
		for (Tick &tick: ticks) {
			double y = toScreen(tick.pos);
			tick.item->setLine(x, y, x - axisTickSizeVertical, y);
		}
		setLine(x, zeroOnScreen, x, zeroOnScreen - size);
	}
}

ValueAxis::ValueAxis(QtCharts::QChart *chart, double min, double max, int decimals, bool horizontal) :
	StatsAxis(chart, horizontal, false),
	min(min), max(max), decimals(decimals)
{
}

void ValueAxis::updateLabels()
{
	using QtCharts::QValueAxis;
	labels.clear();
	ticks.clear();

	// Avoid degenerate cases
	if (max - min < 0.0001) {
		max += 0.5;
		min -= 0.5;
	}

	QLocale loc;
	QString minString = loc.toString(min, 'f', decimals);
	QString maxString = loc.toString(max, 'f', decimals);
	int numTicks = guessNumTicks({ minString, maxString});

	// Use full decimal increments
	double height = max - min;
	double inc = height / numTicks;
	double digits = floor(log10(inc));
	int digits_int = lrint(digits);
	double digits_factor = pow(10.0, digits);
	int inc_int = std::max((int)ceil(inc / digits_factor), 1);
	// Do "nice" increments of the leading digit: 1, 2, 4, 5.
	if (inc_int > 5)
		inc_int = 10;
	if (inc_int == 3)
		inc_int = 4;
	inc = inc_int * digits_factor;
	if (-digits_int > decimals)
		decimals = -digits_int;

	double actMin = floor(min /  inc) * inc;
	double actMax = ceil(max /  inc) * inc;
	int num = lrint((actMax - actMin) / inc);
	setRange(actMin, actMax);

	double actStep = (actMax - actMin) / static_cast<double>(num);
	double act = actMin;
	labels.reserve(num + 1);
	ticks.reserve(num + 1);
	for (int i = 0; i <= num; ++i) {
		addLabel(loc.toString(act, 'f', decimals), act);
		addTick(act);
		act += actStep;
	}
}

CountAxis::CountAxis(QtCharts::QChart *chart, int count, bool horizontal) :
	ValueAxis(chart, 0.0, (double)count, 0, horizontal),
	count(count)
{
}

void CountAxis::updateLabels()
{
	labels.clear();
	ticks.clear();

	QLocale loc;
	QString countString = loc.toString(count);
	int numTicks = guessNumTicks({ countString });

	// Get estimate of step size
	if (count <= 0)
		count = 1;
	// When determining the step size, make sure to round up
	int step = (count + numTicks - 1) / numTicks;
	if (step <= 0)
		step = 1;

	// Get the significant first or first two digits
	int scale = 1;
	int significant = step;
	while (significant > 25) {
		significant /= 10;
		scale *= 10;
	}

	for (int increment: { 1, 2, 4, 5, 10, 15, 20, 25 }) {
		if (increment >= significant) {
			significant = increment;
			break;
		}
	}
	step = significant * scale;

	// Make maximum an integer number of steps, equal or greater than the needed counts
	int num = (count - 1) / step + 1;
	int max = num * step;

	setRange(0, max);

	labels.reserve(max + 1);
	ticks.reserve(max + 1);
	for (int i = 0; i <= max; i += step) {
		addLabel(loc.toString(i), static_cast<double>(i));
		addTick(static_cast<double>(i));
	}
}

CategoryAxis::CategoryAxis(QtCharts::QChart *chart, const std::vector<QString> &labelsIn, bool horizontal) :
	StatsAxis(chart, horizontal, true)
{
	labels.reserve(labelsIn.size());
	ticks.reserve(labelsIn.size() + 1);
	double pos = 0.0;
	addTick(-0.5);
	for (const QString &s: labelsIn) {
		addLabel(s, pos);
		addTick(pos + 0.5);
		pos += 1.0;
	}
	setRange(-0.5, static_cast<double>(labelsIn.size()) + 0.5);
}

void CategoryAxis::updateLabels()
{
}

HistogramAxis::HistogramAxis(QtCharts::QChart *chart, std::vector<HistogramAxisEntry> bins, bool horizontal) :
	StatsAxis(chart, horizontal, false),
	bin_values(std::move(bins))
{
	if (bin_values.size() < 2) // Less than two makes no sense -> there must be at least one category
		return;

	// The caller can declare some bin labels as preferred, when there are
	// too many labels to show all. Try to infer the preferred step size
	// by finding two consecutive preferred labels. This supposes that
	// the preferred labels are equi-distant and that the caller does not
	// use large prime (or nearly prime) steps.
	auto it1 = std::find_if(bin_values.begin(), bin_values.end(),
				[](const HistogramAxisEntry &e) { return e.recommended; });
	auto next_it = it1 == bin_values.end() ? it1 : std::next(it1);
	auto it2 = std::find_if(next_it, bin_values.end(),
				[](const HistogramAxisEntry &e) { return e.recommended; });
	preferred_step = it2 == bin_values.end() ? 1 : it2 - it1;
	setRange(bin_values.front().value, bin_values.back().value);
}

// Initialize a histogram axis with the given labels. Labels are specified as (name, value, recommended) triplets.
// If labels are skipped, try to skip it in such a way that a recommended label is shown.
// The one example where this is relevant is the quarterly bins, which are formated as (2019, q1, q2, q3, 2020, ...).
// There, we obviously want to show the years and not the quarters.
void HistogramAxis::updateLabels()
{
	labels.clear();
	ticks.clear();

	if (bin_values.size() < 2) // Less than two makes no sense -> there must be at least one category
		return;

	std::vector<QString> strings;
	strings.reserve(bin_values.size());
	for (auto &[name, value, recommended]: bin_values)
		strings.push_back(name);
	int maxLabels = guessNumTicks(strings);

	int step = ((int)bin_values.size() - 1) / maxLabels + 1;
	if (step < preferred_step) {
		if (step * 2 > preferred_step)  {
			step = preferred_step;
		} else {
			int gcd = std::gcd(step, preferred_step);
			while (preferred_step % step != 0)
				step += gcd;
		}
	} else if (step > preferred_step) {
		int remainder = (step + preferred_step) % preferred_step;
		if (remainder != 0)
			step = step + preferred_step - remainder;
	}
	int first = 0;
	if (step > 1) {
		for (int i = 0; i < (int)bin_values.size(); ++i) {
			const auto &[name, value, recommended] = bin_values[i];
			if (recommended) {
				first = i % step;
				break;
			}
		}
	}
	labels.reserve((bin_values.size() - first) / step + 1);
	for (int i = first; i < (int)bin_values.size(); i += step) {
		const auto &[name, value, recommended] = bin_values[i];
		addLabel(name, value);
		addTick(value);
	}
}

// Helper function to turn days since "Unix epoch" into a timestamp_t
static const double seconds_in_day = 86400.0;
static timestamp_t double_to_timestamp(double d)
{
	return timestamp_t{ lrint(d * seconds_in_day) };
}

// Turn double to (year, month) pair
static std::pair<int, int> double_to_month(double d)
{
	struct tm tm;
	utc_mkdate(double_to_timestamp(d), &tm);
	return { tm.tm_year, tm.tm_mon };
}

// Increase (year, month) pair by one month
static void inc(std::pair<int, int> &ym)
{
	if (++ym.second >= 12) {
		++ym.first;
		ym.second = 0;
	}
}

static std::array<int, 3> double_to_day(double d)
{
	struct tm tm;
	utc_mkdate(double_to_timestamp(d), &tm);
	return { tm.tm_year, tm.tm_mon, tm.tm_mday };
}

// This is trashy: to increase a day, turn into timestamp and back.
// This surely can be done better.
static void inc(std::array<int, 3> &ymd)
{
	struct tm tm = { 0 };
	tm.tm_year = ymd[0];
	tm.tm_mon = ymd[1];
	tm.tm_mday = ymd[2] + 1;
	timestamp_t t = utc_mktime(&tm);
	utc_mkdate(t, &tm);
	ymd = { tm.tm_year, tm.tm_mon, tm.tm_mday };
}

// Use heuristics to determine the preferred day/month format:
// Try to see whether day or month comes first and try to extract
// the separator character. Returns a (day_first, separator) pair.
static std::pair<bool, char> day_format()
{
	const char *fmt = prefs.date_format;
	const char *d, *m, *sep;
	for (d = fmt; *d && *d != 'd' && *d != 'D'; ++d)
		;
	for (m = fmt; *m && *m != 'm' && *m != 'M'; ++m)
		;
	for(sep = std::min(m, d); *sep == 'm' || *sep == 'M' || *sep == 'd' || *sep == 'D'; ++sep)
		;
	return { d < m, *sep ? *sep : '.' };
}

// For now, misuse the histogram axis for creating a time axis. Depending on the range,
// create year, month or day-based bins. This is certainly not efficient and may need
// some tuning. However, it should ensure that no crazy number of bins is generated.
// Ultimately, this should be replaced by a better and dynamic scheme
// From and to are given in seconds since "epoch".
static std::vector<HistogramAxisEntry> timeRangeToBins(double from, double to)
{
	// from and two are given in days since the "Unix epoch".
	// The lowest precision we do is two days.
	if (to - from < 2.0) {
		double center = (from + to) / 2.0;
		from = center + 1.0;
		to = center - 1.0;
	}

	std::vector<HistogramAxisEntry> res;
	if (to - from > 2.0 * 356.0) {
		// For two years or more, do year based bins
		int year_from = utc_year(double_to_timestamp(from));
		int year_to = utc_year(double_to_timestamp(to)) + 1;
		for (int year = year_from; year <= year_to; ++year)
			res.push_back({ QString::number(year), date_to_double(year, 0, 0), true });
	} else if (to - from > 2.0 * 30.0) {
		// For two months or more, do month based bins
		auto year_month_from = double_to_month(from);
		auto year_month_to = double_to_month(to);
		inc(year_month_to);
		for (auto act = year_month_from; act <= year_month_to; inc(act)) {
			double val = date_to_double(act.first, act.second, 0);
			if (act.second == 0)
				res.push_back({ QString::number(act.first), val, true });
			else
				res.push_back({ monthname(act.second), val, false });
		}
	} else {
		// For less than two months, do date based bins
		auto day_from = double_to_day(from);
		auto day_to = double_to_day(to);
		inc(day_to);
		auto [day_before_month, separator] = day_format();
		QString format = day_before_month ? QStringLiteral("%1%2%3")
						  : QStringLiteral("%3%2%1");
		QString sep = QString(separator);
		for (auto act = day_from; act < day_to; inc(act)) {
			double val = date_to_double(act[0], act[1], act[2]);
			if (act[1] == 0) {
				res.push_back({ QString::number(act[0]), val, true });
			} else if (act[2] == 0) {
				res.push_back({ monthname(act[1]), val, true });
			} else {
				QString s = format.arg(QString::number(act[2]), sep, QString::number(act[1]));
				res.push_back({s, val, true });
			}
		}
	}
	return res;
}

DateAxis::DateAxis(QtCharts::QChart *chart, double from, double to, bool horizontal) :
	HistogramAxis(chart, timeRangeToBins(from, to), horizontal)
{
}
