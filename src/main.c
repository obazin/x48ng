#include <fcntl.h>
#include <langinfo.h>
#include <locale.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/time.h>

#include <glib.h>

#include "core/debugger.h"
#include "core/emulate.h"
#include "core/init.h"
#include "core/timers.h"

#include "ui4x/src/api.h"

#include "emulator_api.h"
#include "options.h"
#include "romio.h"

config_t x48ng_config;

void signal_handler( int sig )
{
    /* static int nb_refreshes_since_last_checking_events = 0; */

    switch ( sig ) {
        case SIGINT: /* Ctrl-C */
            enter_debugger |= USER_INTERRUPT;
            break;
        case SIGALRM:
            sigalarm_triggered = true;
            break;
        case SIGPIPE:
            ui4x_exit();
            stop_emulator();
            exit( EXIT_SUCCESS );
        default:
            break;
    }
}

int main( int argc, char** argv )
{
    setlocale( LC_ALL, "C" );

    char* progname = g_path_get_basename( argv[ 0 ] );
    char* progpath = g_path_get_dirname( argv[ 0 ] );
    char* default_datadir = g_build_filename( g_get_user_config_dir(), progname, NULL );

    /* The config is set from, by order of priority:
       - command line arguments
       - configuration file
       - hard-coded default values
     */
    /* First config pass, core x48ng parses argv */
    x48ng_config = *config_init( argc, argv );

    /********************/
    /* initialize stuff */
    /********************/

    ui4x_emulator_api_t emulator_api = {
        .press_key = press_key,
        .release_key = release_key,
        .is_key_pressed = is_key_pressed,
        .is_display_on = get_display_state,
        .get_annunciators = get_annunciators,
        .get_lcd_buffer = get_lcd_buffer,
        .get_contrast = get_contrast,
        .do_mount_sd = NULL,
        .do_unmount_sd = NULL,
        .is_sd_mounted = NULL,
        .get_sd_path = NULL,
        .do_reset = NULL,
        .do_stop = exit_emulator,
        .do_sleep = NULL,
        .do_wake = NULL,
        .do_debug = NULL,
    };
    ui4x_init( argc, argv, progname, progpath, default_datadir, &emulator_api );

    /* Emulator */
    init_emulator();

    ui4x_config.model = opt_gx ? MODEL_48GX : MODEL_48SX;

    /*****************************************/
    /* handlers for SIGALRM, SIGPIPE */
    /*****************************************/
    sigset_t set;
    struct sigaction sa;
    sigemptyset( &set );
    sigaddset( &set, SIGALRM );
    sa.sa_handler = signal_handler;
    sa.sa_mask = set;
#ifdef SA_RESTART
    sa.sa_flags = SA_RESTART;
#endif
    sigaction( SIGALRM, &sa, ( struct sigaction* )0 );

    sigemptyset( &set );
    sigaddset( &set, SIGINT );
    sa.sa_handler = signal_handler;
    sa.sa_mask = set;
#ifdef SA_RESTART
    sa.sa_flags = SA_RESTART;
#endif
    sigaction( SIGINT, &sa, ( struct sigaction* )0 );

    sigemptyset( &set );
    sigaddset( &set, SIGPIPE );
    sa.sa_handler = signal_handler;
    sa.sa_mask = set;
#ifdef SA_RESTART
    sa.sa_flags = SA_RESTART;
#endif
    sigaction( SIGPIPE, &sa, ( struct sigaction* )0 );

    /************************************/
    /* set the real time interval timer */
    /************************************/
    /*
      Every <interval>µs setitimer will trigger a SIGALRM
      which will set sigalarm_triggered to true
      In emulate() sigalarm_triggered triggers LCD refresh and UI event handling
     */
    struct itimerval it;
    it.it_interval.tv_sec = 0;
    it.it_interval.tv_usec = USEC_PER_FRAME;
    it.it_value.tv_sec = it.it_interval.tv_sec;
    it.it_value.tv_usec = it.it_interval.tv_usec;
    setitimer( ITIMER_REAL, &it, ( struct itimerval* )0 );

    /**********************************************************/
    /* Set stdin flags to not include O_NDELAY and O_NONBLOCK */
    /**********************************************************/
    /*
      I don't know what this is for?
     */
    long flags;
    flags = fcntl( STDIN_FILENO, F_GETFL, 0 );
    flags &= ~O_NDELAY;
    flags &= ~O_NONBLOCK;
    fcntl( STDIN_FILENO, F_SETFL, flags );

    /************************/
    /* Start emulation loop */
    /************************/
    struct timeval tv;
    struct timeval tv2;
    struct timezone tz;
    do {
        reset_timer( T1_TIMER );
        reset_timer( RUN_TIMER );
        reset_timer( IDLE_TIMER );

        set_accesstime();

        start_timer( T1_TIMER );
        start_timer( RUN_TIMER );

        sched_timer1 = t1_i_per_tick = saturn.t1_tick;
        sched_timer2 = t2_i_per_tick = saturn.t2_tick;

        set_t1 = saturn.timer1;

        do {
            step_instruction();

            if ( x48ng_config.enable_debugger && ( exec_flags & EXEC_BKPT ) && check_breakpoint( BP_EXEC, saturn.pc ) ) {
                enter_debugger |= BREAKPOINT_HIT;
                break;
            }

            for ( int i = 0; i < KEYS_BUFFER_SIZE; i++ )
                if ( saturn.keybuf[ i ] || x48ng_config.throttle ) {
                    /* Throttling speed if needed */
                    gettimeofday( &tv, &tz );
                    gettimeofday( &tv2, &tz );

                    while ( ( tv.tv_sec == tv2.tv_sec ) && ( ( tv.tv_usec - tv2.tv_usec ) < 2 ) )
                        gettimeofday( &tv, &tz );

                    break;
                }

            if ( schedule_event < 0 ) // "bug"
                schedule_event = 0;

            if ( schedule_event-- <= 0 )
                schedule();
        } while ( /* !please_exit && */ !enter_debugger );

        if ( enter_debugger )
            debug();
    } while ( true /* !please_exit */ );

    /* Never reached when not using please_exit */
    ui4x_exit();
    stop_emulator();

    return 0;
}
