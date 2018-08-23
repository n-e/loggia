#include <string.h>
#include <stdlib.h>
#include "goaccess/parser.h"
#include "stddef.h"

typedef struct {
    char *label;
    int offset; // offset of the field in glogitem
    char *descr;
    char *prepend; // characters to prepend in help
} field_t;

field_t fields[] = {
    {"agent", offsetof(GLogItem,agent),"Full User Agent String",""},
    {"browser", offsetof(GLogItem,browser),"Full Browser String (e.g.: Chrome/68.0.3440...)","  "},
    {"browser_type", offsetof(GLogItem,browser_type),"Browser type (e.g. Chrome, Crawlers...)","  "},
    // continent, country
    {"date", offsetof(GLogItem,date),"Date",""},
    {"host", offsetof(GLogItem,host),"Client Host (usually its IP)",""},
    // keyphrase
    {"method", offsetof(GLogItem,method),"HTTP Method (GET, POST...)",""},
    // os, os_type
    {"protocol", offsetof(GLogItem,protocol),"Protocol (HTTP/1.1...)",""},
    // qstr
    {"referer", offsetof(GLogItem,ref),"Referer",""},
    {"req", offsetof(GLogItem,req),"Request (/plip/plop)",""},
    // req_key
    {"status", offsetof(GLogItem,status),"Status Code (404...)",""},
    {"time", offsetof(GLogItem,time),"Time",""},
    // uniq_key
    {"vhost", offsetof(GLogItem,vhost),"Virtual Host",""},
    {"url", offsetof(GLogItem,userid),"vhost + req",""}, // CAUTION: user_id is reused as url
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
        fprintf(
            stderr,
            "\t%s%-*s %s\n",
            fields[i].prepend,
            15-(int)strlen(fields[i].prepend),
            fields[i].label,
            fields[i].descr);
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

int match_filterspec(GLogItem *logitem, char *spec) {
    if (*spec == 0)
        return 1;

    // Parse results
    char key[20] = {0};
    int op = 0; // 1 : =, 2: <>
    char val[200] = {0};

    // Parsing
    int cursor = 0;
    while (((spec[cursor] >= 'a' && spec[cursor] <='z') || spec[cursor]== '_' ) && spec[cursor]!=0)
        cursor++;
    strncpy(key,spec,cursor); key[cursor < 20 ? cursor:20]= 0;

    if ((spec[cursor] == '=' || !strncmp(&spec[cursor],"<>",2)) && spec[cursor]!=0) {
        op = spec[cursor] == '=' ? 1 : 2;
        cursor++;
        if (op == 2)
            cursor++;
    }

    int start = cursor;
    while (spec[cursor]!=0)
        cursor++;
    strncpy(val,&spec[start],cursor-start); key[cursor < 200 ? cursor:200]= 0;


    field_t *f = find_field(key);
    if (!f) {
        fprintf(stderr,"Invalid key \"%s\".\n\nValid values are:\n",key);
        print_fields();
        exit(1);
    }

    char *item = getstr(logitem,f->offset);

    if (op == 1)
        return !strcmp(item,val);
    else
        return strcmp(item,val);
}