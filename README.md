# x48ng -- HP 48 emulator

This is my fork of x48-0.6.4 where I deviate from the original code and do my own thing. (See ./README_0.6.4 for the original README)

**x48ng** is part of my little collection of HP calculators' emulators' that I took over maintenance :

- [x48ng](https://codeberg.org/gwh/x48ng) ← You are here
- [x50ng](https://codeberg.org/gwh/x50ng)
- [saturnng](https://codeberg.org/gwh/saturnng)
- [hpemung](https://codeberg.org/gwh/hpemung)

[ The reference public source repository is https://codeberg.org/gwh/x48ng ]

> **This repository is a modified fork of [x48ng](https://codeberg.org/gwh/x48ng)** that adds macOS support (native build, SDL rendering fixes, a self-contained `x48ng.app`) and various fixes; see the git history for the changes. Like the original, it is distributed under the GNU General Public License version 2 (`LICENSE`), and the vendored `src/ui4x` under the GNU General Public License version 3 (`src/ui4x/LICENSE`).

## Usage

`x48ng-launcher --help`

## Screenshots

### `--tui-small` version (ncurses, 4 pixels per character)

![screenshot of x48ng --tui-small running in a terminal](./tui-small-screenshot.png?raw=true "screenshot of x48ng --tui-small running in a terminal")

### `--tui-tiny --mono` version (ncurses, 8 pixels per character)

![screenshot of x48ng --tui-tiny --mono running in a terminal](./tui-tiny-screenshot.png?raw=true "screenshot of x48ng --tui-tiny --mono running in a terminal")

### `--tui` version (ncurses)

![screenshot of x48ng --tui running in a terminal](./tui-screenshot.png?raw=true "screenshot of x48ng --tui running in a terminal")

### `--sdl` version

![screenshot of x48ng --sdl](./sdl-screenshot.png?raw=true "screenshot of x48ng --sdl")

### manual setup

1. Create `$XDG_CONFIG_HOME/x48ng` (usually `~/.config/x48ng/`)
2. Copy `/usr/share/x48ng/ROMs/gxrom-r` (or any other rom) to `$XDG_CONFIG_HOME/x48ng/rom`
3. Run `x48ng --print-config > $XDG_CONFIG_HOME/x48ng/config.lua`
4. Run `x48ng`

### Ncurses UI (`--tui`)

I had to be a bit 'creative' mapping keys there:
```
┌─[ Help ]─────────────────────────────────────────────────────────┐
│Special keys:                                                     │
│ F1: Help, F7: Quit                                               │
│Calculator keys:                                                  │
│ all alpha-numerical keys                                         │
│ F2: Left-Shift, F3: Right-Shift, F4: Alpha, F5: On, F6: Enter    │
└──────────────────────────────────────────────────────────────────┘
```

There's also additional _hidden_ shortcuts that might work and/or come handy:

- *Left-Shift*: `[` or `PgUp`
- *Right-Shift*: `]` or `PgDn`
- *Alpha*: `;` or `Ins`
- *On*: `Esc` or `Home`
- *Enter*: `,`
- quit: `|` or `Shift + End` or `F10`

## What have I done:

0. renamed it to x48ng to avoid confusion
1. merged in a SDL1 version I found @ https://code.google.com/archive/p/x48-sdl/source/default/source
2. removed the code supporting Solaris, HP-UX, etc.
3. removed the autotools-based build system and wrote a simple Makefile instead
4. added a x48ng.desktop file and an icon
5. refactoring as a way to explore/understand the codebase
6. drop Xresources
7. link to lua to use it as a config file reader
8. ported the SDL1 GUI to SDL2 to SDL3

## Bugs to fix

See and report at https://codeberg.org/gwh/x48ng/issues or https://github.com/gwenhael-le-moine/x48ng/issues

## What more I would like to do:

### long(er) term

- GUI in gtk(4)?

## Compilation

The GTK and ncurses front-ends are always built. The SDL front-end is built with SDL3 by default: use `make WITH_SDL2=yes` to build it with SDL2 instead, or `make WITH_SDL=no` to leave it out.

The UI library, [ui4x](https://codeberg.org/gwh/ui4x), is included in `src/ui4x/` (vendored from ui4x 1.1.1 and modified since), so there is no submodule to fetch.

### Dependencies (see .github/workflows/c-cpp.yml for debian packages names)

- Lua
- readline
- GLib and GTK 4 (for the GTK front-end, the default)
- ncursesw (for the ncurses front-ends)
- SDL3, or SDL2 with `WITH_SDL2=yes` (for the SDL front-end)
- zlib

## Installation

0. Run `make get-roms` unless you have a good reason not to. (ROMs licenses are unclear so they're not bundled by default.)
1. Run `sudo make install PREFIX=/usr` (see the Makefile to see what variables your can override.)
2. Run `x48ng-launcher`. On first launch or if no ~/.config/x48ng/ exists it will be created and populated.
   By default it sets up a HP 48GX with ROM `R` with a 128KB card in port 1 and a 4MB card in port 2. You can change the ROM by setting the ROM environment variable (`ROM=sxrom-j x48ng-launcher` for example.)

The data directory (themes, ROMs) is compiled in from `PREFIX`; changing `PREFIX` triggers a full rebuild.

### macOS

The provided `shell.nix` brings every dependency, and `.envrc` loads it automatically with [direnv](https://direnv.net/) and [nix-direnv](https://github.com/nix-community/nix-direnv) (run `direnv allow` once).

- Build with `nix-shell --run make`, or just `make` once direnv is set up.
- `/usr` is read-only on macOS, so install somewhere else, for example `make install PREFIX=$HOME/.local` (with `~/.local/bin` in your `PATH`).
- `make install-app` puts an `x48ng.app` in `~/Applications` (override with `APPDIR`), to start the calculator from Spotlight, the Dock or Finder without a terminal. The app is self-contained: it carries the binary, every library it links to (copied out of the Nix store and re-signed), the themes and the ROMs (downloaded from hpcalc.org on the first build, like `make get-roms`), so it keeps working after the Nix store is garbage-collected and doesn't need `make install`. It shares `~/.config/x48ng` with the command-line version, and sets it up on first launch. `make app` only builds the bundle in `dist/`, and `make uninstall-app` removes it.
- The SDL front-end (`x48ng-launcher --sdl`, or `frontend = "sdl"` in `~/.config/x48ng/config.lua`) renders with Metal. It stays responsive while its window is hidden and handles resizing.

## Development

- `make FULL_WARNINGS=yes` to compile with all warnings
- `make clean` and `make mrproper` to clean between compilation runs
- `make pretty-code` to format the code using `clang-format` and the provided `.clang-format`

## Friends and inspiration

- https://github.com/gwenhael-le-moine/x48 (original x48 slightly touched but not too much)
- https://github.com/shagr4th/droid48 (x48 ported to Android)
- https://hp.giesselink.com/emu48.htm (The Emu48)
- https://github.com/dgis/emu48android (Emu48 ported to Android)
- https://github.com/gwenhael-le-moine/jsEmu48 (another HP 48 emulator)
- [x50ng](https://codeberg.org/gwh/x50ng)
- [saturnng](https://codeberg.org/gwh/saturnng)
- [hpemung](https://codeberg.org/gwh/hpemung)
