#ifndef _g_LRUC_SS_H__
#define _g_LRUC_SS_H__

#include "lruc.h"

lruc_t lruc_ss_new(unsigned int bsize, unsigned int max);

int lruc_ss_insert(lruc_t, const char* key, const char* value);

const char* lruc_ss_find(lruc_t, const char* key);

#define lruc_ss_free(lruc) lruc_free(lruc)


#endif 
