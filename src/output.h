#pragma once

#include <dbus/dbus.h>

enum output_format {
	FORMAT_TEXT,
	FORMAT_JSON
};

void output_notification(DBusMessage *message, dbus_uint32_t id, enum output_format fmt, const char *delimiter);
