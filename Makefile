# Makefile to build x48ng without autotools
#
# The cc-option function and the C{,PP}FLAGS logic were copied from the
# fsverity-utils project.
# https://git.kernel.org/pub/scm/fs/fsverity/fsverity-utils.git/
# The governing license can be found in the LICENSE file or at
# https://opensource.org/license/MIT.

TARGET = x48ng

VERSION_MAJOR = 1
VERSION_MINOR = 3
PATCHLEVEL = 1

PREFIX ?= /usr
BINDIR ?= $(PREFIX)/bin
DATADIR ?= $(PREFIX)/share/$(TARGET)
DOCDIR ?= $(PREFIX)/share/doc/$(TARGET)
MENUDIR ?= $(PREFIX)/share/applications
MANDIR ?= $(PREFIX)/share/man/man1
APPDIR ?= $(HOME)/Applications

PKG_CONFIG ?= pkg-config

LUA_VERSION ?= lua				# or luajit for example
WITH_SDL ?= yes
WITH_SDL2 ?= no

# optimization level for cc
OPTIM ?= 2

##############################
# No serviceable parts under #
##############################

MAKEFLAGS +=-j$(NUM_CORES) -l$(NUM_CORES)

# pkg-config --cflags, with -I turned into -isystem so our warnings don't apply to third-party headers
pkg-cflags = $(patsubst -I%,-isystem %,$(shell "$(PKG_CONFIG)" --cflags $(1)))

# -Werror: clang merely warns about unknown options, which would make every probe succeed
cc-option = $(shell if $(CC) -Werror $(1) -c -x c /dev/null -o /dev/null > /dev/null 2>&1; \
		  then echo $(1); fi)

GLIB2_CFLAGS = $(call pkg-cflags,glib-2.0)
GLIB2_LDLIBS = $(shell "$(PKG_CONFIG)" --libs glib-2.0)

GTK_CFLAGS = $(call pkg-cflags,gtk4) -DHAS_GTK=1
GTK_LDLIBS = $(shell "$(PKG_CONFIG)" --libs gtk4)

LUA_CFLAGS = $(call pkg-cflags,$(LUA_VERSION))
LUA_LDLIBS = $(shell "$(PKG_CONFIG)" --libs $(LUA_VERSION))

NCURSES_CFLAGS = $(call pkg-cflags,ncursesw readline tinfo) -DNCURSES_WIDECHAR=1 -DHAS_NCURSES=1
NCURSES_LDLIBS = $(shell "$(PKG_CONFIG)" --libs ncursesw readline tinfo)

### SDL UI
ifeq ($(WITH_SDL), yes)
	SDL_CFLAGS = $(call pkg-cflags,sdl3) -DHAS_SDL=1
	SDL_LDLIBS = $(shell "$(PKG_CONFIG)" --libs sdl3)
endif

ifeq ($(WITH_SDL2), yes)
	SDL_CFLAGS = $(call pkg-cflags,sdl2) -DHAS_SDL=1 -DHAS_SDL2=1
	SDL_LDLIBS = $(shell "$(PKG_CONFIG)" --libs sdl2)
endif

ifeq ($(DEBUG), yes)
	DEBUG_CFLAGS = \
		-g \
		-pg

	DEBUG_LDFLAGS = -g -pg
endif

# ifeq ($(FULL_WARNINGS), no)
# EXTRA_WARNING_CFLAGS := -Wno-unused-function \
#	-Wno-redundant-decls \
#	$(call cc-option,-Wno-maybe-uninitialized) \
#	$(call cc-option,-Wno-discarded-qualifiers) \
#	$(call cc-option,-Wno-uninitialized) \
#	$(call cc-option,-Wno-ignored-qualifiers)
# else
# EXTRA_WARNING_CFLAGS := -Wunused-function \
#	-Wredundant-decls \
#	-fsanitize=thread \
#	$(call cc-option,-Wunused-variable)
# endif

override CFLAGS := \
	$(CFLAGS) \
	-std=gnu2x \
	-O$(OPTIM) \
	-W \
	-Wall \
	-Wextra \
	-pedantic \
	-Wformat=2 \
	-Wshadow \
	-Wno-unused-parameter \
	-Wno-pointer-arith \
	-Wno-overflow  \
	-Wwrite-strings \
	-Wstrict-prototypes \
	-Wold-style-definition \
	-Wnested-externs \
	-Wmissing-include-dirs \
	-Wdouble-promotion \
	-Wno-sign-conversion \
	-Wno-format-nonliteral \
	$(call cc-option,-Wjump-misses-init) \
	$(call cc-option,-Wlogical-op) \
	-Isrc/ \
	-Isrc/core/ \
	$(LUA_CFLAGS) \
	$(GLIB2_CFLAGS) \
	$(GTK_CFLAGS) \
	$(NCURSES_CFLAGS) \
	$(SDL_CFLAGS) \
	$(DEBUG_CFLAGS) \
	-D_GNU_SOURCE=1 \
	-DVERSION_MAJOR=$(VERSION_MAJOR) \
	-DVERSION_MINOR=$(VERSION_MINOR) \
	-DPATCHLEVEL=$(PATCHLEVEL) \
	-DDATADIR=\"$(DATADIR)\"

override LDFLAGS := \
	$(LDFLAGS) \
	$(DEBUG_LDFLAGS)

# Apple's linker has no --as-needed behaviour to turn off
ifneq ($(shell uname -s), Darwin)
override LDFLAGS += -Wl,--no-as-needed
endif

SRCS = src/options.c \
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
OBJS = $(SRCS:.c=.o)

all: do-it-all

ifeq (.depend,$(wildcard .depend))
include .depend
do-it-all: $(TARGET) dot-desktop manfile
else
do-it-all: depend-and-build
endif


# src/ui4x/libui4x ############################
UI4X_DIR = src/ui4x
DATADIR_STAMP = .datadir
UI4X_LDLIBS = -L$(UI4X_DIR) -lui4x

libui4x: $(DATADIR_STAMP)
	$(MAKE) -C $(UI4X_DIR) libui4x.a \
		WITH_SDL=$(WITH_SDL) \
		WITH_SDL2=$(WITH_SDL2) \
		LUA_VERSION=$(LUA_VERSION) \
		VERSION_MAJOR=$(VERSION_MAJOR) \
		VERSION_MINOR=$(VERSION_MINOR) \
		PATCHLEVEL=$(PATCHLEVEL) \
		DATADIR="$(DATADIR)"

clean-libui4x:
	$(MAKE) -C $(UI4X_DIR) clean

distclean-libui4x:
	$(MAKE) -C $(UI4X_DIR) distclean

override LDLIBS := $(LDLIBS) \
	-lz \
	-lm \
	$(LUA_LDLIBS) \
	$(GLIB2_LDLIBS) \
	$(GTK_LDLIBS) \
	$(NCURSES_LDLIBS) \
	$(SDL_LDLIBS) \
	$(UI4X_LDLIBS)


##########
# Depend #
##########
depend:
	$(CC) -MM $(CFLAGS) $(SRCS) > .depend

depend-and-build: depend
	$(MAKE) -C . all


##############
# for clangd #
##############
compile_commands.json: distclean
	bear -- make


####################
# auto-format code #
####################
pretty-code:
	clang-format -i src/*.c src/*.h src/core/*.c src/core/*.h


###############
# Compilation #
###############
# DATADIR is compiled in, so record it and rebuild everything when it changes (e.g. a new PREFIX)
$(DATADIR_STAMP): FORCE
	@if [ "$$(cat $@ 2> /dev/null)" != "$(DATADIR)" ]; then \
		echo "$(DATADIR)" > $@; \
		$(MAKE) -C $(UI4X_DIR) clean; \
	fi

FORCE:

$(OBJS): $(DATADIR_STAMP)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(TARGET): $(OBJS) libui4x
	$(CC) $(CFLAGS) $(LDFLAGS) -o dist/$(TARGET) $(OBJS) $(LDLIBS)

dist/x48ng-dump2rom: src/legacy_tools/dump2rom.o
dist/x48ng-checkrom: src/legacy_tools/checkrom.o src/romio.o


##################
# Populate dist/ #
##################
dot-desktop: src/$(TARGET).desktop.in
	sed 's|@PREFIX@|$(PREFIX)|g' $< > dist/$(TARGET).desktop

manfile: src/$(TARGET).man.1.in
	cp $< dist/$(TARGET).man.1


#####################
# macOS app bundle  #
#####################
# Self-contained: the binary, the libraries it links to (copied out of e.g. the Nix store), the themes and the ROMs (downloaded once).
# The launcher script runs from Contents/MacOS, where links make it and x48ng find ROMs/ and themes/.
# Everything gets an ad-hoc signature because macOS refuses to launch unsigned code.
APP = dist/$(TARGET).app
APP_MINIMUM_SYSTEM_VERSION = 13.0
APP_ICONSET = dist/$(TARGET).iconset

app: all pull-firmware
ifneq ($(shell uname -s), Darwin)
	@echo "make app builds a macOS application bundle and only works on macOS" >&2; exit 1
endif
	rm -rf $(APP) $(APP_ICONSET)
	mkdir -p $(APP)/Contents/MacOS $(APP)/Contents/Resources $(APP_ICONSET)
	sed -e 's|@VERSION@|$(VERSION_MAJOR).$(VERSION_MINOR).$(PATCHLEVEL)|g' \
		-e 's|@MINIMUM_SYSTEM_VERSION@|$(APP_MINIMUM_SYSTEM_VERSION)|g' \
		src/macos/Info.plist.in > $(APP)/Contents/Info.plist
	$(CC) -O2 -mmacosx-version-min=$(APP_MINIMUM_SYSTEM_VERSION) -o $(APP)/Contents/MacOS/$(TARGET)-app src/macos/launcher.c
	cp dist/$(TARGET) dist/$(TARGET)-launcher $(APP)/Contents/MacOS/
	sh src/macos/bundle-libraries.sh $(APP) $(APP)/Contents/MacOS/$(TARGET)
	cp -R dist/ROMs src/ui4x/themes src/ui4x/*.css $(APP)/Contents/Resources/
	ln -s ../Resources/ROMs $(APP)/Contents/MacOS/ROMs
	ln -s ../Resources/themes $(APP)/Contents/MacOS/themes
	for size in 16 32 128 256 512; do \
		sips -s format png -z $$size $$size dist/hplogo.png --out $(APP_ICONSET)/icon_$${size}x$${size}.png > /dev/null; \
		sips -s format png -z $$((size * 2)) $$((size * 2)) dist/hplogo.png --out $(APP_ICONSET)/icon_$${size}x$${size}@2x.png > /dev/null; \
	done
	iconutil -c icns $(APP_ICONSET) -o $(APP)/Contents/Resources/$(TARGET).icns
	rm -rf $(APP_ICONSET)
	codesign --force -s - $(APP)/Contents/MacOS/$(TARGET)-launcher
	codesign --force -s - $(APP)

install-app: app
	mkdir -p "$(DESTDIR)$(APPDIR)/"
	rm -rf "$(DESTDIR)$(APPDIR)/$(TARGET).app"
	cp -R $(APP) "$(DESTDIR)$(APPDIR)/"

uninstall-app:
	rm -rf "$(DESTDIR)$(APPDIR)/$(TARGET).app"

.PHONY: app install-app uninstall-app


###########################################
# Populate dist/firmware/ from hpcalc.org #
###########################################
pull-firmware:
	$(MAKE) -C dist/ROMs
get-roms: pull-firmware

distclean-firmware:
	$(MAKE) -C dist/ROMs mrproper


############
# Cleaning #
############
clean: clean-libui4x
	rm -f core *~ .depend $(DATADIR_STAMP)
	rm -f $(OBJS)

mrproper: clean distclean-libui4x
	rm -f compile_commands.json
	rm -f dist/$(TARGET) dist/$(TARGET).desktop dist/$(TARGET).man.1
	rm -rf dist/$(TARGET).app dist/$(TARGET).iconset
	find . -name \*.o -exec  rm {} \;

distclean: mrproper distclean-firmware


################
# Installation #
################
install: all
	mkdir -p "$(DESTDIR)$(BINDIR)/"
	install -m 755 dist/$(TARGET) "$(DESTDIR)$(BINDIR)/$(TARGET)"
	install -m 755 dist/$(TARGET)-launcher "$(DESTDIR)$(BINDIR)/$(TARGET)-launcher"
	mkdir -p "$(DESTDIR)$(MENUDIR)/"
	install -m 644 dist/$(TARGET).desktop "$(DESTDIR)$(MENUDIR)/$(TARGET).desktop"
	mkdir -p "$(DESTDIR)$(MANDIR)/"
	install -m 644 dist/$(TARGET).man.1 "$(DESTDIR)$(MANDIR)/$(TARGET).1"
	mkdir -p "$(DESTDIR)$(DATADIR)/"
	cp -r dist/ROMs/ "$(DESTDIR)$(DATADIR)/ROMs"
	cp -r dist/hplogo.png "$(DESTDIR)$(DATADIR)/"
	cp -r src/ui4x/themes src/ui4x/*.css "$(DESTDIR)$(DATADIR)/"
	mkdir -p "$(DESTDIR)$(DOCDIR)/"
	cp -r ./doc* ./romdump/ ./README.* ./LICENSE ./AUTHORS ./*.png "$(DESTDIR)$(DOCDIR)/"

uninstall:
	rm -f "$(DESTDIR)$(BINDIR)/$(TARGET)"
	rm -f "$(DESTDIR)$(MENUDIR)/$(TARGET).desktop"
	rm -f "$(DESTDIR)$(MANDIR)/$(TARGET).1"
	rm -fr "$(DESTDIR)$(DATADIR)/"
	rm -fr "$(DESTDIR)$(DOCDIR)/"
