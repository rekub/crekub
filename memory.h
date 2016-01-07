#include <stdlib.h>
void * Malloc(size_t sz);
void * Realloc(void *p, size_t sz);
char * Strdup(const char * s);
char * Stradd(const char *, const char *);
#define Free(p) free((void*)p)
void Astrcat(char ** dst, char * src);
#define ForgetStr(s) do if(s) { Free(s); s=0; } while(0)
void Asprintf(char ** dst, const char * format, ...);
