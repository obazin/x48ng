#!/bin/sh
# Copies every non-system library an executable needs (recursively) into the app's Contents/Frameworks,
# and rewrites the references to @rpath so the app no longer depends on where they were built (e.g. the Nix store).
#
# usage: bundle-libraries.sh <app bundle> <executable inside it>

set -eu

bundle=$1
executable=$2
frameworks="$bundle/Contents/Frameworks"

mkdir -p "$frameworks"

is_system_library() {
    case "$1" in
        /usr/lib/* | /System/*) return 0 ;;
        *) return 1 ;;
    esac
}

# prints the libraries a Mach-O file links to, without its own install name
dependencies_of() {
    own_name=$(otool -D "$1" | tail -n +2)
    otool -L "$1" | tail -n +2 | awk '{ print $1 }' | while read -r reference; do
        [ "$reference" = "$own_name" ] || echo "$reference"
    done
}

# removes the run paths that point outside the bundle
delete_external_run_paths() {
    otool -l "$1" | awk '/cmd LC_RPATH/ { getline; getline; print $2 }' | while read -r run_path; do
        case "$run_path" in
            @*) ;;
            *) install_name_tool -delete_rpath "$run_path" "$1" ;;
        esac
    done
}

pending="$executable"
while [ -n "$pending" ]; do
    file=${pending%%
*}
    if [ "$file" = "$pending" ]; then pending=""; else pending=${pending#*
}; fi

    for reference in $(dependencies_of "$file"); do
        is_system_library "$reference" && continue
        case "$reference" in
            @*) continue ;; # already relative, i.e. bundled by a previous step
        esac

        # several names (libz.dylib, libz.1.dylib) can point to the same file: bundle it once, under its real name
        source=$(realpath "$reference")
        name=$(basename "$source")
        if [ ! -e "$frameworks/$name" ]; then
            cp "$source" "$frameworks/$name"
            chmod u+w "$frameworks/$name"
            install_name_tool -id "@rpath/$name" "$frameworks/$name" 2> /dev/null
            pending="$pending${pending:+
}$frameworks/$name"
        fi
        install_name_tool -change "$reference" "@rpath/$name" "$file" 2> /dev/null
    done

    delete_external_run_paths "$file"
done

install_name_tool -add_rpath "@executable_path/../Frameworks" "$executable"

# install_name_tool invalidates the signatures, and Apple Silicon refuses to load unsigned code
for library in "$frameworks"/*.dylib; do
    codesign --force -s - "$library" 2> /dev/null
done
codesign --force -s - "$executable" 2> /dev/null
