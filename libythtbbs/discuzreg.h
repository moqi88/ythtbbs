/* discuzreg.c */
#ifndef __DISCUZREG_H
#define __DISCUZREG_H
// discuzÏÂµÄusername length
#define DISCUZ_USERNAME_LENGTH 15

#include "discuzsql.h"
#include "discuzmodule.h"
int discuzreg(char* useridutf8, char* passbuf, int salt);
int discuzupdateemail(char* userid, char* email, time_t firstlogin);

#endif
