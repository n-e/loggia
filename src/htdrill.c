#include <stdio.h>
#include <sys/ioctl.h>
#include <errno.h>
#include "goaccess/parser.h"
#include "goaccess/options.h"
#include "khash.h"
#include "ksort.h"
#include "goaccess/settings.h"

typedef struct {
    const char *name;
    int total;
} col_t;
#define col_gt(a, b) ((a).total > (b).total)
KSORT_INIT(col, col_t, col_gt)

GConf conf = {
    .append_method = 1,
    .append_protocol = 1,
    .hl_header = 1,
    .num_tests = 10,
};

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

// Callback that is called by the goaccess parser
// each time a log item (= valid line) is encountered
void
process_log (GLogItem * logitem)
{
    int empty;
    khint_t k;

    // Update the row and the columns for a row
    k = kh_put(rowh,rows,logitem->vhost,&empty);
    if (empty) {
        kh_key(rows, k) = strdup(logitem->vhost);
        kh_value(rows, k).cols = kh_init(str_int);
        kh_value(rows, k).total = 1;
    }
    else
        kh_value(rows, k).total++;

    khash_t(str_int) *row_cols = kh_value(rows, k).cols;
    k = kh_put(str_int,row_cols,logitem->status,&empty);
    if (empty) {
        kh_key(row_cols, k) = strdup(logitem->status);
        kh_value(row_cols, k) = 1;
    }
    else
        kh_value(row_cols, k)++;
    
    // Update the global columns object
    k = kh_put(str_int,cols,logitem->status,&empty);
    if (empty) {
        kh_key(cols, k) = strdup(logitem->status);
        kh_value(cols, k) = 1;
    }
    else
        kh_value(cols, k)++;
}

int get_term_width() {
    struct winsize ws;
    ioctl(0, TIOCGWINSZ, &ws); 
    if (errno > 0)
        return -1;
    else
        return ws.ws_col;
}

void print_ruler(int w0, int w, int n) {
    int i;
    w = w+1; // account for the field separator
    for (i = 0 ; i < w0 + w*n; i++)
        putchar((i - w0) % w == 0 && i >= w0 ? '+' : '-');
    putchar('\n');
}

int main(int argc, char const *argv[])
{
    int i,j;
    glog = init_log ();
    int ret;

    read_option_args(argc, argv);
    set_spec_date_format();

    rows = kh_init(rowh);
    cols = kh_init(str_int);

    int term_width = get_term_width();

    FILE *fp = fopen("data/access.log.1","r");
    conf.read_stdin = 1;
    conf.filenames[conf.filenames_idx++] = "-";
    glog->pipe = fp;
    
    if ((ret = parse_log (&glog, NULL, 0))) {
        goto clean;
    }


    khint_t k;
    const char *row, *col;
    int cnt,total_cnt;
    row_t row_val;

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
    // Print headers

    int h_width = 0, h2_width = 10;
    kh_foreach(rows, row, row_val, {
        int len = strlen(row);
        h_width = h_width < len ? len : h_width;
    })

    printf("%*s",h_width,"");
    for (i=0; i < cols_array_len; i++) {
        printf("|%*s",h2_width,cols_array[i].name);
    }
    printf("\n");

    print_ruler(h_width,h2_width,kh_size(cols)+1);

    // Print table contents
    for (j=0; j < rows_array_len; j++) {
        rowa_t row_j = rows_array[j];

        printf("%-*s", h_width, row_j.name);
        for (i=0; i < cols_array_len; i++) {
            k = kh_get(str_int,row_j.cols,cols_array[i].name);
            if (k != kh_end(row_j.cols))
                printf("|%*d",h2_width,kh_value(row_j.cols,k));
            else
                printf("|%*s",h2_width,"");
        }
        printf("|\033[1m%*d\033[0m",h2_width, row_j.total);
        printf("\n");
    }

    // print_ruler(h_width,h2_width,kh_size(cols));

    // print totals
    printf("%*s",h_width,"");
    for (i=0; i < cols_array_len; i++) {
        printf("|\033[1m%*d\033[0m",h2_width, cols_array[i].total);
    }
    printf("\n");

    clean:
    /* unable to process valid data */
    if (ret)
        output_logerrors (glog);
    return 0;
}
