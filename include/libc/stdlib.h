#ifndef __STDLIB__
#define __STDLIB
int atoi(const char *s);

int setenv (const char *name, const char *value, int overwrite);
void ftoa (float n, char *result, int precision);
void itoa (int xx, char *str, int base, int sgn);
//uint strlen (char *s);
char * strncpy (char *s, const char *t, int n);
#endif