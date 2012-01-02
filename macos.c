/* macos.c */
/* implements Mac OS X specific functions */
#include "display-gtk.h"
#include <CoreFoundation/CoreFoundation.h>
#include <mach-o/dyld.h>
#include "gtkosxapplication.h"

static GtkOSXApplication *theApp;

/* macos defines CFSTR to create a CFString object from a constant,
 * but no similar macros if a C string variable is supposed to be
 * the argument. We add this here (hardcoding the default allocator
 * and MacRoman encoding */
#define CFSTR_VAR(_var) CFStringCreateWithCStringNoCopy(kCFAllocatorDefault,	\
					(_var), kCFStringEncodingMacRoman,	\
					kCFAllocatorNull)

#define SUBSURFACE_PREFERENCES CFSTR("org.hohndel.subsurface")
#define REL_ICON_PATH "Resources/Subsurface.icns"
#define UI_FONT "Arial Unicode MS 12"
#define DIVELIST_MAC_DEFAULT_FONT "Arial Unicode MS 9"

void subsurface_open_conf(void)
{
}

void subsurface_set_conf(char *name, pref_type_t type, const void *value)
{
	switch (type) {
	case PREF_BOOL:
		CFPreferencesSetAppValue(CFSTR_VAR(name), value == NULL ? kCFBooleanFalse : kCFBooleanTrue, SUBSURFACE_PREFERENCES);
		break;
	case PREF_STRING:
		CFPreferencesSetAppValue(CFSTR_VAR(name), CFSTR_VAR(value), SUBSURFACE_PREFERENCES);
	}
}

const void *subsurface_get_conf(char *name, pref_type_t type)
{
	Boolean boolpref;
	CFPropertyListRef strpref;

	switch (type) {
	case PREF_BOOL:
		boolpref =  CFPreferencesGetAppBooleanValue(CFSTR_VAR(name), SUBSURFACE_PREFERENCES, FALSE);
		if (boolpref)
			return (void *) 1;
		else
			return NULL;
	case PREF_STRING:
		strpref = CFPreferencesCopyAppValue(CFSTR_VAR(name), SUBSURFACE_PREFERENCES);
		if (!strpref) {
			return NULL;
		}
		return CFStringGetCStringPtr(strpref, kCFStringEncodingMacRoman);
	}
	/* we shouldn't get here, but having this line makes the compiler happy */
	return NULL;
}

void subsurface_close_conf(void)
{
	int ok = CFPreferencesAppSynchronize(SUBSURFACE_PREFERENCES);
	if (!ok) {
		printf("Could not save preferences\n");
	}
}

const char *subsurface_USB_name()
{
	return "/dev/tty.SLAB_USBtoUART";
}

const char *subsurface_icon_name()
{
	static char path[1024];
	char *ptr1, *ptr2;
	uint32_t size = sizeof(path); /* need extra space to copy icon path */
	if (_NSGetExecutablePath(path, &size) == 0) {
		ptr1 = strcasestr(path,"MacOS/subsurface");
		ptr2 = strcasestr(path,"Contents");
		if (ptr1 && ptr2) {
			/* we are running as installed app from a bundle */
			if (ptr1 - path < size - strlen(REL_ICON_PATH)) {
				strcpy(ptr1,REL_ICON_PATH);
				return path;
			}
		}
	}
	return "packaging/macosx/Subsurface.icns";
}

void subsurface_ui_setup(GtkSettings *settings, GtkWidget *menubar,
		GtkWidget *vbox)
{
	if (!divelist_font)
		divelist_font = DIVELIST_MAC_DEFAULT_FONT;
	g_object_set(G_OBJECT(settings), "gtk-font-name", UI_FONT, NULL);

	theApp = g_object_new(GTK_TYPE_OSX_APPLICATION, NULL);
	gtk_widget_hide (menubar);
	gtk_osxapplication_set_menu_bar(theApp, GTK_MENU_SHELL(menubar));
	gtk_osxapplication_set_use_quartz_accelerators(theApp, TRUE);
	gtk_osxapplication_ready(theApp);

}
