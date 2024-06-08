// SPDX-License-Identifier: GPL-2.0
// A structure that contains all the data we store in a divelog files
#ifndef DIVELOG_H
#define DIVELOG_H

#include "divelist.h"
#include "divesitetable.h"
#include "triptable.h"

#include <memory>
#include <vector>

struct trip_table;
struct device;
struct filter_preset_table;

struct divelog {
	dive_table dives;
	trip_table trips;
	dive_site_table sites;
	std::vector<device> devices;
	std::unique_ptr<filter_preset_table> filter_presets;
	bool autogroup;

	divelog();
	~divelog();
	divelog(divelog &&); // move constructor (argument is consumed).
	divelog &operator=(divelog &&); // move assignment (argument is consumed).

	void delete_single_dive(int idx);
	void delete_multiple_dives(const std::vector<dive *> &dives);
	void clear();
	bool is_trip_before_after(const struct dive *dive, bool before) const;
};

extern struct divelog divelog;

#endif
