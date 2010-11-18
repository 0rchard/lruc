#include "lruc.h"
#include <stdlib.h>


void* _lruc_alloc_fun(void* context, unsigned int size){
    return calloc(1, size);
}

void _lruc_free_fun(void* context, void* p){
    return free(p);
}

struct lruc_alloc_st _lruc_alloc_def={
    NULL, _lruc_alloc_fun, _lruc_free_fun};



