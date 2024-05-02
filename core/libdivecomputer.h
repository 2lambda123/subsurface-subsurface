// SPDX-License-Identifier: GPL-2.0
#ifndef LIBDIVECOMPUTER_H
#define LIBDIVECOMPUTER_H

#include <stdint.h>
#include <stdio.h>

/* libdivecomputer */

#ifdef DC_VERSION /* prevent a warning with wingdi.h */
#undef DC_VERSION
#endif
#include <libdivecomputer/version.h>
#include <libdivecomputer/device.h>
#include <libdivecomputer/parser.h>

// Even if we have an old libdivecomputer, Uemis uses this
#ifndef DC_TRANSPORT_USBSTORAGE
#define DC_TRANSPORT_USBSTORAGE (1 << 6)
#define dc_usb_storage_open(stream, context, devname) (DC_STATUS_UNSUPPORTED)
#endif

#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif

struct dive;
struct divelog;
struct devices;

typedef struct {
	dc_descriptor_t *descriptor = nullptr;
	const char *vendor = nullptr, *product = nullptr, *devname = nullptr;
	const char *model = nullptr, *btname = nullptr;
	unsigned char *fingerprint = nullptr;
	unsigned int fsize = 0, fdeviceid = 0, fdiveid = 0;
	struct dc_event_devinfo_t devinfo = { };
	uint32_t diveid = 0;
	dc_device_t *device = nullptr;
	dc_context_t *context = nullptr;
	dc_iostream_t *iostream = nullptr;
	bool force_download = false;
	bool libdc_log = false;
	bool libdc_dump = false;
	bool bluetooth_mode = false;
	bool sync_time = false;
	FILE *libdc_logfile = nullptr;
	struct divelog *log = nullptr;
	void *androidUsbDeviceDescriptor = nullptr;
} device_data_t;

const char *errmsg (dc_status_t rc);
std::string do_libdivecomputer_import(device_data_t *data);
dc_status_t libdc_buffer_parser(struct dive *dive, device_data_t *data, unsigned char *buffer, int size);
void logfunc(dc_context_t *context, dc_loglevel_t loglevel, const char *file, unsigned int line, const char *function, const char *msg, void *userdata);
dc_descriptor_t *get_descriptor(dc_family_t type, unsigned int model);

extern int import_thread_cancelled;
extern const char *progress_bar_text;
extern void (*progress_callback)(const char *text);
extern double progress_bar_fraction;

dc_status_t ble_packet_open(dc_iostream_t **iostream, dc_context_t *context, const char* devaddr, void *userdata);
dc_status_t rfcomm_stream_open(dc_iostream_t **iostream, dc_context_t *context, const char* devaddr);
dc_status_t ftdi_open(dc_iostream_t **iostream, dc_context_t *context);
dc_status_t serial_usb_android_open(dc_iostream_t **iostream, dc_context_t *context, void *androidUsbDevice);

dc_status_t divecomputer_device_open(device_data_t *data);

unsigned int get_supported_transports(device_data_t *data);

#ifdef __cplusplus
}

#include <string>
extern std::string logfile_name;
extern std::string dumpfile_name;

#endif

#endif // LIBDIVECOMPUTER_H
