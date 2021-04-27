#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>

#include "output.h"
#include "tmisu.h"

static void print_sanitized(const char *string, const char *escape) {
	while (*string) {
		size_t len = strcspn(string, escape);
		printf("%.*s", (int)len, string);
		string += len;
		while (*string && strchr(escape, *string)) {
			if (*string == '\n')
				printf("\\n");
			else
				printf("\\%c", *string);
			string++;
		}
	}
}

static void hints_output_iterator(DBusMessageIter *hints, const char *key_prefix, const char *key_suffix, const char *str_prefix, const char *str_suffix, const char *escape, const char *delimiter)
{
	for (; dbus_message_iter_get_arg_type(hints) != DBUS_TYPE_INVALID; dbus_message_iter_next(hints)) {
		DBusMessageIter dictentry;
		DBusMessageIter variant;
		DBusBasicValue value;
		const char *key;

		dbus_message_iter_recurse(hints, &dictentry);
		dbus_message_iter_get_basic(&dictentry, &key);
		dbus_message_iter_next(&dictentry);
		dbus_message_iter_recurse(&dictentry, &variant);

		printf("%s", key_prefix);
		print_sanitized(key, escape);
		printf("%s", key_suffix);

		if (!dbus_type_is_basic(dbus_message_iter_get_arg_type(&variant))) {
			printf("null%s", delimiter);
			continue;
		}

		dbus_message_iter_get_basic(&variant, &value);

		switch (dbus_message_iter_get_arg_type(&variant)) {
		case DBUS_TYPE_BYTE:
			printf("%" PRIu8 "%s", value.byt, delimiter);
			break;
		case DBUS_TYPE_BOOLEAN:
			printf("%s%s", value.bool_val ? "true" : "false", delimiter);
			break;
		case DBUS_TYPE_INT16:
			printf("%" PRId16 "%s", value.i16, delimiter);
			break;
		case DBUS_TYPE_UINT16:
			printf("%" PRIu16 "%s", value.u16, delimiter);
			break;
		case DBUS_TYPE_INT32:
			printf("%" PRId32 "%s", value.i32, delimiter);
			break;
		case DBUS_TYPE_UINT32:
			printf("%" PRIu32 "%s", value.u32, delimiter);
			break;
		case DBUS_TYPE_INT64:
			printf("%" PRId64 "%s", value.i64, delimiter);
			break;
		case DBUS_TYPE_UINT64:
			printf("%" PRIu64 "%s", value.u64, delimiter);
			break;
		case DBUS_TYPE_DOUBLE:
			printf("%lf%s", value.dbl, delimiter);
			break;
		case DBUS_TYPE_STRING:
		case DBUS_TYPE_OBJECT_PATH:
			dbus_message_iter_get_basic(&variant, &value);
			printf("%s", str_prefix);
			print_sanitized(value.str, escape);
			printf("%s%s", str_suffix, delimiter);
			continue;
			break;
		default:
			printf("null%s", delimiter);
			continue;
		}
	}
}

static void default_output(const char *app_name, const char *app_icon, dbus_uint32_t id, dbus_uint32_t replaces_id,
			   dbus_int32_t timeout, DBusMessageIter *hints, DBusMessageIter *actions, const char *summary,
			   const char *body, const char *delimiter)
{
	(void)id;
	printf("app_name: ");
	print_sanitized(app_name, "\n\\");
	printf("%sapp_icon: ", delimiter);
	print_sanitized(app_icon, "\n\\");
	printf("%sreplaces_id: %" PRIu32 "%stimeout: %" PRId32 "%s", delimiter,
	       replaces_id, delimiter,
	       timeout, delimiter);

	printf("hints:%s", delimiter);
	hints_output_iterator(hints, "\t", ": ", "", "", "\n\\", delimiter);

	printf("actions:%s", delimiter);
	while (dbus_message_iter_get_arg_type(actions) != DBUS_TYPE_INVALID) {
		if (!dbus_message_iter_has_next(actions))
			break;
		const char *key;
		const char *value;

		dbus_message_iter_get_basic(actions, &key);
		dbus_message_iter_next(actions);
		dbus_message_iter_get_basic(actions, &value);
		dbus_message_iter_next(actions);

		printf("\t");
		print_sanitized(key, "\n\\");
		printf(": ");
		print_sanitized(value, "\n\\");
		printf("%s", delimiter);
	}

	printf("body: ");
	print_sanitized(body, "\n\\");
	printf("%ssummary: ", delimiter);
	print_sanitized(summary, "\n\\");
	printf("%s", delimiter);
}

static void json_output(const char *app_name, const char *app_icon, dbus_uint32_t id, dbus_uint32_t replaces_id,
			dbus_int32_t timeout, DBusMessageIter *hints, DBusMessageIter *actions, const char *summary,
			const char *body, const char *delimiter) {

	printf("{"
	       "\"id\": %" PRIu32 ", "
	       "\"app_name\": \"", id);
	print_sanitized(app_name, "\n\\\"");
	printf("\", "
	       "\"app_icon\": \"");
	print_sanitized(app_icon, "\n\\\"");
	printf("\", "
	       "\"replaces_id\": %" PRIu32 ", "
	       "\"timeout\": %" PRId32 ", ",
	       replaces_id, timeout);

	printf("\"hints\": {");
	hints_output_iterator(hints, "\"", "\": ", "\"", "\"", "\n\\\"", ", ");
	printf("}, \"actions\": {");

	while (dbus_message_iter_get_arg_type(actions) != DBUS_TYPE_INVALID) {
		if (!dbus_message_iter_has_next(actions))
			break;
		const char *key;
		const char *value;

		dbus_message_iter_get_basic(actions, &key);
		dbus_message_iter_next(actions);
		dbus_message_iter_get_basic(actions, &value);
		dbus_message_iter_next(actions);

		printf("\"");
		print_sanitized(key, "\n\\\"");
		printf("\"");
		print_sanitized(value, "\n\\\"");
		printf("\",");
	}

	printf("}, ");
	printf("\"summary\": \"");
	print_sanitized(summary, "\n\\\"");
	printf("\", "
	       "\"body\": \"");
	print_sanitized(body, "\n\\\"");
	printf("\"}%s", delimiter);
}

void output_notification(DBusMessage *message, dbus_uint32_t id, enum output_format fmt, const char *delimiter)
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

	switch (fmt) {
	case FORMAT_JSON:
		json_output(app_name, app_icon, id, replaces_id,
			    timeout, &hints, &actions, summary, body,
			    delimiter);
		break;
	case FORMAT_TEXT:
	default:
		default_output(app_name, app_icon, id, replaces_id,
			       timeout, &hints, &actions, summary, body,
			       delimiter);
	}

	fflush(stdout);
}

