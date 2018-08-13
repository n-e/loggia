#include <string.h>
#include <stdlib.h>
#include "goaccess/parser.h"
#include "stddef.h"

typedef struct {
    char *label;
    int offset; // offset of the field in glogitem
    char *descr;
} field_t;

field_t fields[] = {
    {"agent", offsetof(GLogItem,agent),"Full User Agent String"},
    // browser, browser type, contient, country
    {"date", offsetof(GLogItem,date),"Date"},
    {"host", offsetof(GLogItem,host),"Client Host (usually its IP)"},
    // keyphrase
    {"method", offsetof(GLogItem,method),"HTTP Method (GET, POST...)"},
    // os, os_type
    {"protocol", offsetof(GLogItem,protocol),"Protocol (HTTP/1.1...)"},
    // qstr
    {"referer", offsetof(GLogItem,ref),"Referer"},
    {"req", offsetof(GLogItem,req),"Request (/plip/plop)"},
    // req_key
    {"status", offsetof(GLogItem,status),"Status Code (404...)"},
    {"time", offsetof(GLogItem,time),"Time"},
    // uniq_key
    {"vhost", offsetof(GLogItem,vhost),"Virtual Host"},
    // userid, + uninteresting ints
    {0, 0, 0}
};

char *getstr(GLogItem *base, size_t off) {
    return *(char**)((char*)base + off); 
}

static field_t *find_field (char *label) {
    for(int i = 0; fields[i].label != 0; i++) {
        if(!strcmp(label,fields[i].label))
            return &fields[i];
    }
    return 0;
}

void print_fields() {
    for(int i = 0; fields[i].label != 0; i++) {
        fprintf(stderr,"\t%-10s %s\n",fields[i].label,fields[i].descr);
    }
}

// Create a key from the spec
// e.g. if spec = "vhost", returns the vhost of logitem
// The return value is a pointer to a malloc()ed string that can be free()d
char *create_key(GLogItem *logitem, char *spec) {
    if (!strcmp(spec,""))
        return "";

    field_t *f = find_field(spec);

    if (!f) {
        fprintf(stderr,"Invalid spec \"%s\".\n\nValid values are:\n",spec);
        print_fields();
        exit(1);
    }

    char *item = getstr(logitem,f->offset);

    char *key = item ? strdup(item) : "(not set)";

    return key;
}