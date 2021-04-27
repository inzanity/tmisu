#ifndef TMISU_H
#define TMISU_H

#include <stdio.h>
#include <string.h>

extern char print_json;
extern const char *delimiter;

#define INTROSPECTION_XML "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"\
	"<node>\n"\
	"   <interface name=\"org.freedesktop.Notifications\">\n"\
	"       <method name=\"Notify\">\n"\
	"            <arg direction=\"in\"  type=\"s\"     name=\"app_name\"/>\n"\
	"            <arg direction=\"in\"  type=\"u\""\
	" name=\"replaces_id\"/>\n"\
	"            <arg direction=\"in\"  type=\"s\"     name=\"app_icon\"/>\n"\
	"            <arg direction=\"in\"  type=\"s\"     name=\"summary\"/>\n"\
	"            <arg direction=\"in\"  type=\"s\"     name=\"body\"/>\n"\
	"            <arg direction=\"in\"  type=\"as\"    name=\"actions\"/>\n"\
	"            <arg direction=\"in\"  type=\"a{sv}\" name=\"hints\"/>\n"\
	"            <arg direction=\"in\"  type=\"i\""\
	" name=\"expire_timeout\"/>\n"\
	"            <arg direction=\"out\" type=\"u\""\
	" name=\"id\"/>\n"\
	"       </method>\n"\
	"        <method name=\"GetServerInformation\">\n"\
	"            <arg direction=\"out\" type=\"s\" name=\"name\"/>\n"\
	"            <arg direction=\"out\" type=\"s\" name=\"vendor\"/>\n"\
	"            <arg direction=\"out\" type=\"s\" name=\"version\"/>\n"\
	"            <arg direction=\"out\" type=\"s\" name=\"spec_version\"/>\n"\
	"        </method>\n"\
	"        <method name=\"GetCapabilities\">\n"\
	"            <arg type=\"as\" name=\"capabilities\" direction=\"out\"/>\n"\
	"        </method>\n"\
	"   </interface>\n"\
	"</node>"

#define INTROSPECTION_NODE_XML "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"\
	"<node>\n"\
	"	<node name=\"%.*s\"/>\n"\
	"</node>"

#endif
