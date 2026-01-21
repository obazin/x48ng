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

    .ir_serial_device = NULL,
};

static void show_help( void )
{
    fprintf( stdout,
             "%s %i.%i.%i\n"
             "Emulator for HP 49G+ / 50G calculators' hardware\n"
             "Usage: %s [<options>]\n"
             "Valid options:\n"
             "-h --help                    print this message and exit\n"
             "-v --version                 print out version\n"
             "-r --reset                   reboot on startup instead of continuing from the saved state in the state file\n"
             "     --print-config       print configuration as config file\n"
             "  -t --terminal           activate pseudo terminal interface (default: "
             "true)\n"
             "  -s --serial             activate serial interface (default: false)\n"
             "     --debug              enable the debugger\n"
             "  -T --throttle           try to emulate real speed (default: false)\n"
             "     --inhibit-shutdown   __tentative fix for stuck-on-OFF bug__ (default: "
             "false)\n"
             "\n"
             "%s\n",
             ui4x_config.progname, VERSION_MAJOR, VERSION_MINOR, PATCHLEVEL, ui4x_config.progname, ui4x_get_help_string() );
}

static char* config_to_string( void )
{
    char* config;
    if ( -1 == asprintf( &config,
                         "--------------------------------------------------------------------------------\n"
                         "-- Configuration file for x50ng\n"
                         "-- This is a comment\n"
                         "%s\n"
                         "--- End of x48ng configuration -----------------------------------------------\n",
                         ui4x_get_config_as_string() ) )
        exit( EXIT_FAILURE );

    return config;
}

config_t* config_init( int argc, char* argv[] )
{
    /* temporary variables for filenames later treated with ui4x_make_filename_absolute() */
    char* bootloader_filename = NULL;
    char* firmware_filename = NULL;

    /***************************/
    /* 1. command-line options */
    /***************************/
    /* The cli options are temporaly stored in their clopt_* variables */
    int clopt_print_config_and_exit = false;

    /* specific */
    bool clopt_do_enable_debugger = false;
    bool clopt_do_start_debugger = false;
    bool clopt_do_flash = false;
    bool clopt_do_flash_full = false;

    const char* optstring = "hvVrtsT";
    struct option long_options[] = {
        {"help",             no_argument,       NULL,                         'h' },
        {"version",          no_argument,       NULL,                         'v' },
        {"verbose",          no_argument,       NULL,                         'V' },

        {"print-config",     no_argument,       &clopt_print_config_and_exit, true},

        {"reset",            no_argument,       NULL,                         'r' },

        {"terminal",         required_argument, NULL,                         't' },
        {"serial",           required_argument, NULL,                         's' },
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
                show_help();
                exit( EXIT_SUCCESS );
                break;
            case 'v':
                fprintf( stderr, "%i.%i.%i\n", VERSION_MAJOR, VERSION_MINOR, PATCHLEVEL );
                exit( EXIT_SUCCESS );
                break;

            case 'r':
                __config.reset = true;
                break;
            case 's':
                __config.enable_wire = true;
                break;
            case 't':
                __config.enable_ir = true;
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

    if ( clopt_print_config_and_exit ) {
        fprintf( stdout, "Calculated configuration:\n%s", config_to_string() );
        exit( EXIT_SUCCESS );
    }

    if ( ui4x_config.verbose )
        fprintf( stdout, "Calculated configuration:\n%s", config_to_string() );

    return &__config;
}
