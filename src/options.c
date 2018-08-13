#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "goaccess/settings.h"
#include "htdrill.h"

static char short_options[] = "hf:Vt:d:";

struct option long_opts[] = {
  {"help"                 , no_argument       , 0 , 'h' } ,
  {"version"              , no_argument       , 0 , 'V' } ,
  {"date-format"          , required_argument , 0 ,  0  } ,
  {"log-format"           , required_argument , 0 ,  0  } ,
  {"time-format"          , required_argument , 0 ,  0  } ,
  {0, 0, 0, 0}
};

/* Command line help. */
void
cmd_help (void)
{
    printf (
    "Usage: "
    "htdrill [ options ... ] [ rows ] [ cols ] [ filter ]\n\n"
    "\n");

    printf (
    /* Log & Date Format Options */
    "Log & Date Format Options\n\n"
    "  -f, --log-format=<logformat>    - Specify log format. Inner quotes need to be\n"
    "                                    escaped, or use single quotes.\n"
    "  -d, --date-format=<dateformat>      - Specify log date format. e.g., %%d/%%b/%%Y\n"
    "  -t, --time-format=<timeformat>      - Specify log time format. e.g., %%H:%%M:%%S\n\n"

    /* Other Options */
    "Other Options\n\n"
    "  -h, --help                      - This help.\n"
    "  -V, --version                   - Display version information and exit.\n"
    "\n"

    " `man goaccess`.\n\n"
    ": http://goaccess.io\n"
    "GoAccess Copyright (C) 2009-2017 by Gerardo Orellana"
    "\n\n"
    );
    exit(1);
}

static void
parse_long_opt (const char *name, const char *oarg)
{
    if (!strcmp ("no-global-config", name))
        return;

    /* LOG & DATE FORMAT OPTIONS
    * ========================= */
    /* log format */
    if (!strcmp ("log-format", name))
        set_log_format_str (oarg);

    /* time format */
    if (!strcmp ("time-format", name))
        set_time_format_str (oarg);

    /* date format */
    if (!strcmp ("date-format", name))
        set_date_format_str (oarg);
}

/* Read the user's supplied command line options. */
void
read_option_args (int argc, char *const *argv)
{
    int o, idx = 0;

    while ((o = getopt_long (argc, argv, short_options, long_opts, &idx)) >= 0) {
        if (-1 == o)
            break;

        switch (o) {
            case 'f':
                set_log_format_str (optarg);
                break;
            case 't':
                set_time_format_str (optarg);
                break;
            case 'd':
                set_date_format_str (optarg);
                break;
            case 'h':
                cmd_help ();
                break;
            case 'V':
                // display_version ();
                exit (0);
                break;
            case 0:
                parse_long_opt (long_opts[idx].name, optarg);
                break;
            case '?':
                exit (1);
            default:
                exit (1);
        }
    }

    idx = optind;
    if (idx < argc)
        myconf.rowspec = argv[idx++];
    if (idx < argc)
        myconf.colspec = argv[idx++];
    if (idx < argc)
        myconf.filterspec = argv[idx++];
}