#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include "ui4x/src/api.h"

#include "options.h"

static config_t __config = {
    .enable_wire = false,
    .enable_ir = false,
    .enable_debugger = false,
    .throttle = false,
    .reset = false,

    .print_config_and_exit = false,

    .ir_serial_device = ( char* )"/dev/ttyS0",
};

/* runs before ui4x_init(), so ui4x_config.progname isn't set yet */
static void show_help( const char* progname )
{
    fprintf( stdout,
             "%s %i.%i.%i\n"
             "Emulator for HP 48SX / 48GX calculators' hardware\n"
             "Usage: %s [<options>]\n"
             "Valid options:\n"
             "-h --help                    print this message and exit\n"
             "-v --version                 print out version\n"
             "-r --reset                   reboot on startup instead of continuing from the saved state in the state file\n"
             "   --print-config            print configuration as config file\n"
             "-t --terminal                activate the wire interface on a pseudo terminal (default: false)\n"
             "   --serial                  activate the IR interface on the serial line (default: false)\n"
             "   --serial-line=<path>      serial line used by the IR interface (default: %s)\n"
             "   --debug                   enable the debugger\n"
             "-T --throttle                try to emulate real speed (default: false)\n"
             "   --inhibit-shutdown        __tentative fix for stuck-on-OFF bug__ (default: false)\n"
             "\n"
             "%s\n",
             progname, VERSION_MAJOR, VERSION_MINOR, PATCHLEVEL, progname, __config.ir_serial_device, ui4x_get_help_string() );
}

char* config_to_string( void )
{
    char* config;
    if ( -1 == asprintf( &config,
                         "--------------------------------------------------------------------------------\n"
                         "-- Configuration file for x48ng\n"
                         "-- This is a comment\n"
                         "%s\n"
                         "--- End of x48ng configuration -----------------------------------------------\n",
                         ui4x_get_config_as_string() ) )
        exit( EXIT_FAILURE );

    return config;
}

config_t* config_init( int argc, char* argv[] )
{
    /***************************/
    /* 1. command-line options */
    /***************************/
    /* The cli options are temporaly stored in their clopt_* variables */
    int clopt_print_config_and_exit = false;

    /* no -s: ui4x parses the same command line and uses it for --style */
    const char* optstring = "hvVrtT";
    struct option long_options[] = {
        {"help",             no_argument,       NULL,                         'h' },
        {"version",          no_argument,       NULL,                         'v' },
        {"verbose",          no_argument,       NULL,                         'V' },

        {"print-config",     no_argument,       &clopt_print_config_and_exit, true},

        {"reset",            no_argument,       NULL,                         'r' },

        {"terminal",         no_argument,       NULL,                         't' },
        {"serial",           no_argument,       NULL,                         12  },
        {"serial-line",      required_argument, NULL,                         13  },
        {"debug",            no_argument,       NULL,                         11  },

        {"throttle",         no_argument,       NULL,                         'T' },
        {"inhibit-shutdown", no_argument,       NULL,                         90  },

        {0,                  0,                 0,                            0   }
    };
    int option_index;
    int arg = '?';
    opterr = false; /* handle unknown arg silently  */
    optind = 0;     /* reset arg position to allow multiple parsing passes */
    while ( arg != EOF ) {
        arg = getopt_long( argc, argv, optstring, long_options, &option_index );
        switch ( arg ) {
            case 'h':
                show_help( g_path_get_basename( argv[ 0 ] ) );
                exit( EXIT_SUCCESS );
                break;
            case 'v':
                fprintf( stderr, "%i.%i.%i\n", VERSION_MAJOR, VERSION_MINOR, PATCHLEVEL );
                exit( EXIT_SUCCESS );
                break;

            case 'r':
                __config.reset = true;
                break;
            case 't':
                __config.enable_wire = true;
                break;
            case 12:
                __config.enable_ir = true;
                break;
            case 13:
                __config.ir_serial_device = strdup( optarg );
                break;
            case 'T':
                __config.throttle = true;
                break;
            case 11:
                __config.enable_debugger = true;
                break;
            case 90:
                __config.inhibit_shutdown = true;
                break;

            default:
                break;
        }
    }

    __config.print_config_and_exit = clopt_print_config_and_exit;

    return &__config;
}
