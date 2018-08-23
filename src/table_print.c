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

#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include "table_print.h"

int get_term_width() {
    struct winsize ws;
    ioctl(0, TIOCGWINSZ, &ws); 
    if (errno > 0)
        return -1;
    else
        return ws.ws_col;
}

void print_ruler(TableSpec s) {
    int i;
    int w = s.w+1; // account for the field separator
    for (i = 0 ; i < s.w0 + w*s.ncols; i++)
        putchar((i - s.w0) % w == 0 && i >= s.w0 ? '+' : '-');
    putchar('\n');
}

static void print_row_any(int width, const char *str) {
    int len = strlen(str);

    if (len <= width)
        printf("%-*s",width,str);
    else {
        char str2[width+1];
        strncpy(str2,str,width);
        str2[width] = 0;
        if (width >= 3)
            strcpy(&str2[width-3],"...");
        printf("%s",str2);
    }
}

void print_row_header(TableSpec s, const char *str) {
    print_row_any(s.w0,str);
}

void print_row_value(TableSpec s, const char *str) {
    putchar('|');
    print_row_any(s.w,str);
}

void print_row_value_int(TableSpec s, int val) {
    char buf[s.w+1];
    snprintf(buf, s.w+1, "%*d", s.w,val);
    print_row_value(s,buf);
}

static int get_row_width(TableSpec s) {
    return s.w0 + s.ncols * (s.w+1);
}

#define retifok() {if (get_row_width(*s) < term_width) goto ret;}
#define MAX(a,b) ((a) > (b) ? a : b)
#define MIN(a,b) ((a) < (b) ? a : b)
void crop_to_termwidth(TableSpec *s) {
    int term_width = get_term_width()-1;
    int initial_w0 = s->w0;
    int rw; 
    if (term_width < 20) term_width = 20;
    
    retifok()

    if (s->w0 > term_width/2) s->w0 = term_width / 2;
    retifok()
    if (s->w0 > term_width/3) s->w0 = MAX(term_width / 3,10);
    retifok()
    while (s->w > 15) {
        s->w--;
        retifok()
    }
    if (s->w0 > 15) s->w0 = 15;
    retifok()
    // really too small if we do the next
    /*while (s->w > s->minw) {
        s->w--;
        retifok()
    }*/

    // We did everything we could, remove columns now.
    while((rw = get_row_width(*s))>term_width && s-> ncols > 1) {
        s->ncols--;
        retifok()
    }
    ret:
    while((rw = get_row_width(*s))<term_width && s->w0 < initial_w0)
        s->w0++;
    return;
}