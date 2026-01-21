#ifndef _OPTIONS_H
#  define _OPTIONS_H 1

#  include <stdbool.h>

#  ifndef VERSION_MAJOR
#    define VERSION_MAJOR 0
#  endif
#  ifndef VERSION_MINOR
#    define VERSION_MINOR 0
#  endif
#  ifndef PATCHLEVEL
#    define PATCHLEVEL 0
#  endif

typedef struct {
    /* options below are specific to x48ng */
    bool enable_wire;
    bool enable_ir;
    bool enable_debugger;
    bool throttle;
    bool reset;
    bool inhibit_shutdown;

    char* ir_serial_device;
} config_t;
extern config_t config;

/* #  define MAX_LENGTH_FILENAME 2048 */
/* extern char normalized_config_path[ MAX_LENGTH_FILENAME ]; */
/* extern char normalized_config_file[ MAX_LENGTH_FILENAME ]; */
/* extern char normalized_rom_path[ MAX_LENGTH_FILENAME ]; */
/* extern char normalized_ram_path[ MAX_LENGTH_FILENAME ]; */
/* extern char normalized_state_path[ MAX_LENGTH_FILENAME ]; */
/* extern char normalized_port1_path[ MAX_LENGTH_FILENAME ]; */
/* extern char normalized_port2_path[ MAX_LENGTH_FILENAME ]; */

/*************/
/* functions */
/*************/
extern config_t* config_init( int argc, char* argv[] );

#endif /* !_OPTIONS_H */
