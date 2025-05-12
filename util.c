#include "util.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Append helper */
static void append_char(char **buf, size_t *len, size_t *cap, char c) {
    if (*len + 1 >= *cap) {
        *cap *= 2;
        *buf = realloc(*buf, *cap);
    }
    (*buf)[(*len)++] = c;
}

char *unescape(const char *src) {
    size_t cap = 64, len = 0;
    char *out = malloc(cap);
    for (size_t i = 0; src[i]; ) {
        if (src[i]=='\\' && src[i+1]) {
            i++;
            char c = src[i++];
            switch (c) {
              case 'n': append_char(&out,&len,&cap,'\n'); break;
              case 't': append_char(&out,&len,&cap,'\t'); break;
              case 'r': append_char(&out,&len,&cap,'\r'); break;
              case 'b': append_char(&out,&len,&cap,'\b'); break;
              case 'f': append_char(&out,&len,&cap,'\f'); break;
              case '"': append_char(&out,&len,&cap,'"');  break;
              case '\\':append_char(&out,&len,&cap,'\\'); break;
              case 'u': {
                if (isxdigit(src[i])&&isxdigit(src[i+1])&&
                    isxdigit(src[i+2])&&isxdigit(src[i+3])) {
                  char hex[5]={src[i],src[i+1],src[i+2],src[i+3],0};
                  unsigned code = strtoul(hex,NULL,16);
                  if (code<=0x7F) {
                    append_char(&out,&len,&cap,(char)code);
                  } else if (code<=0x7FF) {
                    append_char(&out,&len,&cap,(char)(0xC0|(code>>6)));
                    append_char(&out,&len,&cap,(char)(0x80|(code&0x3F)));
                  } else {
                    append_char(&out,&len,&cap,(char)(0xE0|(code>>12)));
                    append_char(&out,&len,&cap,(char)(0x80|((code>>6)&0x3F)));
                    append_char(&out,&len,&cap,(char)(0x80|(code&0x3F)));
                  }
                  i += 4;
                }
                break;
              }
              default:
                append_char(&out,&len,&cap,c);
            }
        } else {
            append_char(&out,&len,&cap,src[i++]);
        }
    }
    out[len] = '\0';
    return out;
}
