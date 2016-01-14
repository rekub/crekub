#include <stdio.h>
typedef struct s_text {
  FILE * f;
  char * name;
  char * buffer;
  size_t sz;
} text;

void OpenText(const char * name, text * t);
int SafeOpenText(const char * name, text * t);
void CloseText(text * t);
const char * GetLine(text * t);
void CopyTail(const char * name, text * t,unsigned * size, unsigned * crc32);
