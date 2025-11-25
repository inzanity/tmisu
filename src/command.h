#ifndef COMMAND_H
#define COMMAND_H

#include <dbus/dbus.h>

void run_command(DBusMessage *message, const char *cmd, unsigned notification_id);

#endif
