#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <getopt.h>
#include <dbus/dbus.h>

#include "tmisu.h"
#include "output.h"

struct conf {
	enum output_format fmt;
	const char *delimiter;
};

DBusHandlerResult handle_message(DBusConnection *connection, DBusMessage *message, void *user_data)
{
	static unsigned notification_id = 0;
	struct conf *cnf = user_data;

	if (dbus_message_is_method_call(message, "org.freedesktop.DBus.Introspectable", "Introspect")) {
		static const char *notificationpath = "/org/freedesktop/Notifications";
		const char *path = dbus_message_get_path(message);
		size_t pl = strlen(path);
		DBusMessage *reply;

		if (strncmp(notificationpath, path, strlen(path)))
			return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

		if (pl == 1)
			pl = 0;

		if (notificationpath[pl] == '/') {
			size_t npl = strcspn(notificationpath + pl + 1, "/");
			char *buffer = malloc(sizeof(INTROSPECTION_NODE_XML) + npl);
			sprintf(buffer, INTROSPECTION_NODE_XML, (int)npl, notificationpath + pl + 1);
			reply = dbus_message_new_method_return(message);
			dbus_message_append_args(reply,
						 DBUS_TYPE_STRING, &buffer,
						 DBUS_TYPE_INVALID);
			free(buffer);

			dbus_connection_send(connection, reply, NULL);
			dbus_message_unref(reply);
			return DBUS_HANDLER_RESULT_HANDLED;
		}

		if (notificationpath[pl])
			return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

		reply = dbus_message_new_method_return(message);
		dbus_message_append_args(reply,
					 DBUS_TYPE_STRING, &(const char *){ INTROSPECTION_XML },
					 DBUS_TYPE_INVALID);
		dbus_connection_send(connection, reply, NULL);
		dbus_message_unref(reply);
		return DBUS_HANDLER_RESULT_HANDLED;
	} else if (dbus_message_is_method_call(message, "org.freedesktop.Notifications", "GetServerInformation")) {
		DBusMessage *reply = dbus_message_new_method_return(message);
		dbus_message_append_args(reply,
					 DBUS_TYPE_STRING, &(const char *){ "tiramisu" },
					 DBUS_TYPE_STRING, &(const char *){ "Sweets" },
					 DBUS_TYPE_STRING, &(const char *){ "1.0" },
					 DBUS_TYPE_STRING, &(const char *){ "1.2" },
					 DBUS_TYPE_INVALID);
		dbus_connection_send(connection, reply, NULL);
		dbus_message_unref(reply);
		return DBUS_HANDLER_RESULT_HANDLED;
	} else if (dbus_message_is_method_call(message, "org.freedesktop.Notifications", "GetCapabilities")) {
		DBusMessage *reply = dbus_message_new_method_return(message);
		DBusMessageIter i;
		DBusMessageIter j;

		dbus_message_iter_init_append(reply, &i);
		dbus_message_iter_open_container(&i, DBUS_TYPE_ARRAY, DBUS_TYPE_STRING_AS_STRING, &j);
		dbus_message_iter_append_basic(&j, DBUS_TYPE_STRING, &(const char *){ "body" });
		dbus_message_iter_append_basic(&j, DBUS_TYPE_STRING, &(const char *){ "body-markup" });
		dbus_message_iter_close_container(&i, &j);

		dbus_connection_send(connection, reply, NULL);
		dbus_message_unref(reply);
		return DBUS_HANDLER_RESULT_HANDLED;
	} else if (dbus_message_is_method_call(message, "org.freedesktop.Notifications", "Notify")) {
		DBusMessage *reply;
		if (!dbus_message_has_signature(message, "susssasa{sv}i"))
			return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
		reply = dbus_message_new_method_return(message);
		output_notification(message, ++notification_id, cnf->fmt, cnf->delimiter);
		dbus_message_append_args(reply,
					 DBUS_TYPE_UINT32, &(dbus_uint32_t){ notification_id },
					 DBUS_TYPE_INVALID);
		dbus_connection_send(connection, reply, NULL);
		dbus_message_unref(reply);

		return DBUS_HANDLER_RESULT_HANDLED;
	}

	return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

DBusConnection *connection = NULL;

void sig_handler(int signal)
{
	(void)signal;
	dbus_connection_close(connection);
}

int main(int argc, char **argv) {
	/* Parse arguments */
	struct conf cnf = { FORMAT_TEXT, "\n" };

	char argument;
	while ((argument = getopt(argc, argv, "hjd:")) >= 0) {
		switch (argument) {
		case 'd':
			cnf.delimiter = optarg;
			break;
		case 'h':
			printf("%s\n",
			       "tiramisu -[h|d|j]\n"
			       "-h\tHelp dialog\n"
			       "-d\tDelimeter for default output style.\n"
			       "-j\tUse JSON output style\n");
			return EXIT_SUCCESS;
			break;
		case 'j':
			cnf.fmt = FORMAT_JSON;
			break;
		default:
			break;
		}
	}

	connection = dbus_bus_get_private(DBUS_BUS_SESSION, NULL);

	if (!connection) {
		fprintf(stderr, "Could not connect to D-Bus\n");
		return 1;
	}

	int result = dbus_bus_request_name(connection, "org.freedesktop.Notifications", DBUS_NAME_FLAG_REPLACE_EXISTING | DBUS_NAME_FLAG_DO_NOT_QUEUE, NULL);

	if (result != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
		dbus_connection_close(connection);
		dbus_connection_unref(connection);

		fprintf(stderr, "Could not acquire service name, is another notification daemon running?\n");
		return 1;
	}

	dbus_bus_add_match(connection, "interface=org.freedesktop.Notifications,path=/org/freedesktop/Notifications,type=method_call", NULL);
	dbus_bus_add_match(connection, "interface=org.freedesktop.DBus.Introspectable,method=Introspect,type=method_call", NULL);
	dbus_connection_add_filter(connection, handle_message, &cnf, NULL);

	signal(SIGINT, sig_handler);
	signal(SIGTERM, sig_handler);

	while (dbus_connection_read_write_dispatch(connection, -1));

	dbus_connection_unref(connection);

	return 0;
}
