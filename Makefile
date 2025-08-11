CC = gcc
CFLAGS = `pkg-config --cflags gtk4 gio-2.0`
LDFLAGS = `pkg-config --libs gtk4 gio-2.0`

TARGET = Rarch_Installer
SRC = main.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

clean:
	rm -f $(TARGET)
