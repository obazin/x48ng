# Makefile

EXAMPLE_TARGET = ui4x
LIBUI4X_TARGET = libui4x.a

FULL_WARNINGS ?= no
WITH_GTK ?= yes
WITH_SDL ?= yes
WITH_SDL2 ?= no

VERSION_MAJOR ?= 1
VERSION_MINOR ?= 1
PATCHLEVEL ?= 1

LUA_VERSION ?= lua
PKG_CONFIG ?= pkg-config

MAKEFLAGS += -j$(NUM_CORES) -l$(NUM_CORES)

# pkg-config --cflags, with -I turned into -isystem so our warnings don't apply to third-party headers
pkg-cflags = $(patsubst -I%,-isystem %,$(shell "$(PKG_CONFIG)" --cflags $(1)))

# -Werror: clang merely warns about unknown options, which would make every probe succeed
cc-option = $(shell if $(CC) -Werror $(1) -c -x c /dev/null -o /dev/null > /dev/null 2>&1; \
		  then echo $(1); fi)

### lua
LUA_CFLAGS = $(call pkg-cflags,$(LUA_VERSION))
LUA_LDLIBS = $(shell "$(PKG_CONFIG)" --libs $(LUA_VERSION))

ifeq ($(FULL_WARNINGS), no)
EXTRA_WARNING_FLAGS := -Wno-unused-function \
	-Wno-redundant-decls \
	$(call cc-option,-Wno-maybe-uninitialized) \
	$(call cc-option,-Wno-discarded-qualifiers) \
	$(call cc-option,-Wno-uninitialized) \
	$(call cc-option,-Wno-ignored-qualifiers)
endif

ifeq ($(FULL_WARNINGS), yes)
EXTRA_WARNING_FLAGS := -Wunused-function \
	-Wredundant-decls \
	-fsanitize-trap \
	$(call cc-option,-Wunused-variable)
endif

### Text UI
NCURSES_CFLAGS = $(call pkg-cflags,ncursesw) -DNCURSES_WIDECHAR=1 -DHAS_NCURSES=1
NCURSES_LDLIBS = $(shell "$(PKG_CONFIG)" --libs ncursesw)
NCURSES_SRC = src/ncurses.c

### SDL UI
ifeq ($(WITH_SDL), yes)
	SDL_CFLAGS = $(call pkg-cflags,sdl3) -DHAS_SDL=1
	SDL_LDLIBS = $(shell "$(PKG_CONFIG)" --libs sdl3)
	SDL_SRC = src/sdl.c \
		src/sdl_bitmaps.c \
		src/sdl_fonts.c
endif

ifeq ($(WITH_SDL2), yes)
	SDL_CFLAGS = $(call pkg-cflags,sdl2) -DHAS_SDL=1 -DHAS_SDL2=1
	SDL_LDLIBS = $(shell "$(PKG_CONFIG)" --libs sdl2)
	SDL_SRC = src/sdl.c
endif

ifeq ($(WITH_GTK), yes)
	GTK_CFLAGS = $(call pkg-cflags,gtk4) -DHAS_GTK=1
	GTK_LDLIBS = $(shell "$(PKG_CONFIG)" --libs gtk4)
	GTK_SRC = src/gtk.c
endif

override CFLAGS := -std=c2x \
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
	$(EXTRA_WARNING_FLAGS) \
	$(SDL_CFLAGS) \
	$(GTK_CFLAGS) \
	$(NCURSES_CFLAGS) \
	$(LUA_CFLAGS) \
	-DVERSION_MAJOR=$(VERSION_MAJOR) \
	-DVERSION_MINOR=$(VERSION_MINOR) \
	-DPATCHLEVEL=$(PATCHLEVEL) \
	-DDATADIR=\"$(DATADIR)\" \
	$(CFLAGS)

override CPPFLAGS := -I./example/ -I./src/ -D_GNU_SOURCE=1 \
	-DVERSION_MAJOR=$(VERSION_MAJOR) \
	-DVERSION_MINOR=$(VERSION_MINOR) \
	-DPATCHLEVEL=$(PATCHLEVEL) \
	-DDATADIR=\"$(DATADIR)\" \
	$(CPPFLAGS)

LIBS = -lm \
	$(SDL_LDLIBS) \
	$(GTK_LDLIBS) \
	$(NCURSES_LDLIBS) \
	$(LUA_LDLIBS)


LIBUI4X_SRCS = $(NCURSES_SRC) \
	$(GTK_SRC) \
	$(SDL_SRC) \
	src/48gx.c \
	src/48sx.c \
	src/49g.c \
	src/40g.c \
	src/50g.c \
	src/api.c
LIBUI4X_OBJS = $(LIBUI4X_SRCS:.c=.o)

EXAMPLE_SRCS =	example/emulator_api.c \
	example/options.c \
	example/main.c
EXAMPLE_OBJS = $(EXAMPLE_SRCS:.c=.o)

.PHONY: all clean clean-all pretty-code install mrproper

all: $(EXAMPLE_TARGET) $(LIBUI4X_TARGET)

# Binaries
$(EXAMPLE_TARGET): $(EXAMPLE_OBJS) $(LIBUI4X_TARGET)
	$(CC) $^ -o $@ $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $(LIBS)

$(LIBUI4X_TARGET): $(LIBUI4X_OBJS)
	ar -r -c $@ $^

# Cleaning
clean:
	rm -f $(EXAMPLE_OBJS) $(LIBUI4X_OBJS)

distclean: clean
	rm -f $(EXAMPLE_TARGET) $(LIBUI4X_TARGET)

clean-all: distclean

mrproper: distclean

# Formatting
pretty-code:
	clang-format -i example/*.c example/*.h src/*.c src/*.h

# Dependencies
depend: $(LIBUI4X_SRCS) $(EXAMPLE_SRCS)
	$(CC) -MM $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $(LIBS) $^ > depend

include depend
