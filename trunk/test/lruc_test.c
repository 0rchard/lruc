#include "lruc_ss.h"
#include <stdio.h>

int main(){

    lruc_t lruc = lruc_ss_new(64, 128);

    //-----------------------------------
    printf("insert d.");
    lruc_ss_insert(lruc, "d", "dddd");

    const char* v = lruc_ss_find(lruc, "d");
    printf("key=d value=%s\n", v);

    //-----------------------------------
    printf("insert b.");
    lruc_ss_insert(lruc, "b", "bbbb");

    v = lruc_ss_find(lruc, "b");
    printf("key=b value=%s\n", v);

    //-----------------------------------
    printf("insert g.");
    lruc_ss_insert(lruc, "g", "gggg");
    
    v = lruc_ss_find(lruc, "g");
    printf("key=g value=%s\n", v);

    //-----------------------------------
    printf("insert e.");
    lruc_ss_insert(lruc, "e", "eeee");

    v = lruc_ss_find(lruc, "e");
    printf("key=e value=%s\n", v);

    //-----------------------------------
    printf("replace e.");
    lruc_ss_insert(lruc, "e", "xxxx");
    
    v = lruc_ss_find(lruc, "e");
    printf("key=e value=%s\n", v);

    lruc_free(lruc);
}
