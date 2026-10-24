# Development shell providing every dependency the Makefile looks up through pkg-config.
# Usage: nix-shell --run make
{ pkgs ? import <nixpkgs> { } }:

pkgs.mkShell {
  packages = with pkgs; [
    pkg-config
    glib
    gtk4
    lua5_4
    ncurses
    readline
    sdl3
    zlib
    unzip # provides funzip, used by `make get-roms`
  ];
}
