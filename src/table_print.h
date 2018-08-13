typedef struct {
    int w0;
    int w;
    int minw;
    int ncols; // number of columns (except for the row title)
} TableSpec;

int get_term_width();
void print_ruler(TableSpec s);
void print_row_header(TableSpec s, const char *str);
void print_row_value(TableSpec s, const char *str);
void print_row_value_int(TableSpec s, int val);
void crop_to_termwidth(TableSpec *s);