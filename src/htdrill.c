#include <stdio.h>
#include "goaccess/parser.h"
#include "goaccess/options.h"
#include "khash.h"
#include "goaccess/settings.h"

GConf conf = {
    .append_method = 1,
    .append_protocol = 1,
    .hl_header = 1,
    .num_tests = 10,
};

GLog *glog;

void
process_log (GLogItem * logitem)
{
    printf("%s\n", logitem->agent);
}


int main(int argc, char const *argv[])
{
    glog = init_log ();
    int ret;

    set_date_format_str("%d/%b/%Y");
    set_time_format_str("%H:%M:%S");
    set_log_format_str("%v %h %^[%d:%t %^] \"%r\" %s %b \"%R\" \"%u\" %T");
    set_spec_date_format();

    FILE *fp = fopen("data/access.log.1","r");
    add_dash_filename();
    glog->pipe = fp;
    
    if ((ret = parse_log (&glog, NULL, 0))) {
        goto clean;
    }

    clean:
    /* unable to process valid data */
    if (ret)
        output_logerrors (glog);
    return 0;
}
