// SPDX-License-Identifier: GPL-2.0
// The background grid of a chart

#include <memory>
#include <vector>

class StatsAxis;
class StatsView;
class ChartLineItem;

class StatsGrid  {
public:
	StatsGrid(StatsView &view, const StatsAxis &xAxis, const StatsAxis &yAxis);
	void updatePositions();
private:
	StatsView &view;
	const StatsAxis &xAxis, &yAxis;
	std::vector<std::unique_ptr<ChartLineItem>> lines;
};
