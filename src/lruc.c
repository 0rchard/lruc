#include "lruc.h"
#include <stdlib.h>
#include <string.h>

void* _lru_alloc_fun(void* context, unsigned int size){
    return calloc(1, size);
}

void _lru_free_fun(void* context, void* p){
    return free(p);
}

static struct lru_alloc_st _lru_alloc_def={
    NULL, _lru_alloc_fun, _lru_free_fun};

#ifdef _g_LRUC_DEBUG_

struct lru_margin_str{
    int margin1;
    int margin2;
};

typedef struct lru_margin_str* lru_margin_t;

int lruc_margin_check(lru_margin_t margin){
    if(margin->margin1==DEBUG_MARGIN1 && 
            margin->margin2==DEBUG_MARGIN1){
        return 0;
    }
    return 1;
}

void lruc_margin_init(lru_margin_t margin){
    margin->margin1=DEBUG_MARGIN1;
    margin->margin2=DEBUG_MARGIN2;
}

void lruc_debug_check(lru_t lru){
    //assert(lru);
    return;
}

#endif


//
lru_t lruc_new(lru_alloc_t alloc, hash_f *hash, comp_f *comp, destory_f *destory, 
        unsigned int ksize, unsigned int vsize, unsigned int bsize, unsigned int max){

    lru_t lru;
    void * mem = NULL;
    unsigned int size = sizeof(struct lru_st);

    size += sizeof(struct lru_node_list_st) * bsize;

    if(alloc==NULL){
        alloc = &_lru_alloc_def;
    }

    mem = alloc->alloc(alloc->context, size);

    if(mem == NULL){
        return NULL;
    }

    lru = mem;
    lru->alloc = alloc;

    lru->hash = hash;
    lru->comp = comp;
    lru->destory  = destory;

    lru->ksize = ksize;
    lru->vsize = vsize;
    lru->nsize = ksize + vsize + sizeof(struct lru_node_st);

    lru->koffset = sizeof(struct lru_node_st);
    lru->voffset = lru->koffset + lru->ksize;

    lru->max = max;
    lru->bsize = bsize;

    lru->bucket = (lru_node_list_t)(((char*)lru) + sizeof(struct lru_st));

    TAILQ_INIT(&lru->fifo);

    int i=0;
    for(; i<bsize; i++){
        TAILQ_INIT(&lru->bucket[i]);
    }

    return lru;
}

void lruc_set_cookie(lru_t lru, void* cookie){
    G_LRUC_DEBUG_CHECK(lru);
   
    lru->cookie = cookie;
}

void lruc_set_max_size(lru_t lru, int max){
    G_LRUC_DEBUG_CHECK(lru);
    
    lru->max = max;
}

void lruc_free(lru_t lru){
    G_LRUC_DEBUG_CHECK(lru);

    lru_node_t node;

    while(!TAILQ_EMPTY(&lru->fifo)){
        node = TAILQ_FIRST(&lru->fifo);
        TAILQ_REMOVE(&lru->fifo, node, queue_link);
        lru->alloc->free(lru->alloc->context, node);
    }

    lru->alloc->free(lru->alloc->context, lru);
}

//create a new node in heap
lru_node_t lruc_alloc_node(lru_t lru){
    G_LRUC_DEBUG_CHECK(lru);

    return lru->alloc->alloc(lru->alloc->context, lru->nsize);
}

//which never insert into lru
void lruc_free_node(lru_t lru, lru_node_t node){
    G_LRUC_DEBUG_CHECK(lru);
    G_LRUC_DEBUG_CHECK_NODE(lru, node);

    lru->alloc->free(lru->alloc->context, node);
}

//interate
lru_node_t lruc_walk(lru_t lru, walkcb_f *walkcb){
    G_LRUC_DEBUG_CHECK(lru);

    return NULL;
}

int lruc_find_below(lru_t lru, void* key, void* value, lru_node_t* node){

    return 0;
}

#define LRUC_NODE_KEY(lru, node) \
    (((char*)(node))+(lru->koffset))

#define LRUC_NODE_VALUE(lru, node) \
    (((char*)(node))+(lru->voffset))

#define LRUC_NODE_TO_HEADER(lru, node)

//add a node into lru
int lruc_insert(lru_t lru, void* key, void*  value){
    G_LRUC_DEBUG_CHECK(lru);

    lru_node_t onode = NULL;
    lru_node_t nnode= NULL;

    int index = 0;

    int ret = lruc_find_below(lru, key, &index, &onode);

    if(ret == 0){
        if(lru->destory){
            lru->destory(onode);
        }

        memcpy(LRUC_NODE_KEY(lru, onode), key, lru->ksize);
        memcpy(LRUC_NODE_VALUE(lru, onode), value, lru->vsize);
        LRUC_NODE_TO_HEADER(lru, onode);
        return 0;
    }

    nnode = lruc_alloc_node(lru);

    if( nnode == NULL ){
        return 1;
    }

    memcpy(LRUC_NODE_KEY(lru, nnode), key, lru->ksize);
    memcpy(LRUC_NODE_VALUE(lru, nnode), value, lru->vsize);

    if(onode){
        //insert in queue
    }
    else{
        //insert in header
    }

    //insert in to fifo
}

int lruc_insert_node(lru_t lru, lru_node_t node){
    G_LRUC_DEBUG_CHECK(lru);
    G_LRUC_DEBUG_CHECK_NODE(lru, node);

    lru_node_t onode = NULL;
    lru_node_t nnode= NULL;

    int index = 0;

    int ret = lruc_find_below(lru, LRUC_NODE_KEY(lru, node), &index, &onode);


    if(ret == 0){
        
    }
}

//find in lru
lru_node_t lruc_find(lru_t lru, void* key){
    G_LRUC_DEBUG_CHECK(lru);

    lru_node_t onode = NULL;
    lru_node_t nnode= NULL;

    int index = 0;
    int ret = lruc_find_below(lru, key, &index, &onode);

    if(ret == 0){
        return onode;
    }
    
    return NULL;
}

//should not be call in walk callback
void lruc_del(lru_t lru, void* key){
    G_LRUC_DEBUG_CHECK(lru);

    lru_node_t onode = NULL;
    lru_node_t nnode= NULL;

    int index = 0;
    int ret = lruc_find_below(lru, key, &index, &onode);

    lru_node_t node = lruc_find(lru, key);

    if(node){
        lruc_del_node(lru, node);
        return;
    }
}

void lruc_del_node(lru_t lru, lru_node_t node){
    G_LRUC_DEBUG_CHECK(lru);

    //release in hash table
    //
    //

    
    lruc_free_node(lru, node);
} 


#ifdef _g_LRUC_INFO__
//
lru_info_t lru_info(lru_t lru){
    return &lru->info;
}
#endif

