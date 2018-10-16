/*  Copyright (c) 2018 Nicolas Even <nicolas@even.li>
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this
    list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
    ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "goaccess/settings.h"
#include "loggia.h"
#include "colors.h"

static char short_options[] = "hf:Vt:d:l:";

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
cmd_help (const char *err_msg)
{
    if (err_msg)
        printf (
            TBOLD TRED "%s\n\n" TRESET,
            err_msg
        );

    printf (
    "Usage: "
    "loggia -l log-format -f log-file [ rows ] [ cols ] [ filter ]\n\n"
    "\n");

    printf (
    "  -f <logfile>\n"
    /* Log & Date Format Options */
    "  -l, --log-format=<logformat>\n"
    "          Common formats: COMMON, VCOMMON, COMBINED, VCOMBINED\n"
    "                          W3C, SQUID, CLOUDFRONT, AWSS3\n"
    "  -d, --date-format=<dateformat>\n"
    "  -t, --time-format=<timeformat>\n\n"

    /* Other Options */
    "  -h, --help                      This help.\n"
    "  -V, --version                   Display version information and exit.\n"
    "\n"
    "loggia <https://github.com/n-e/loggia> "
    "Copyright (C) 2018 Nicolas Even"
    "\n"
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
                conf.filenames[conf.filenames_idx++] = strdup(optarg);
                break;
            case 'l':
                set_log_format_str (optarg);
                break;
            case 't':
                set_time_format_str (optarg);
                break;
            case 'd':
                set_date_format_str (optarg);
                break;
            case 'h':
                cmd_help (NULL);
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