- Desktop: fix issue with dive list row height in case of larger fonts [#1600]
- Mobile: add default cylinder functionality for new dives [#1535]
- Desktop/Export: fix bug related to quoted text when exporting to HTML [#1576]
- Desktop/Map-Widget: add support for filtering of map locations [#1581]
- Android: switch to Download page when dive computer is plugged in and try to
  populate vendor / device / connection based on the information we get
- Mobile: enable multiple cylinder editing for dives that uses more than one cylinder 
- Mac: fix problem downloading from divelogs.de
- Dive media: paint duration of videos on thumbnails in the dive-photo tab
- Dive media: extract video thumbnails using ffmpeg
- Dive media: implement "open folder of selected media files" [#1514]
- Dive media: show play-time of videos on the profile plot
- Dive media: draw thumbnails on top of deco-ceilings
- Dive media: Change term "photos" and "images" everywhere in the UI to "media files"
- mobile: show developer menu is now saved on disk and remebered between start of Subsurface
- tests: add qml test harness
- Dive media: experimental support for metadata extraction from AVI and WMV files
- Dive media: sort thumbnails by timestamp
- Dive media: don't recalculate all pictures on drag & drop
- Profile: immediately update thumbnail positions on deletion
---
* Always add new entries at the very top of this file above other existing entries and this note.
* Use this layout for new entries: `[Area]: [Details about the change] [reference thread / issue]`
