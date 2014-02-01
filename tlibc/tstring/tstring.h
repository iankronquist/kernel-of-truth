#ifndef TSTRING_H
#define TSTRING_H


typedef char* tstring;

char elem(tstring str, int index);
char at(tstring str, int index);
unsigned long length(tstring str);
void extend(tstring str, int extra_space, tstring new_string);
void concat(tstring a, tstring b, tstring new_string);
void copy(tstring a, tstring b);
int compare(tstring a, tsring b);
unsigned long realLength(tstring a);
unsigned long prefixLength(tstring a);

#endif
