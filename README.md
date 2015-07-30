# lruc
Automatically exported from code.google.com/p/lruc
A lightweight c/c++ LRU memory cache. 

Example:


#include "lruc_ss.h"
#include <stdio.h>
int main(){
    lruc_t lruc = lruc_ss_new(64, 128);
    lruc_ss_insert(lruc, "fan", "abcd");
    const char* v = lruc_ss_find(lruc, "fan");
    if(v != 0)
        printf("v = %s\n", v);
    lruc_ss_free(lruc);
}
