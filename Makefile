ifndef CC
CC = gcc
endif

ifndef CPPFLAGS
CPPFLAGS =
endif

ifndef CFLAGS
CFLAGS = -O0 -g
endif

ifndef LFFLAGS
LDFLAGS = -g
endif

BINDIR = bin
SRCDIR = src
TESTDIR = test

HEADERS = $(addprefix $(SRCDIR)/,globals.h commands.h)
OBJECTS = $(addprefix $(BINDIR)/,att.o commands.o connect.o)
EXAMPLES = $(addprefix $(BINDIR)/,test-led test-port-update test-motor-sync test-tilt-sensor)

.PHONY: all clean

all: $(EXAMPLES)

clean:
	rm $(BINDIR)/*.o $(BINDIR)/

$(BINDIR)/.keep :
	mkdir -p $(BINDIR)
	@touch $@

$(BINDIR)/%.o : $(SRCDIR)/%.c $(BINDIR)/.keep $(HEADERS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

$(BINDIR)/test-led: $(TESTDIR)/test_led.c $(OBJECTS)
	$(CC) $(LDFLAGS) -Isrc -o $@ $^ -lbluetooth

$(BINDIR)/test-port-update: $(TESTDIR)/test_port_update.c $(OBJECTS)
	$(CC) $(LDFLAGS) -Isrc -o $@ $^ -lbluetooth

$(BINDIR)/test-motor-sync: $(TESTDIR)/test_motor_sync.c $(OBJECTS)
	$(CC) $(LDFLAGS) -Isrc -o $@ $^ -lbluetooth

$(BINDIR)/test-tilt-sensor: $(TESTDIR)/test_tilt_sensor.c $(OBJECTS)
	$(CC) $(LDFLAGS) -Isrc -o $@ $^ -lbluetooth
