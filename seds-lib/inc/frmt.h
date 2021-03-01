#ifndef __FRMT_H
#define __FRMT_H

#include <time.h>
#include "cache.h"

int frmt_set_resline(char *buf, int size, char *ver, int code);
int frmt_set_servline(char *buf, int size, char *serv);
int frmt_set_date(char *buf, int size, time_t *date, char *title);

int frmt_set_cont_len(char *buf, int size, int len);
int frmt_set_cont_type(char *buf, int size, char *type);

#endif
