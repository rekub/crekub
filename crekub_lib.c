#include "error.h"
#include "memory.h"
#include "text.h"
#include <errno.h>
#include <stdarg.h>
#include <string.h>
#ifdef RAISE_ON_ERROR
#  include <signal.h>
#endif
#define TEXTCHUNKSIZE 1024

void * Malloc(size_t sz) {
  void * p = malloc(sz);
  if (!p) error(0);
  return p;
}

char * Strdup(const char * s) {
  char * copy = strdup(s);
  if (!copy) error(0);
  return copy;
}

char * Stradd(const char * s1, const char * s2) {
  char * res = Malloc(strlen(s1) + strlen(s2) + 1);
  (void)strcpy(stpcpy(res,s1),s2);
  return res;
}

void * Realloc(void * p, size_t sz) {
  p = realloc(p,sz);
  if (!p) error(0);
  return p;
}

void error(const char * errmsg, ...) {
  if (errmsg) {
    va_list ap;
    va_start(ap, errmsg);
    vfprintf(stderr, errmsg, ap);
    va_end(ap);
    fprintf(stderr, "\n");
  }
  fprintf(stderr, "error: %d - ",errno);
  perror(NULL);
#ifdef RAISE_ON_ERROR
  raise(RAISE_ON_ERROR);
#endif
  if (errno) exit(errno);
  exit(127);
}

void OpenText(const char * name, text * t) {
  if (SafeOpenText(name,t)) error("Cannot open file: \"%s\".",name);
}

int SafeOpenText(const char * name, text * t) {
  if (name) {
    t->name = Strdup(name);
    t->f = fopen(t->name,"r");
    if (!t->f) {
      free(t->name);
      return -1;
    }
  }
  else {
    t->f = stdin;
    t->name = 0;
  }
  t->buffer = Malloc(TEXTCHUNKSIZE);
  t->sz = TEXTCHUNKSIZE;
  return 0;
}

void CloseText(text * t) {
  free(t->buffer);
  if (t->name) {
    if (fclose(t->f)) error("Cannot close file: \"%s\".",t->name);
    free(t->name);
  }
}

const char * GetLine(text * t) {
  char * line;
  size_t L;
  if (line = fgetln(t->f,&L)) {
    if (!L) {
      errno = EDOOFUS;
      error("fgetln does not work");
    }
    if (line[L-1] == '\n') --L;
    if (t->sz < L+1) {
      t->sz = (L/TEXTCHUNKSIZE+1)*TEXTCHUNKSIZE;
      t->buffer = Realloc(t->buffer,t->sz);
    }
    t->buffer[L] = 0;
    memcpy(t->buffer,line,L);
    return t->buffer;
  }
  else {
    if (ferror(t->f)) {
      if (t->name) error("Cannot read file \"%s\".",t->name);
      else error("Cannot read from stdin");
    }
    return 0;
  }
}

void Astrcat(char ** dst, char * src) {
  if (!*dst) *dst = src;
  else {
    *dst = Realloc(*dst,strlen(*dst)+strlen(src)+1);
    strcat(*dst,src);
    Free(src);
  }
}

void Asprintf(char ** dst, const char * format, ...) {
  va_list ap;
  va_start(ap, format);
  if (-1 == vasprintf(dst, format, ap)) error(0);
  va_end(ap);
}

void CopyTail(const char * name, text * t,unsigned * size, unsigned * crc32) {
  unsigned crc_table[256];
  unsigned crc; int i, j;
  for (i = 0; i < 256; i++)
  {
    crc = i;
    for (j = 0; j < 8; j++)
      crc = crc & 1 ? (crc >> 1) ^ 0xEDB88320UL : crc >> 1;
    crc_table[i] = crc;
  };
  crc = 0xFFFFFFFFUL;
  FILE * out = fopen(name,"w");
  if (!out) error("Cannot open file \"%s\"",name);
  int c;
  *size = 0;
  while (EOF != (c = getc(t->f))) {
    ++*size;
    crc = crc_table[(crc ^ c) & 0xFF] ^ (crc >> 8);
    if (EOF == putc(c,out)) error("Cannot write file \"%s\"",name);
  }
  if (ferror(t->f)) {
    if (t->name) error("Cannot read file \"%s\".",t->name);
    else error("Cannot read from stdin");
  }
  if (fclose(out)) error("Cannot close file \"%s\"",name);
  CloseText(t);
  *crc32 = crc ^ 0xFFFFFFFFUL;
}
