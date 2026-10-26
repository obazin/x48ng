/* Executable of x48ng.app: macOS only launches app bundles whose executable is a signed binary, not a script.
   It runs the x48ng-launcher script next to it, which sets up ~/.config/x48ng and starts the bundled x48ng. */
#include <libgen.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <mach-o/dyld.h>

int main( int argc, char** argv )
{
    char executable_path[ PATH_MAX ];
    uint32_t size = sizeof( executable_path );
    char resolved_path[ PATH_MAX ];
    char launcher[ PATH_MAX ];

    if ( _NSGetExecutablePath( executable_path, &size ) != 0 || realpath( executable_path, resolved_path ) == NULL ) {
        fprintf( stderr, "can't find where x48ng.app is\n" );
        return EXIT_FAILURE;
    }
    snprintf( launcher, sizeof( launcher ), "%s/x48ng-launcher", dirname( resolved_path ) );

    argv[ 0 ] = launcher;
    execv( launcher, argv );

    perror( launcher );
    return EXIT_FAILURE;
}
