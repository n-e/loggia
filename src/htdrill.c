#include <stdio.h>
#include "goaccess/parser.h"
#include "goaccess/options.h"
#include "goaccess/browsers.h"
#include "khash.h"
#include "ksort.h"
#include "goaccess/settings.h"
#include "htdrill.h"
#include "spec_parser.h"
#include "table_print.h"

typedef struct {
    const char *name;
    int total;
} col_t;
#define col_gt(a, b) ((a).total > (b).total)
KSORT_INIT(col, col_t, col_gt)

// goaccess conf (for the parser)
GConf conf = {
    .append_method = 1,
    .append_protocol = 1,
    .hl_header = 1,
    .num_tests = 10,
};

MyConf myconf = {"", "", ""};

GLog *glog;

KHASH_MAP_INIT_STR(str_int,int)

typedef struct {
    const char *name;
    khash_t(str_int)* cols;
    int total;
} rowa_t;
typedef struct {
    khash_t(str_int)* cols;
    int total;
} row_t;
KHASH_MAP_INIT_STR(rowh, row_t)
KSORT_INIT(row, rowa_t, col_gt)

khash_t(rowh) *rows; // all the rows, each row contains the columns that have data in it
khash_t(str_int) *cols; // all the columns

int digits(long long x)
{
    x < 0 ? x = -x : 0;
    return x < 10 ? 1 :
        x < 100 ? 2 :
        x < 1000 ? 3 :
        x < 10000 ? 4 :
        x < 100000 ? 5 :
        x < 1000000 ? 6 :
        x < 10000000 ? 7 :
        x < 100000000 ? 8 :
        x < 1000000000 ? 9 :
        x < 10000000000 ? 10 : 0;
}

// Add calculated data to the logitem
static void fill_logitem(GLogItem *logitem) {
    // browser and browser_type
    char *agent = NULL;
    char browser_type[BROWSER_TYPE_LEN] = "";

    if (logitem->agent == NULL || *logitem->agent == '\0')
        return;

    agent = strdup (logitem->agent);
    logitem->browser = verify_browser (agent, browser_type);
    logitem->browser_type = strdup (browser_type);

    // url (we reuse userid for this)
    int len = strlen(logitem->req) + strlen(logitem->vhost);
    logitem->userid = malloc(len+1);
    strcpy(logitem->userid, logitem->vhost);
    strcat(logitem->userid,logitem->req);
}

// Callback that is called by the goaccess parser
// each time a log item (= valid line) is encountered
void
process_log (GLogItem * logitem)
{
    int empty;
    khint_t k;

    fill_logitem(logitem);

    char *row_key = create_key(logitem, myconf.rowspec);
    char *col_key = create_key(logitem, myconf.colspec);
    if (!match_filterspec(logitem, myconf.filterspec))
        return;

    // Filter
    // if(!strcmp())

    // Update the row and the columns for a row
    k = kh_put(rowh,rows,row_key,&empty);
    if (empty) {
        // kh_key(rows, k) = strdup(logitem->vhost);
        kh_value(rows, k).cols = kh_init(str_int);
        kh_value(rows, k).total = 1;
    }
    else
        kh_value(rows, k).total++;

    khash_t(str_int) *row_cols = kh_value(rows, k).cols;
    k = kh_put(str_int,row_cols,col_key,&empty);
    if (empty) {
        // kh_key(row_cols, k) = strdup(logitem->status);
        kh_value(row_cols, k) = 1;
    }
    else
        kh_value(row_cols, k)++;
    
    // Update the global columns object
    k = kh_put(str_int,cols,col_key,&empty);
    if (empty) {
        // kh_key(cols, k) = strdup(logitem->status);
        kh_value(cols, k) = 1;
    }
    else
        kh_value(cols, k)++;
}


int main(int argc, char *argv[])
{
    int i,j;
    glog = init_log ();
    int ret;

    read_option_args(argc, argv);
    set_spec_date_format();
    // printf("%s %s %s",myconf.rowspec,myconf.colspec,myconf.filterspec);

    rows = kh_init(rowh);
    cols = kh_init(str_int);


    FILE *fp = fopen("data/access.log.1","r");
    conf.read_stdin = 1;
    conf.filenames[conf.filenames_idx++] = "-";
    glog->pipe = fp;
    
    if ((ret = parse_log (&glog, NULL, 0))) {
        goto clean;
    }


    khint_t k;
    const char *row, *col;
    int total_cnt;
    row_t row_val;
    int col_val;

    // Sort columns
    int cols_array_len = kh_size(cols);
    col_t *cols_array = malloc(cols_array_len * sizeof(col_t));

    col_t *ptr = cols_array;
    kh_foreach(cols, col, total_cnt, {
        ptr->name = col;
        ptr->total = total_cnt;
        ptr++;
    })

    ks_mergesort(col,cols_array_len,cols_array,0);

    // Sort rows
    int rows_array_len = kh_size(rows);
    rowa_t *rows_array = malloc(rows_array_len * sizeof(rowa_t));

    rowa_t *rptr = rows_array;
    kh_foreach(rows, row, row_val, {
        rptr->name = row;
        rptr->cols = row_val.cols;
        rptr->total = row_val.total;
        rptr++;
    })

    ks_mergesort(row,rows_array_len,rows_array,0);

    if (rows_array_len == 0) {
        printf("(0 results)\n");
        exit(0);
    }

    // Print headers

    TableSpec table_spec;
    table_spec.w0 = 0;
    table_spec.w = 0;
    table_spec.minw = 0;
    table_spec.ncols = kh_size(cols);

    // Set w0
    kh_foreach(rows, row, row_val, {
        int len = strlen(row);
        table_spec.w0 = table_spec.w0 < len ? len : table_spec.w0;
    })

    // Set w to the length of the largest column title
    for(int i = 0; i < cols_array_len; i++) {
        int len = strlen(cols_array[i].name);
        table_spec.w = table_spec.w < len ? len : table_spec.w;
    }

    // set minw to the length of the longest value in the table
    int max_val = 0;
    for(int i = 0; i < rows_array_len; i++) {
        kh_foreach(rows_array[i].cols, col, col_val, {
            if (col_val > max_val)
                max_val = col_val;
        })
    }
    table_spec.minw = digits(max_val);

    if (table_spec.w < table_spec.minw)
        table_spec.w = table_spec.minw;
    
    int original_ncols = table_spec.ncols;
    crop_to_termwidth(&table_spec);

    print_row_header(table_spec,"");
    int max = original_ncols == table_spec.ncols ? table_spec.ncols : table_spec.ncols-1;
    for (i=0; i < max; i++) {
        print_row_value(table_spec,cols_array[i].name);
    }
    if (original_ncols > table_spec.ncols) {
        print_row_value(table_spec,"...");
    }
    printf("\n");

    print_ruler(table_spec);

    // Print table contents
    for (j=0; j < rows_array_len; j++) {
        rowa_t row_j = rows_array[j];

        print_row_header(table_spec,row_j.name);
        for (i=0; i < max; i++) {
            k = kh_get(str_int,row_j.cols,cols_array[i].name);
            if (k != kh_end(row_j.cols))
                print_row_value_int(table_spec,kh_value(row_j.cols,k));
            else
                print_row_value(table_spec,"");
        }
        // printf("|\033[1m%*d\033[0m",h2_width, row_j.total);
        printf("\n");
    }

    // print_ruler(h_width,h2_width,kh_size(cols));

    // print totals
    /*printf("%*s",h_width,"");
    for (i=0; i < cols_array_len; i++) {
        printf("|\033[1m%*d\033[0m",h2_width, cols_array[i].total);
    }
    printf("\n");*/

    clean:
    /* unable to process valid data */
    if (ret)
        output_logerrors (glog);
    return 0;
}
