#include <assert.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/times.h>
#include <unistd.h>

#include <glib.h>

#include "api.h"
#include "gtk.h"
#include "inner.h"
#include "ncurses.h"
#ifdef HAS_SDL
#  include "sdl.h"
#endif

#ifndef LUA_OK
#  define LUA_OK 0
#endif

#ifdef DATADIR
#  define GLOBAL_DATADIR DATADIR
#else
#  define GLOBAL_DATADIR ui4x_config.progpath
#endif

typedef struct ui4x_temp_config_t {
    int model;
    int shiftless;
    int black_lcd;
    int newrpl_keyboard;

    int frontend;
    int mono;
    int gray;

    int chromeless;
    int fullscreen;

    int tiny;
    int small;

    double zoom;
    int netbook;
    int netbook_pivot_line;

    char* name;

    char* datadir;
    char* style_filename;

    char* sd_dir;
} ui4x_temp_config_t;

static ui4x_temp_config_t __arg_config = {
    .model = -1,
    .shiftless = -1,
    .black_lcd = -1,
    .newrpl_keyboard = -1,

    .frontend = -1,
    .mono = -1,
    .gray = -1,

    .chromeless = -1,
    .fullscreen = -1,

    .tiny = -1,
    .small = -1,

    .zoom = -1.0,
    .netbook = -1,
    .netbook_pivot_line = -1,

    .name = NULL,

    .datadir = NULL,
    .style_filename = NULL,

    .sd_dir = NULL,
};

static ui4x_temp_config_t __file_config = {
    .model = -1,
    .shiftless = -1,
    .black_lcd = -1,
    .newrpl_keyboard = -1,

    .frontend = -1,
    .mono = -1,
    .gray = -1,

    .chromeless = -1,
    .fullscreen = -1,

    .tiny = -1,
    .small = -1,

    .zoom = -1.0,
    .netbook = -1,
    .netbook_pivot_line = -1,

    .name = NULL,

    .datadir = NULL,
    .style_filename = NULL,

    .sd_dir = NULL,
};

char* ui_annunciators[ NB_ANNUNCIATORS ] = { "⮢", "⮣", "α", "🪫", "⌛", "⇄" };

ui4x_emulator_api_t ui4x_emulator_api;

ui4x_config_t ui4x_config = {
    .model = MODEL_50G,
    .shiftless = false,
    .black_lcd = false,
    .newrpl_keyboard = false,

    .frontend = FRONTEND_GTK,
    .mono = false,
    .gray = false,

    .chromeless = false,
    .fullscreen = false,

    .tiny = false,
    .small = false,

    .verbose = false,

    .zoom = 1.0,
    .netbook = false,
    .netbook_pivot_line = 3,

    .haz_config_file = false,

    .name = NULL,
    .progname = NULL,
    .progpath = NULL,
    .wire_name = NULL,
    .ir_name = NULL,

    .datadir = NULL,
    .style_filename = NULL,

    .sd_dir = NULL,
};

/*************/
/* functions */
/*************/
static void newrplify_buttons_50g( void )
{
    if ( ui4x_config.model != MODEL_50G )
        return;

    // modify keys' labeling for newRPL
    for ( int i = HP4950_KEY_A; i <= HP4950_KEY_F; i++ ) {
        buttons_50g[ i ].left = "";

        buttons_50g[ i ].left_sdl = "";
    }

    for ( int i = HP4950_KEY_G; i <= HP4950_KEY_I; i++ ) {
        buttons_50g[ i ].label = "";
        buttons_50g[ i ].left = "";
        buttons_50g[ i ].right = NULL;

        buttons_50g[ i ].label_sdl = "";
        buttons_50g[ i ].left_sdl = "";
        buttons_50g[ i ].right_sdl = NULL;
    }

    for ( int i = HP4950_KEY_J; i <= HP4950_KEY_K; i++ ) {
        buttons_50g[ i ].label = "";
        buttons_50g[ i ].left = "";
        buttons_50g[ i ].right = NULL;

        buttons_50g[ i ].label_sdl = "";
        buttons_50g[ i ].left_sdl = "";
        buttons_50g[ i ].right_sdl = NULL;
    }

    buttons_50g[ HP4950_KEY_UP ].left = "UPDIR";
    buttons_50g[ HP4950_KEY_UP ].left_sdl = "UPDIR";

    buttons_50g[ HP4950_KEY_LEFT ].left = "BEG";
    buttons_50g[ HP4950_KEY_LEFT ].left_sdl = "BEG";
    buttons_50g[ HP4950_KEY_LEFT ].right = "COPY";
    buttons_50g[ HP4950_KEY_LEFT ].right_sdl = "COPY";

    buttons_50g[ HP4950_KEY_DOWN ].left = "CUT";
    buttons_50g[ HP4950_KEY_DOWN ].left_sdl = "CUT";

    buttons_50g[ HP4950_KEY_RIGHT ].left = "END";
    buttons_50g[ HP4950_KEY_RIGHT ].left_sdl = "END";
    buttons_50g[ HP4950_KEY_RIGHT ].right = "PASTE";
    buttons_50g[ HP4950_KEY_RIGHT ].right_sdl = "PASTE";

    buttons_50g[ HP4950_KEY_M ].label = "STO⏵";
    buttons_50g[ HP4950_KEY_M ].left = "RCL";
    buttons_50g[ HP4950_KEY_M ].right = "PREV.M";

    buttons_50g[ HP4950_KEY_M ].label_sdl = "STO";
    buttons_50g[ HP4950_KEY_M ].left_sdl = "RCL";
    buttons_50g[ HP4950_KEY_M ].right_sdl = "PREV.M";

    for ( int i = HP4950_KEY_N; i <= HP4950_KEY_O; i++ ) {
        buttons_50g[ i ].left = "";
        buttons_50g[ i ].left_sdl = "";
        buttons_50g[ i ].right = NULL;
        buttons_50g[ i ].right_sdl = NULL;
    }

    buttons_50g[ HP4950_KEY_P ].label = "MENU";
    buttons_50g[ HP4950_KEY_P ].label_sdl = "MENU";

    buttons_50g[ HP4950_KEY_BACKSPACE ].left = "";
    buttons_50g[ HP4950_KEY_BACKSPACE ].left_sdl = "";

    for ( int i = HP4950_KEY_S; i <= HP4950_KEY_U; i++ ) {
        buttons_50g[ i ].right = NULL;
        buttons_50g[ i ].right_sdl = NULL;
    }

    for ( int i = HP4950_KEY_ALPHA; i <= HP4950_KEY_9; i++ ) {
        buttons_50g[ i ].left = "";
        buttons_50g[ i ].left_sdl = "";
    }

    buttons_50g[ HP4950_KEY_8 ].right = NULL;

    for ( int i = HP4950_KEY_4; i <= HP4950_KEY_6; i++ ) {
        buttons_50g[ i ].left = "";
        buttons_50g[ i ].left_sdl = "";
        buttons_50g[ i ].right = NULL;
        buttons_50g[ i ].right_sdl = NULL;
    }

    buttons_50g[ HP4950_KEY_2 ].left = "";
    buttons_50g[ HP4950_KEY_2 ].left_sdl = "";

    buttons_50g[ HP4950_KEY_ON ].left = "";
    buttons_50g[ HP4950_KEY_ON ].left_sdl = "";
    buttons_50g[ HP4950_KEY_ON ].below = NULL;

    buttons_50g[ HP4950_KEY_ENTER ].left_sdl = "";
    buttons_50g[ HP4950_KEY_ENTER ].left = "";
}

static ui4x_temp_config_t* ui_config_read_argv( int argc, char** argv )
{
    const char* optstring = "d:n:s:Vz:";
    struct option long_options[] = {
        {"verbose",            no_argument,       NULL, 'V' },

        {"datadir",            required_argument, NULL, 'd' },

        {"sd-dir",             required_argument, NULL, 800 },

        {"name",               required_argument, NULL, 'n' },

        {"gtk",                no_argument,       NULL, 900 },
#ifdef HAS_SDL
        {"sdl",                no_argument,       NULL, 904 },
#endif
        {"tui",                no_argument,       NULL, 901 },
        {"tui-small",          no_argument,       NULL, 902 },
        {"tui-tiny",           no_argument,       NULL, 903 },

        {"netbook",            no_argument,       NULL, 10  },
        {"netbook-pivot-line", required_argument, NULL, 1001},
        {"newrpl-keyboard",    no_argument,       NULL, 20  },
        {"style",              required_argument, NULL, 's' },
        {"zoom",               required_argument, NULL, 'z' },
        {"chromeless",         no_argument,       NULL, 30  },
        {"fullscreen",         no_argument,       NULL, 40  },
        {"mono",               no_argument,       NULL, 50  },
        {"gray",               no_argument,       NULL, 60  },
        {"black-lcd",          no_argument,       NULL, 70  },
        {"shiftless",          no_argument,       NULL, 80  },

        {"48sx",               no_argument,       NULL, 485 },
        {"48gx",               no_argument,       NULL, 486 },
        {"40g",                no_argument,       NULL, 406 },
        {"49g",                no_argument,       NULL, 496 },
        {"50g",                no_argument,       NULL, 506 },

        {0,                    0,                 0,    0   }
    };

    int option_index;
    int arg = '?';
    opterr = false; /* handle unknown arg silently  */
    optind = 0;     /* reset arg position to allow multiple parsing passes */
    while ( arg != EOF ) {
        arg = getopt_long( argc, argv, optstring, long_options, &option_index );

        switch ( arg ) {
            case 10:
                __arg_config.netbook = true;
                break;
            case 20:
                __arg_config.newrpl_keyboard = true;
                break;
            case 30:
                __arg_config.chromeless = true;
                break;
            case 40:
                __arg_config.fullscreen = true;
                break;
            case 50:
                __arg_config.mono = true;
                break;
            case 60:
                __arg_config.gray = true;
                break;
            case 70:
                __arg_config.black_lcd = true;
                break;
            case 80:
                __arg_config.shiftless = true;
                break;
            case 900:
                __arg_config.frontend = FRONTEND_GTK;
                break;
            case 901:
                __arg_config.frontend = FRONTEND_NCURSES;
                __arg_config.small = false;
                __arg_config.tiny = false;
                break;
            case 902:
                __arg_config.frontend = FRONTEND_NCURSES;
                __arg_config.small = true;
                __arg_config.tiny = false;
                break;
            case 903:
                __arg_config.frontend = FRONTEND_NCURSES;
                __arg_config.small = false;
                __arg_config.tiny = true;
                break;
            case 904:
                __arg_config.frontend = FRONTEND_SDL;
                break;
            case 1001:
                __arg_config.netbook_pivot_line = atoi( optarg );
                break;
            case 'd':
                __arg_config.datadir = strdup( optarg );
                break;
            case 'n':
                __arg_config.name = strdup( optarg );
                break;
            case 's':
                __arg_config.style_filename = strdup( optarg );
                break;
            case 'z':
                __arg_config.zoom = atof( optarg );
                break;
            case 'V':
                /* --verbose is effective immediatly */
                ui4x_config.verbose = true;
                break;

            case 800:
                __arg_config.sd_dir = strdup( optarg );
                break;

            case 485:
                __arg_config.model = MODEL_48SX;
                break;
            case 486:
                __arg_config.model = MODEL_48GX;
                break;
            case 406:
                __arg_config.model = MODEL_40G;
                break;
            case 496:
                __arg_config.model = MODEL_49G;
                break;
            case 506:
                __arg_config.model = MODEL_50G;
                break;

            default:
                break;
        }
    }

    return &__arg_config;
}

static ui4x_temp_config_t* ui_config_read_file( char* filename )
{
    /**********************/
    /* 3. read config.lua */
    /**********************/
    if ( ui4x_config.verbose )
        fprintf( stderr, "Loading configuration file %s\n", filename );

    /* populates config_lua_values[]  */
    lua_State* config_lua_values = ui4x_load_lua_file( filename );
    ui4x_config.haz_config_file = config_lua_values != NULL;
    if ( ui4x_config.haz_config_file ) {
        /* Now pull config options' values from config_lua_values by name
           options are set directly into __file_config or temp filename variables
         */

        lua_getglobal( config_lua_values, "frontend" );
        const char* frontend = luaL_optstring( config_lua_values, -1, "gtk" );
        if ( frontend != NULL ) {
            if ( strcmp( frontend, "gtk" ) == 0 || strcmp( frontend, "gui" ) == 0 ) {
                __file_config.frontend = FRONTEND_GTK;
                __file_config.small = false;
                __file_config.tiny = false;
            }
            if ( strcmp( frontend, "sdl" ) == 0 ) {
                __file_config.frontend = FRONTEND_SDL;
                __file_config.small = false;
                __file_config.tiny = false;
            }
            if ( strcmp( frontend, "tui" ) == 0 ) {
                __file_config.frontend = FRONTEND_NCURSES;
                __file_config.small = false;
                __file_config.tiny = false;
            }
            if ( strcmp( frontend, "tui-small" ) == 0 ) {
                __file_config.frontend = FRONTEND_NCURSES;
                __file_config.small = true;
                __file_config.tiny = false;
            }
            if ( strcmp( frontend, "tui-tiny" ) == 0 ) {
                __file_config.frontend = FRONTEND_NCURSES;
                __file_config.small = false;
                __file_config.tiny = true;
            }
        }

        lua_getglobal( config_lua_values, "model" );
        const char* svalue_model = luaL_optstring( config_lua_values, -1, "48gx" );
        if ( svalue_model != NULL ) {
            if ( strcmp( svalue_model, "50g" ) == 0 )
                __file_config.model = MODEL_50G;
            if ( strcmp( svalue_model, "49g" ) == 0 )
                __file_config.model = MODEL_49G;
            if ( strcmp( svalue_model, "40g" ) == 0 )
                __file_config.model = MODEL_40G;
            if ( strcmp( svalue_model, "48gx" ) == 0 )
                __file_config.model = MODEL_48GX;
            if ( strcmp( svalue_model, "48sx" ) == 0 )
                __file_config.model = MODEL_48SX;
        }

        lua_getglobal( config_lua_values, "style" );
        const char* lua_style_filename = luaL_optstring( config_lua_values, -1, NULL );
        if ( lua_style_filename != NULL )
            __file_config.style_filename = strdup( lua_style_filename );

        lua_getglobal( config_lua_values, "name" );
        const char* lua_name = luaL_optstring( config_lua_values, -1, NULL );
        if ( lua_name != NULL )
            __file_config.name = strdup( lua_name );

        lua_getglobal( config_lua_values, "shiftless" );
        __file_config.shiftless = lua_toboolean( config_lua_values, -1 );

        lua_getglobal( config_lua_values, "zoom" );
        __file_config.zoom = luaL_optnumber( config_lua_values, -1, __file_config.zoom );

        lua_getglobal( config_lua_values, "netbook" );
        __file_config.netbook = lua_toboolean( config_lua_values, -1 );

        lua_getglobal( config_lua_values, "netbook_pivot_line" );
        __file_config.netbook_pivot_line = luaL_optinteger( config_lua_values, -1, __file_config.netbook_pivot_line );

        lua_getglobal( config_lua_values, "newrpl_keyboard" );
        __file_config.newrpl_keyboard = lua_toboolean( config_lua_values, -1 );

        lua_getglobal( config_lua_values, "chromeless" );
        __file_config.chromeless = lua_toboolean( config_lua_values, -1 );

        lua_getglobal( config_lua_values, "fullscreen" );
        __file_config.fullscreen = lua_toboolean( config_lua_values, -1 );

        lua_getglobal( config_lua_values, "mono" );
        __file_config.mono = lua_toboolean( config_lua_values, -1 );

        lua_getglobal( config_lua_values, "gray" );
        __file_config.gray = lua_toboolean( config_lua_values, -1 );

        lua_getglobal( config_lua_values, "black_lcd" );
        __file_config.black_lcd = lua_toboolean( config_lua_values, -1 );

        lua_getglobal( config_lua_values, "sd_dir" );
        const char* lua_sd_dir = luaL_optstring( config_lua_values, -1, NULL );
        if ( lua_sd_dir != NULL && 0 < strlen( lua_sd_dir ) )
            __file_config.sd_dir = strdup( lua_sd_dir );
    }

    return &__file_config;
}

static char* bool_to_string( bool val ) { return val ? "true" : "false"; }

static char* frontend_to_string( ui4x_frontend_t f )
{
    return f == FRONTEND_GTK
               ? "gtk"
               : ( f == FRONTEND_SDL ? "sdl" : ( ui4x_config.tiny ? "tui-tiny" : ( ui4x_config.small ? "tui-small" : "tui" ) ) );
}

static char* model_to_string( ui4x_model_t m )
{
    switch ( m ) {
        case MODEL_48GX:
            return "48gx";
        case MODEL_48SX:
            return "48sx";
        case MODEL_40G:
            return "40g";
        case MODEL_49G:
            return "49g";
        case MODEL_50G:
            return "50g";
        default:
            return "??";
    }
}

/********************/
/* Public functions */
/********************/
lua_State* ui4x_load_lua_file( const char* filename )
{
    assert( filename != NULL );

    /*---------------------------------------------------
    ; Create the Lua state, which includes NO predefined
    ; functions or values.  This is literally an empty
    ; slate.
    ;----------------------------------------------------*/
    lua_State* lua_values = luaL_newstate();
    if ( lua_values == NULL ) {
        fprintf( stderr, "cannot create Lua state\n" );
        return NULL;
    }

    /*-----------------------------------------------------
    ; For the truly paranoid about sandboxing, enable the
    ; following code, which removes the string library,
    ; which some people find problematic to leave un-sand-
    ; boxed. But in my opinion, if you are worried about
    ; such attacks in a configuration file, you have bigger
    ; security issues to worry about than this.
    ;------------------------------------------------------*/
    lua_pushliteral( lua_values, "x" );
    lua_pushnil( lua_values );
    lua_setmetatable( lua_values, -1 );
    lua_pop( lua_values, 1 );

    /*-----------------------------------------------------
    ; Lua 5.2+ can restrict scripts to being text only,
    ; to avoid a potential problem with loading pre-compiled
    ; Lua scripts that may have malformed Lua VM code that
    ; could possibly lead to an exploit, but again, if you
    ; have to worry about that, you have bigger security
    ; issues to worry about.  But in any case, here I'm
    ; restricting the file to "text" only.
    ;------------------------------------------------------*/
    int rc = luaL_loadfile( lua_values, filename );
    if ( rc != LUA_OK ) {
        fprintf( stderr, "Lua error: (%d) %s\n", rc, lua_tostring( lua_values, -1 ) );
        return NULL;
    }

    rc = lua_pcall( lua_values, 0, 0, 0 );
    if ( rc != LUA_OK ) {
        fprintf( stderr, "Lua error: (%d) %s\n", rc, lua_tostring( lua_values, -1 ) );
        return NULL;
    }

    return lua_values;
}

char* ui4x_get_help_string( void )
{
    return "-V --verbose                 print out more information\n"
           "-d --datadir[=absolute path] alternate datadir (default: $XDG_CONFIG_HOME/%s/)\n"
           "   --sd-dir[=absolute path]  directory to mount as SD card (default: none)\n"
           "\n"
           "-n --name[=text]             customize the title of the window\n"
           "   --48gx         emulate a HP 48GX\n"
           "   --48sx         emulate a HP 48SX\n"
           "   --40g          emulate a HP 40G\n"
           "   --49g          emulate a HP 49G\n"
           "   --50g          emulate a HP 50G\n"
           "   --gtk                     use gtk GUI (Graphical UI) (default: true)\n"
           "   --sdl                     use sdl GUI (Graphical UI) (default: false)\n"
           "   --tui                     use ncurses TUI (Terminal text UI) (default: false)\n"
           "   --tui-small               use ncurses TUI (4 pixels per character) (Terminal text UI) (default: false)\n"
           "   --tui-tiny                use ncurses TUI (8 pixels per character) (Terminal text UI) (default: false)\n"
           "   --chromeless              borderless LCD (ncurses or sdl only) (default: false)\n"
           "   --fullscreen              fullscreen (sdl only) (default: false)\n"
           "   --netbook                 horizontal window (gtk only) (default: false)\n"
           "   --netbook-pivot-line      at which line is the keyboard split in netbook mode (GUI only) (default: 3)\n"
           "   --newrpl-keyboard         label keyboard for newRPL (GUI only)\n"
           "-s --style[=filename]        css filename in <datadir> (gtk only)\n"
           "-z --zoom[=X]                scale LCD by X (gtk or sdl only)\n"
           "   --chromeless              only show display (default: false)\n"
           "   --fullscreen              make the UI fullscreen (default: false)\n"
           "   --mono                    make the UI monochrome (default: false)\n"
           "   --gray                    make the UI grayscale (default: false)\n"
           "   --black-lcd\n"
           "   --shiftless               don't map the shift keys to let them free for numbers (default: false)\n";
}

char* ui4x_get_config_as_string( void )
{
    char* config;
    if ( -1 == asprintf( &config,
                         "--- UI configuration -----------------------------------------------------------\n"
                         "-- This is a comment\n"
                         "name = \"%s\"  -- this customize the title of the window\n"
                         "\n"
                         "frontend = \"%s\" -- possible values are: \"gtk\" (default), \"sdl\", \"tui\", \"tui-small\", \"tui-tiny\"\n"
                         "model = \"%s\"\n"
                         "\n"
                         "style = \"%s\" -- CSS file (relative to this file) (gtk only)\n"
                         "zoom = %f -- (gtk or sdl only)\n"
                         "netbook = %s -- (gtk only)\n"
                         "netbook_pivot_line = %i -- marks the transition between higher and lower keyboard (gtk only)\n"
                         "newrpl_keyboard = %s -- makes the keyboard labels more suited to newRPL use (gtk or sdl only)\n"
                         "shiftless = %s\n"
                         "chromeless = %s\n"
                         "fullscreen = %s\n"
                         "mono = %s\n"
                         "gray = %s\n"
                         "black_lcd = %s\n"
                         "--- End of UI configuration --------------------------------------------------\n",
                         ui4x_config.name, frontend_to_string( ui4x_config.frontend ), model_to_string( ui4x_config.model ),
                         ui4x_config.style_filename, ui4x_config.zoom, bool_to_string( ui4x_config.netbook ),
                         ui4x_config.netbook_pivot_line, bool_to_string( ui4x_config.newrpl_keyboard ),
                         bool_to_string( ui4x_config.shiftless ), bool_to_string( ui4x_config.chromeless ),
                         bool_to_string( ui4x_config.fullscreen ), bool_to_string( ui4x_config.mono ), bool_to_string( ui4x_config.gray ),
                         bool_to_string( ui4x_config.black_lcd ) ) )
        exit( EXIT_FAILURE );

    return config;
}

static char* ui4x_get_temp_config_as_string( ui4x_temp_config_t conf )
{
    char* config;
    if ( -1 == asprintf( &config,
                         "--- UI configuration -----------------------------------------------------------\n"
                         "-- This is a comment\n"
                         "name = \"%s\"  -- this customize the title of the window\n"
                         "\n"
                         "frontend = \"%s\" -- possible values are: \"gtk\" (default), \"sdl\", \"tui\", \"tui-small\", \"tui-tiny\"\n"
                         "model = \"%s\"\n"
                         "\n"
                         "style = \"%s\" -- CSS file (relative to this file) (gtk only)\n"
                         "zoom = %f -- (gtk or sdl only)\n"
                         "netbook = %s -- (gtk only)\n"
                         "netbook_pivot_line = %i -- marks the transition between higher and lower keyboard (gtk only)\n"
                         "newrpl_keyboard = %s -- makes the keyboard labels more suited to newRPL use (gtk or sdl only)\n"
                         "shiftless = %s\n"
                         "chromeless = %s\n"
                         "fullscreen = %s\n"
                         "mono = %s\n"
                         "gray = %s\n"
                         "black_lcd = %s\n"
                         "--- End of UI configuration --------------------------------------------------\n",
                         conf.name, frontend_to_string( conf.frontend ), model_to_string( conf.model ), conf.style_filename, conf.zoom,
                         bool_to_string( conf.netbook ), conf.netbook_pivot_line, bool_to_string( conf.newrpl_keyboard ),
                         bool_to_string( conf.shiftless ), bool_to_string( conf.chromeless ), bool_to_string( conf.fullscreen ),
                         bool_to_string( conf.mono ), bool_to_string( conf.gray ), bool_to_string( conf.black_lcd ) ) )
        exit( EXIT_FAILURE );

    return config;
}

int ui4x_get_lcd_height( void ) { return LCD_HEIGHT; }

int ui4x_get_nb_keys( void ) { return NB_KEYS; }

int ui4x_get_n_levels_of_gray( void ) { return N_LEVELS_OF_GRAY; }

void ui4x_handle_pending_inputs( void )
{
    switch ( ui4x_config.frontend ) {
        case FRONTEND_NCURSES:
            ncurses_handle_pending_inputs();
            break;
#ifdef HAS_SDL
        case FRONTEND_SDL:
            sdl_ui4x_handle_pending_inputs();
            break;
#endif
        case FRONTEND_GTK:
        default:
            gtk_ui4x_handle_pending_inputs();
            break;
    }
}

void ui4x_refresh_output( void )
{
    switch ( ui4x_config.frontend ) {
        case FRONTEND_NCURSES:
            ncurses_refresh_lcd();
            break;
#ifdef HAS_SDL
        case FRONTEND_SDL:
            sdl_ui_refresh_lcd();
            break;
#endif
        case FRONTEND_GTK:
        default:
            gtk_ui_refresh_lcd();
            break;
    }
}

char* ui4x_make_filename_absolute_for_loading( char* filename )
{
    /* is filename readable as-is? */
    char* full_path = g_build_filename( filename, NULL );
    if ( ui4x_config.verbose )
        fprintf( stderr, "  - 1. trying '%s'\n", full_path );
    if ( g_file_test( full_path, G_FILE_TEST_EXISTS ) )
        return full_path;

    /* does filename exist in configured datadir? */
    full_path = g_build_filename( ui4x_config.datadir, filename, NULL );
    if ( ui4x_config.verbose )
        fprintf( stderr, "  - 2. trying '%s'\n", full_path );
    if ( g_file_test( full_path, G_FILE_TEST_EXISTS ) )
        return full_path;

    /* does filename exist in global datadir? */
    full_path = g_build_filename( GLOBAL_DATADIR, filename, NULL );
    if ( ui4x_config.verbose )
        fprintf( stderr, "  - 3. trying '%s'\n", full_path );
    if ( g_file_test( full_path, G_FILE_TEST_EXISTS ) )
        return full_path;

    /* out of options, hope filename exists relatively to binary */
    full_path = g_build_filename( ui4x_config.progpath, filename, NULL );
    if ( ui4x_config.verbose )
        fprintf( stderr, "  - 4. praying for '%s'\n", full_path );

    return full_path;
}

char* ui4x_make_filename_absolute_for_saving( char* filename )
{
    /* is filename writable as-is? */
    char* full_path = g_build_filename( filename, NULL );
    if ( ui4x_config.verbose )
        fprintf( stderr, "  - 1. trying '%s'\n", full_path );
    if ( g_file_test( full_path, G_FILE_TEST_EXISTS ) )
        return full_path;

    /* can we save filename in configured datadir? */
    if ( ui4x_config.verbose )
        fprintf( stderr, "  - 2. trying '%s'\n", ui4x_config.datadir );
    if ( g_file_test( ui4x_config.datadir, G_FILE_TEST_IS_DIR ) ) {
        full_path = g_build_filename( ui4x_config.datadir, filename, NULL );
        return full_path;
    }

    /* out of options, hope we can save relatively to binary */
    full_path = g_build_filename( ui4x_config.progpath, filename, NULL );
    if ( ui4x_config.verbose )
        fprintf( stderr, "  - 4. praying for '%s'\n", full_path );

    return full_path;
}

char* ui4x_get_config_file_path( void ) { return g_build_filename( ui4x_config.datadir, "config.lua", NULL ); }

void ui4x_init( int argc, char** argv, char* progname, char* progpath, char* datadir, ui4x_emulator_api_t* emulator_api )
{
    /* Store emulator's API for ui4xusage */
    ui4x_emulator_api = *emulator_api;

    /* Set config values given */
    ui4x_config.progname = strdup( progname );
    ui4x_config.progpath = strdup( progpath );
    ui4x_config.datadir = strdup( datadir );

    /* Read commandline parameters */
    ui4x_temp_config_t config_from_argv = *ui_config_read_argv( argc, argv ); /* this can overrride ui4x_config.datadir */
    if ( ui4x_config.verbose )
        fprintf( stderr, "** CONFIG from argv :\n%s\n**\n", ui4x_get_temp_config_as_string( config_from_argv ) );
    if ( config_from_argv.datadir != NULL )
        ui4x_config.datadir = strdup( config_from_argv.datadir );

    /* Read configuration file parameters */
    ui4x_temp_config_t config_from_file = *ui_config_read_file( ui4x_get_config_file_path() );

    /* Set config first from the values read from the configuration file */
    if ( ui4x_config.haz_config_file ) {
        if ( ui4x_config.verbose )
            fprintf( stderr, "** CONFIG from %s :\n%s\n**\n", ui4x_get_config_file_path(),
                     ui4x_get_temp_config_as_string( config_from_file ) );

        if ( config_from_file.model != -1 )
            ui4x_config.model = config_from_file.model;
        if ( config_from_file.shiftless != -1 )
            ui4x_config.shiftless = config_from_file.shiftless;
        if ( config_from_file.black_lcd != -1 )
            ui4x_config.black_lcd = config_from_file.black_lcd;
        if ( config_from_file.newrpl_keyboard != -1 )
            ui4x_config.newrpl_keyboard = config_from_file.newrpl_keyboard;
        if ( config_from_file.frontend != -1 )
            ui4x_config.frontend = config_from_file.frontend;
        if ( config_from_file.mono != -1 )
            ui4x_config.mono = config_from_file.mono;
        if ( config_from_file.gray != -1 )
            ui4x_config.gray = config_from_file.gray;
        if ( config_from_file.chromeless != -1 )
            ui4x_config.chromeless = config_from_file.chromeless;
        if ( config_from_file.fullscreen != -1 )
            ui4x_config.fullscreen = config_from_file.fullscreen;
        if ( config_from_file.tiny != -1 )
            ui4x_config.tiny = config_from_file.tiny;
        if ( config_from_file.small != -1 )
            ui4x_config.small = config_from_file.small;
        if ( config_from_file.zoom != -1.0 )
            ui4x_config.zoom = config_from_file.zoom;
        if ( config_from_file.netbook != -1 )
            ui4x_config.netbook = config_from_file.netbook;
        if ( config_from_file.netbook_pivot_line != -1 )
            ui4x_config.netbook_pivot_line = config_from_file.netbook_pivot_line;
        if ( config_from_file.name != NULL )
            ui4x_config.name = strdup( config_from_file.name );
        if ( config_from_file.style_filename != NULL )
            ui4x_config.style_filename = strdup( config_from_file.style_filename );
        if ( config_from_file.sd_dir != NULL )
            ui4x_config.sd_dir = strdup( config_from_file.sd_dir );
    }

    /* Then possibly override them with the values from commandline parameters */
    if ( config_from_argv.model != -1 )
        ui4x_config.model = config_from_argv.model;
    if ( config_from_argv.shiftless != -1 )
        ui4x_config.shiftless = config_from_argv.shiftless;
    if ( config_from_argv.black_lcd != -1 )
        ui4x_config.black_lcd = config_from_argv.black_lcd;
    if ( config_from_argv.newrpl_keyboard != -1 )
        ui4x_config.newrpl_keyboard = config_from_argv.newrpl_keyboard;
    if ( config_from_argv.frontend != -1 )
        ui4x_config.frontend = config_from_argv.frontend;
    if ( config_from_argv.mono != -1 )
        ui4x_config.mono = config_from_argv.mono;
    if ( config_from_argv.gray != -1 )
        ui4x_config.gray = config_from_argv.gray;
    if ( config_from_argv.chromeless != -1 )
        ui4x_config.chromeless = config_from_argv.chromeless;
    if ( config_from_argv.fullscreen != -1 )
        ui4x_config.fullscreen = config_from_argv.fullscreen;
    if ( config_from_argv.tiny != -1 )
        ui4x_config.tiny = config_from_argv.tiny;
    if ( config_from_argv.small != -1 )
        ui4x_config.small = config_from_argv.small;
    if ( config_from_argv.zoom != -1.0 )
        ui4x_config.zoom = config_from_argv.zoom;
    if ( config_from_argv.netbook != -1 )
        ui4x_config.netbook = config_from_argv.netbook;
    if ( config_from_argv.netbook_pivot_line != -1 )
        ui4x_config.netbook_pivot_line = config_from_argv.netbook_pivot_line;
    if ( config_from_argv.name != NULL )
        ui4x_config.name = strdup( config_from_argv.name );
    if ( config_from_argv.datadir != NULL )
        ui4x_config.datadir = strdup( config_from_argv.datadir );
    if ( config_from_argv.style_filename != NULL )
        ui4x_config.style_filename = strdup( config_from_argv.style_filename );
    if ( config_from_argv.sd_dir != NULL )
        ui4x_config.sd_dir = strdup( config_from_argv.sd_dir );

    if ( ui4x_config.verbose )
        fprintf( stderr, "** GLOBAL_DATADIR = %s\n", GLOBAL_DATADIR );

    /* Continue with initialization of UI4x */
    if ( ui4x_config.newrpl_keyboard )
        newrplify_buttons_50g();
}

void ui4x_start( void )
{
    switch ( ui4x_config.frontend ) {
        case FRONTEND_NCURSES:
            ncurses_init();
            break;
#ifdef HAS_SDL
        case FRONTEND_SDL:
            sdl_ui4x_init();
            break;
#endif
        case FRONTEND_GTK:
        default:
            gtk_ui4x_init();
            break;
    }
}

void ui4x_exit( void )
{
    switch ( ui4x_config.frontend ) {
        case FRONTEND_NCURSES:
            ncurses_exit();
            break;
#ifdef HAS_SDL
        case FRONTEND_SDL:
            sdl_exit();
            break;
#endif
        case FRONTEND_GTK:
        default:
            gtk_exit();
            break;
    }
}
