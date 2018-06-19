- Equipment: fix bug with the maximum number of allowed tanks [#1401]
- Mobile: add cloud storage user name to global drawer banner
- Mobile: add button to About page to copy logs to clipboard; this makes it
  easier to provide the required information when reporting issues
- Mobile/iOS: adjusted launch page to be centered
- Mobile: adjusted size of about page for small screen (no scrolling)
- Mobile: download page now remembers last used connection
- Shearwater import: include DC reported ceiling
- Dive pictures: don't read invalid files twice
- Dive pictures: don't plot pictures twice on switching dives
- Dive pictures: don't replot all pictures on deletion
- Dive pictures: remove stale code for cloud storage of pictures
- Dive pictures: save thumbnails in individual files
- Dive pictures: don't keep all thumbnails in memory
- Dive pictures: automatically recalculate thumbnail if older than picture
- Planner: fix output of ICD-notes
- Desktop: fix crash on manual addition of dive to fresh logbook
- Desktop: fix showing of the manual
- Desktop: don't save empty file to cloud [#1228]
- Desktop: allow editing from list-view [#1213]
- Desktop: don't add six seconds to dive duration [#554]
- Desktop: fix manual editing of dive duration [#1211]
- Desktop: implement bailout events in logbook and planner
- Desktop: fix time format in main tab [#1234]
- Mobile: recalculate derived values after dive edit
- Mobile: fold dive trips to give better overview for large dive lists
- Mobile: fix using current GPS position when editing or adding dives
- Profile: add information about actual isobaric counter diffusion happening
- Profile: add information about actual isobaric counter diffusion happening
- Profile: fix memory leak if animations were turned off
- Profile: use local filename to load picture on thumbnail-click
- Profile: fix toggling of picture-visibility
- Suunto DM import: improved temperature parsing in special cases
# Always add new entries at the very top of this file above other
# existing entries.
# Use this layout for new entries:
# <Area>: <Details about the change> [reference thread / issue]
