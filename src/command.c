#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <inttypes.h>

#include <unistd.h>
#include <sys/wait.h>

#include <dbus/dbus.h>

void run_command(DBusMessage *message, const char *cmd, unsigned notification_id)
{
	DBusMessageIter i;
	DBusMessageIter actions;
	DBusMessageIter hints;
	const char *app_name;
	dbus_uint32_t replaces_id;
	dbus_int32_t timeout;
	const char *app_icon;
	const char *summary;
	const char *body;
	const char *argv[4];
	char buf[sizeof(union { unsigned u; dbus_int32_t i; }) * 3 + 2];
	pid_t child;

	dbus_message_iter_init(message, &i);
	dbus_message_iter_get_basic(&i, &app_name);
	dbus_message_iter_next(&i);
	dbus_message_iter_get_basic(&i, &replaces_id);
	dbus_message_iter_next(&i);
	dbus_message_iter_get_basic(&i, &app_icon);
	dbus_message_iter_next(&i);
	dbus_message_iter_get_basic(&i, &summary);
	dbus_message_iter_next(&i);
	dbus_message_iter_get_basic(&i, &body);

	dbus_message_iter_next(&i);
	dbus_message_iter_recurse(&i, &actions);

	dbus_message_iter_next(&i);
	dbus_message_iter_recurse(&i, &hints);

	dbus_message_iter_next(&i);
	dbus_message_iter_get_basic(&i, &timeout);

	switch ((child = fork())) {
	case -1:
		perror("fork failed:");
		return;
	case 0:
		break;
	default:
		waitpid(child, NULL, 0);
		return;
	}

	if (fork() != 0)
		exit(0);

	sprintf(buf, "%u", notification_id);
	setenv("NOTIFICATION_ID", buf, 1);
	setenv("NOTIFICATION_ICON", app_icon, 1);
	setenv("NOTIFICATION_APP", app_name, 1);
	sprintf(buf, "%" PRIi32, timeout);
	setenv("NOTIFICATION_TIMEOUT", buf, 1);

	argv[0] = cmd;
	argv[1] = summary;
	argv[2] = body;
	argv[3] = NULL;

	execvp(cmd, (char **)argv);
	exit(1);
}
