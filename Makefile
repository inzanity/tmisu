
TARGET	=	tmisu
SRC    :=	src/tmisu.c src/output.c src/command.c
HEDRS  :=	src/tmisu.h src/output.h src/command.h

PREFIX ?=	/usr/local

CFLAGS +=	-W -Wall -std=c99
IFLAGS	=	$(shell pkg-config --cflags dbus-1)
LFLAGS	=	$(shell pkg-config --libs dbus-1)
OBJ    :=	$(patsubst %.c,%.o,$(SRC))

all: $(TARGET)

%.o: %.c $(HDRS)
	$(CC) -c $< $(CFLAGS) $(IFLAGS) -o $@

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(IFLAGS) $(OBJ) $(LFLAGS) $(LDFLAGS) -o $(TARGET)

install: $(TARGET)
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	install $(TARGET) $(DESTDIR)$(PREFIX)/bin/$(TARGET)

clean:
	$(RM) -f $(OBJ) $(TARGET)
