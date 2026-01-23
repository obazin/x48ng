# Makefile to build x48ng without autotools
#
# The cc-option function and the C{,PP}FLAGS logic were copied from the
# fsverity-utils project.
# https://git.kernel.org/pub/scm/fs/fsverity/fsverity-utils.git/
# The governing license can be found in the LICENSE file or at
# https://opensource.org/license/MIT.

TARGETS = dist/x48ng dist/x48ng-checkrom dist/x48ng-dump2rom

VERSION_MAJOR = 1
VERSION_MINOR = 0
PATCHLEVEL = 0

PREFIX ?= /usr
DOCDIR ?= $(PREFIX)/doc/x48ng
MANDIR ?= $(PREFIX)/man

LUA_VERSION ?= lua
PKG_CONFIG ?= pkg-config

OPTIM ?= 2
FULL_WARNINGS ?= no
WITH_SDL ?= yes
WITH_GTK ?= yes
WITH_SDL2 = no

makeflags +=-j$(NUM_CORES) -l$(NUM_CORES)

cc-option = $(shell if $(CC) $(1) -c -x c /dev/null -o /dev/null > /dev/null 2>&1; \
		  then echo $(1); fi)

##############################
# No serviceable parts under #
##############################
GLIB2_CFLAGS = $(shell "$(PKG_CONFIG)" --cflags glib-2.0)
GLIB2_LDLIBS = $(shell "$(PKG_CONFIG)" --libs glib-2.0)

GTK_CFLAGS = $(shell "$(PKG_CONFIG)" --cflags gtk4) -DHAS_GTK=1
GTK_LDLIBS = $(shell "$(PKG_CONFIG)" --libs gtk4)

LUA_CFLAGS = $(shell "$(PKG_CONFIG)" --cflags $(LUA_VERSION))
LUA_LDLIBS = $(shell "$(PKG_CONFIG)" --libs $(LUA_VERSION))

NCURSES_CFLAGS = $(shell "$(PKG_CONFIG)" --cflags ncursesw) -DNCURSES_WIDECHAR=1 -DHAS_NCURSES=1
NCURSES_LDLIBS = $(shell "$(PKG_CONFIG)" --libs ncursesw) -lreadline

### SDL UI
ifeq ($(WITH_SDL), yes)
	SDL_CFLAGS = $(shell "$(PKG_CONFIG)" --cflags sdl3) -DHAS_SDL=1
	SDL_LDLIBS = $(shell "$(PKG_CONFIG)" --libs sdl3)
endif

ifeq ($(WITH_SDL2), yes)
	SDL_CFLAGS = $(shell "$(PKG_CONFIG)" --cflags sdl2) -DHAS_SDL=1 -DHAS_SDL2=1
	SDL_LDLIBS = $(shell "$(PKG_CONFIG)" --libs sdl2)
endif

ifeq ($(FULL_WARNINGS), no)
EXTRA_WARNING_CFLAGS := -Wno-unused-function \
	-Wno-redundant-decls \
	$(call cc-option,-Wno-maybe-uninitialized) \
	$(call cc-option,-Wno-discarded-qualifiers) \
	$(call cc-option,-Wno-uninitialized) \
	$(call cc-option,-Wno-ignored-qualifiers)
else
EXTRA_WARNING_CFLAGS := -Wunused-function \
	-Wredundant-decls \
	-fsanitize=thread \
	$(call cc-option,-Wunused-variable)
endif

override CFLAGS := -std=gnu2x \
	-Wall -Wextra -Wpedantic \
	-Wformat=2 -Wshadow \
	-Wwrite-strings -Wstrict-prototypes -Wold-style-definition \
	-Wnested-externs -Wmissing-include-dirs \
	-Wdouble-promotion \
	-Wno-sign-conversion \
	-Wno-unused-variable \
	-Wno-unused-parameter \
	-Wno-conversion \
	-Wno-format-nonliteral \
	$(call cc-option,-Wjump-misses-init) \
	$(call cc-option,-Wlogical-op) \
	$(call cc-option,-Wno-unknown-warning-option) \
	$(EXTRA_WARNING_CFLAGS) \
	$(SDL_CFLAGS) \
	$(GTK_CFLAGS) \
	$(NCURSES_CFLAGS) \
	$(LUA_CFLAGS) \
	$(DEBUG_CFLAGS) \
	-O$(OPTIM) \
	-D_GNU_SOURCE=1 \
	-DVERSION_MAJOR=$(VERSION_MAJOR) \
	-DVERSION_MINOR=$(VERSION_MINOR) \
	-DPATCHLEVEL=$(PATCHLEVEL) \
	-I./src/ \
	$(CFLAGS)

# src/ui4x/libui4x ############################
UI4X_DIR = src/ui4x
UI4X_LDLIBS = -L$(UI4X_DIR) -lui4x

libui4x:
	make -C $(UI4X_DIR) libui4x.a WITH_SDL=$(WITH_SDL) WITH_SDL2=$(WITH_SDL2) LUA_VERSION=$(LUA_VERSION) VERSION_MAJOR=$(VERSION_MAJOR) VERSION_MINOR=$(VERSION_MINOR) PATCHLEVEL=$(PATCHLEVEL) DATADIR=$(DATADIR)

clean-libui4x:
	$(MAKE) -C $(UI4X_DIR) clean

distclean-libui4x:
	$(MAKE) -C $(UI4X_DIR) distclean

pull-ui4x:
	git submodule update --recursive --remote
#######################

override LDLIBS := $(LDLIBS) \
	$(LUA_LDLIBS) \
	$(GLIB2_LDLIBS) \
	$(GTK_LDLIBS) \
	$(NCURSES_LDLIBS) \
	$(SDL_LDLIBS) \
	-lz \
	-lm \
	$(UI4X_LDLIBS)

SRC = src/options.c \
	src/romio.c \
	src/main.c \
	src/core/debugger.c \
	src/core/emulate.c \
	src/core/init.c \
	src/core/persistence.c \
	src/core/memory.c \
	src/core/registers.c \
	src/core/timers.c \
	src/core/serial.c \
	src/emulator_api.c
OBJS = $(SRC:.c=.o)

depfiles = $(objects:.o=.d)

# Have the compiler output dependency files with make targets for each
# of the object files. The `MT` option specifies the dependency file
# itself as a target, so that it's regenerated when it should be.
%.dep.mk: %.c
	$(CC) -M -MP -MT '$(<:.c=.o) $@' $(CPPFLAGS) $< > $@

# Include each of those dependency files; Make will run the rule above
# to generate each dependency file (if it needs to).
-include $(depfiles)

.PHONY: all clean clean-all pretty-code mrproper install uninstall

all: $(TARGETS)

dist/x48ng-dump2rom: src/legacy_tools/dump2rom.o
dist/x48ng-checkrom: src/legacy_tools/checkrom.o src/romio.o
dist/x48ng: $(OBJS) $(HEADERS) libui4x

# Binaries
$(TARGETS): libui4x
	$(CC) $(OBJS) -o $@ $(CFLAGS) $(LDFLAGS) $(LDLIBS)

# Cleaning
clean: clean-libui4x
	rm -f $(OBJS) src/legacy_tools/*.o src/*.dep.mk src/legacy_tools/*.dep.mk

mrproper: clean distclean-libui4x
	rm -f $(TARGETS)
	make -C dist/ROMs mrproper

clean-all: mrproper

# for clangd
compile_commands.json: mrproper
	bear -- make dist/x48ng

# Formatting
pretty-code:
	clang-format -i src/*.c src/*.h src/legacy_tools/*.c src/ui4x/src/*.c src/ui4x/src/*.h

# Installing
get-roms:
	make -C dist/ROMs

dist/config.lua: dist/x48ng
	$^ --print-config > $@

install: all dist/config.lua
	install -m 755 -d -- $(DESTDIR)$(PREFIX)/bin
	install -c -m 755 dist/x48ng $(DESTDIR)$(PREFIX)/bin/x48ng
	install -c -m 755 dist/x48ng-launcher $(DESTDIR)$(PREFIX)/bin/x48ng-launcher

	install -m 755 -d -- $(DESTDIR)$(PREFIX)/share/x48ng
	install -c -m 644 dist/hplogo.png $(DESTDIR)$(PREFIX)/share/x48ng/hplogo.png
	cp -R dist/ROMs/ $(DESTDIR)$(PREFIX)/share/x48ng/
	cp src/ui4x/*.css "$(DESTDIR)$(PREFIX)/share/x48ng/"

	install -m 755 -d -- $(DESTDIR)$(PREFIX)/libexec
	install -c -m 755 dist/x48ng-dump2rom $(DESTDIR)$(PREFIX)/libexec/x48ng-dump2rom
	install -c -m 755 dist/x48ng-checkrom $(DESTDIR)$(PREFIX)/libexec/x48ng-checkrom

	install -m 755 -d -- $(DESTDIR)$(MANDIR)/man1
	sed "s|@VERSION@|$(VERSION_MAJOR).$(VERSION_MINOR).$(PATCHLEVEL)|g" dist/x48ng.man.1 > $(DESTDIR)$(MANDIR)/man1/x48ng.1

	install -m 755 -d -- $(DESTDIR)$(DOCDIR)
	cp -R AUTHORS LICENSE README* doc* romdump/ $(DESTDIR)$(DOCDIR)
	install -c -m 644 dist/config.lua $(DESTDIR)$(DOCDIR)/config.lua

	install -m 755 -d -- $(DESTDIR)$(PREFIX)/share/applications
	sed "s|@PREFIX@|$(PREFIX)|g" dist/x48ng.desktop > $(DESTDIR)$(PREFIX)/share/applications/x48ng.desktop

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/x48ng
	rm -f $(DESTDIR)$(PREFIX)/bin/x48ng-launcher
	rm -f $(DESTDIR)$(PREFIX)/libexec/x48ng-dump2rom
	rm -f $(DESTDIR)$(PREFIX)/libexec/x48ng-checkrom
	rm -f $(DESTDIR)$(MANDIR)/man1/x48ng.1
	rm -f $(DESTDIR)$(PREFIX)/share/applications/x48ng.desktop

	rm -fr $(DESTDIR)$(PREFIX)/share/x48ng/
	rm -fr $(DESTDIR)$(DOCDIR)
