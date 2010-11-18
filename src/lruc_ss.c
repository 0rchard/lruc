#include "lruc_ss.h"

#include <stdlib.h>
#include <string.h>

void _lruc_ss_destroy(char** key, char** value){
    free(*key);
    free(*value);
}

int _lruc_ss_comp(char** key, char** value){
   return strcmp(*key, *value); 
}

int _lruc_ss_hash(char** key){
    return 0;
}

lruc_t lruc_new_ss(unsigned int bsize, unsigned int max){
    return lruc_new(NULL, 
            (hash_f*)_lruc_ss_hash, 
            (comp_f*)_lruc_ss_comp, 
            (destroy_f*)_lruc_ss_destroy,
            sizeof(char*), sizeof(char*),
            bsize, max);
}


int lruc_ss_insert(lruc_t lruc, const char* key, const char* value){
    lruc_node_t node = lruc_alloc_node(lruc); 

    if(node == NULL){
        return 1;
    }

    *((char**)LRUC_NODE_KEY(lruc, node)) = strdup(key);
    *((char**)LRUC_NODE_VALUE(lruc, node)) = strdup(value);


    return lruc_insert_node(lruc, node);
}

const char* lruc_ss_find(lruc_t lruc, const char* key){

    lruc_node_t node = lruc_find(lruc, &key);

    if(node == NULL){
        return NULL;
    }

    return *((char**)LRUC_NODE_VALUE(lruc, node));
}
