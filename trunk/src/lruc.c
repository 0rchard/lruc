#include "lruc_imp.h"

#include <stdlib.h>
#include <string.h>

void* lruc_node_key(lruc_t lruc, lruc_node_t node){
    LRUC_DEBUG_CHECK(lruc);
    LRUC_DEBUG_CHECK_NODE(lruc, node);
    
    return LRUC_NODE_KEY(lruc, node);
}


void* lruc_node_value(lruc_t lruc, lruc_node_t node){
    LRUC_DEBUG_CHECK(lruc);
    LRUC_DEBUG_CHECK_NODE(lruc, node);

    return LRUC_NODE_VALUE(lruc, node);
}

#ifdef _g_LRUC_DEBUG_

int lruc_margin_check(lrucmargin_t margin){
    if(margin->margin1==DEBUG_MARGIN1 && 
            margin->margin2==DEBUG_MARGIN1){
        return 0;
    }
    return 1;
}

void lruc_margin_init(lrucmargin_t margin){
    margin->margin1=DEBUG_MARGIN1;
    margin->margin2=DEBUG_MARGIN2;
}

void lruc_debug_check(lruc_t lruc){
    //assert(lruc);
    return;
}

#endif

//
lruc_t lruc_new(lruc_alloc_t alloc, hash_f *hash, comp_f *comp, destroy_f *destroy, 
        unsigned int ksize, unsigned int vsize, unsigned int bsize, unsigned int max){

    lruc_t lruc;
    void * mem = NULL;
    unsigned int size = sizeof(struct lruc_st);

    size += sizeof(struct lruc_node_list_st) * bsize;

    if(alloc==NULL){
        alloc = &_lruc_alloc_def;
    }

    mem = alloc->alloc(alloc->context, size);

    if(mem == NULL){
        return NULL;
    }

    lruc = mem;
    lruc->alloc = alloc;

    lruc->hash = hash;
    lruc->comp = comp;
    lruc->destroy  = destroy;

    lruc->ksize = ksize;
    lruc->vsize = vsize;
    lruc->nsize = ksize + vsize + sizeof(struct lruc_node_st);

    lruc->koffset = sizeof(struct lruc_node_st);
    lruc->voffset = lruc->koffset + lruc->ksize;

    lruc->max = max;
    lruc->bsize = bsize;

    lruc->bucket = (lruc_node_list_t)(((char*)lruc) + sizeof(struct lruc_st));

    TAILQ_INIT(&lruc->fifo);

    int i=0;
    for(; i<bsize; i++){
        TAILQ_INIT(&lruc->bucket[i]);
    }

    return lruc;
}

void lruc_set_cookie(lruc_t lruc, void* cookie){
    LRUC_DEBUG_CHECK(lruc);
   
    lruc->cookie = cookie;
}

void lruc_set_max_size(lruc_t lruc, int max){
    LRUC_DEBUG_CHECK(lruc);
    
    lruc->max = max;
}

void lruc_free(lruc_t lruc){
    LRUC_DEBUG_CHECK(lruc);

    lruc_node_t node;

    while(!TAILQ_EMPTY(&lruc->fifo)){
        node = TAILQ_FIRST(&lruc->fifo);
        TAILQ_REMOVE(&lruc->fifo, node, queue_link);
        lruc->alloc->free(lruc->alloc->context, node);
    }

    lruc->alloc->free(lruc->alloc->context, lruc);
}

//create a new node in heap
lruc_node_t lruc_alloc_node(lruc_t lruc){
    LRUC_DEBUG_CHECK(lruc);

    lruc_node_t node = 
        lruc->alloc->alloc(lruc->alloc->context, lruc->nsize);

#ifdef _g_LRUC_DEBUG_
    if(node){
        node->lruc = lruc;
    }
#endif 

    return node;
}

//which never insert into lru
void lruc_free_node(lruc_t lruc, lruc_node_t node){
    LRUC_DEBUG_CHECK(lruc);
    LRUC_DEBUG_CHECK_NODE(lruc, node);

    lruc->destroy(LRUC_NODE_KEY(lruc, node), LRUC_NODE_VALUE(lruc, node));
    lruc->alloc->free(lruc->alloc->context, node);
}

//interate
lruc_node_t lruc_walk(lruc_t lruc, walkcb_f *walkcb){
    LRUC_DEBUG_CHECK(lruc);

    int ret;
    lruc_node_t node;
    lruc_node_t prev=NULL;

#ifdef _g_LRUC_DEBUG_ 
    lruc->in_walk = 1;
#endif

it_begin:
    if(!TAILQ_EMPTY(&lruc->fifo)){
        TAILQ_FOREACH(node, &lruc->fifo, queue_link){
            ret = walkcb(lruc, node, LRUC_NODE_KEY(lruc, node), LRUC_NODE_VALUE(lruc, node));

            if(ret & 0x02){
                int index = LRUC_HASH_INDEX(lruc, LRUC_NODE_KEY(lruc, node));

                TAILQ_REMOVE(&(lruc->bucket[index]), node, bucket_link);
                TAILQ_REMOVE(&(lruc->fifo), node, queue_link);

                lruc->destroy(LRUC_NODE_KEY(lruc, node), LRUC_NODE_VALUE(lruc, node));
                lruc->alloc->free(lruc->alloc->context, node);
            }
            else{
                prev = node;
            }

            if(ret & 0x01){
                break;
            }
            else{
                if(prev == NULL){
                    goto it_begin;
                }
            }
        }
    }

#ifdef _g_LRUC_DEBUG_ 
    lruc->in_walk = 0;
#endif
    return NULL;
}

static int _lruc_lower_bound(lruc_t lruc, void* key, int* index, lruc_node_t* pnode){
    int ret;
    lruc_node_t node;

    *index = LRUC_HASH_INDEX(lruc, key);

    if(!TAILQ_EMPTY(&lruc->bucket[*index])){
        TAILQ_FOREACH(node, &lruc->bucket[*index], bucket_link){
            ret = lruc->comp(key, LRUC_NODE_KEY(lruc, node));

            if(ret == 0){
                *pnode = node;
                break;
            }
            else if(ret < 0){
                *pnode = node;
            }
            else{
                break;
            }
        }
    }

    return ret;
}

//add a node into lru
int lruc_insert(lruc_t lruc, void* key, void*  value){
    LRUC_DEBUG_CHECK(lruc);

    lruc_node_t onode = NULL;
    lruc_node_t nnode= NULL;

    int index = 0;

    int ret = _lruc_lower_bound(lruc, key, &index, &onode);

    if(ret == 0 && onode){
        if(lruc->destroy){
            lruc->destroy(LRUC_NODE_KEY(lruc, onode), LRUC_NODE_VALUE(lruc, onode));
        }

        memcpy(LRUC_NODE_KEY(lruc, onode), key, lruc->ksize);
        memcpy(LRUC_NODE_VALUE(lruc, onode), value, lruc->vsize);

        LRUC_NODE_TOUCH(lruc, onode);
        return 0;
    }

    if(lruc->count >= lruc->max){
        nnode = TAILQ_LAST(&lruc->fifo, lruc_node_list_st);
        
        TAILQ_REMOVE(&(lruc->bucket[index]), nnode, bucket_link);
        TAILQ_REMOVE(&lruc->fifo, nnode, queue_link);

        lruc->destroy(LRUC_NODE_KEY(lruc, nnode), LRUC_NODE_VALUE(lruc, nnode));
    }
    else{
        nnode = lruc_alloc_node(lruc);

        if( nnode == NULL ){
            return 1;
        }

        lruc->count ++;
    }

    memcpy(LRUC_NODE_KEY(lruc, nnode), key, lruc->ksize);
    memcpy(LRUC_NODE_VALUE(lruc, nnode), value, lruc->vsize);

    if(onode == NULL){
        TAILQ_INSERT_HEAD(&lruc->bucket[index], nnode, bucket_link); 
    }
    else{
        TAILQ_INSERT_AFTER(&lruc->bucket[index], onode, nnode, bucket_link);
    }

    TAILQ_INSERT_HEAD(&lruc->fifo, nnode, queue_link);

    return 0;
}

int lruc_insert_node(lruc_t lruc, lruc_node_t node){
    LRUC_DEBUG_CHECK(lruc);
    LRUC_DEBUG_CHECK_NODE(lruc, node);

    lruc_node_t onode = NULL;
    lruc_node_t nnode= NULL;

    int index = 0;

    int ret = _lruc_lower_bound(lruc, LRUC_NODE_KEY(lruc, node), &index, &onode);

    if(ret == 0 && onode){
        if(lruc->destroy){
            lruc->destroy(LRUC_NODE_KEY(lruc, onode), LRUC_NODE_VALUE(lruc, onode));
        }

        memcpy(LRUC_NODE_KEY(lruc, onode), LRUC_NODE_KEY(lruc, node), lruc->ksize);
        memcpy(LRUC_NODE_VALUE(lruc, onode), LRUC_NODE_VALUE(lruc, node), lruc->vsize);

        LRUC_NODE_TOUCH(lruc, onode);

        lruc->alloc->free(lruc->alloc->context, node);
        return 0;
    }

    if(lruc->count >= lruc->max){
        nnode = TAILQ_LAST(&lruc->fifo, lruc_node_list_st);

        TAILQ_REMOVE(&(lruc->bucket[index]), nnode, bucket_link);
        TAILQ_REMOVE(&lruc->fifo, nnode, queue_link);

        lruc->destroy(LRUC_NODE_KEY(lruc, nnode), LRUC_NODE_VALUE(lruc, nnode));
        lruc->alloc->free(lruc->alloc->context, nnode);
    }
    else{
        lruc->count ++;
    }

    if(onode == NULL){
        TAILQ_INSERT_HEAD(&lruc->bucket[index], node, bucket_link); 
    }
    else{
        TAILQ_INSERT_AFTER(&lruc->bucket[index], onode, node, bucket_link);
    }

    TAILQ_INSERT_HEAD(&lruc->fifo, node, queue_link);

    return 0;
}

void* lruc_find(lruc_t lruc, void* key){
     LRUC_DEBUG_CHECK(lruc);

    lruc_node_t node = NULL;

    int index = 0;
    int ret = _lruc_lower_bound(lruc, key, &index, &node);

#ifdef _g_LRUC_INFO__
    lruc->info.count ++;
#endif

    if(ret == 0){
#ifdef _g_LRUC_INFO__
        lruc->info.hit ++;
#endif
        return LRUC_NODE_VALUE(lruc, node);
    }
    
    return NULL;
}

//find in lruc
lruc_node_t lruc_find_node(lruc_t lruc, void* key){
    LRUC_DEBUG_CHECK(lruc);


    lruc_node_t node = NULL;

    int index = 0;
    int ret = _lruc_lower_bound(lruc, key, &index, &node);

#ifdef _g_LRUC_INFO__
    lruc->info.count ++;
#endif

    if(ret == 0){
#ifdef _g_LRUC_INFO__
        lruc->info.hit ++;
#endif
        return node;
    }
    
    return NULL;
}

//should not be call in walk callback
int lruc_del(lruc_t lruc, void* key){
    LRUC_DEBUG_CHECK(lruc);

    lruc_node_t node = NULL;

    int index = 0;
    int ret = _lruc_lower_bound(lruc, key, &index, &node);

    if(ret == 0){
        TAILQ_REMOVE(&(lruc->bucket[index]), node, bucket_link);
        TAILQ_REMOVE(&lruc->fifo, node, queue_link);

        lruc_free_node(lruc, node);
        return 0;
    }

    return 1;
}

int lruc_del_node(lruc_t lruc, lruc_node_t node){
    LRUC_DEBUG_CHECK(lruc);
    LRUC_DEBUG_CHECK_NODE(lruc, node);

    int index = LRUC_HASH_INDEX(lruc, LRUC_NODE_KEY(lruc, node));

    TAILQ_REMOVE(&(lruc->bucket[index]), node, bucket_link);
    TAILQ_REMOVE(&lruc->fifo, node, queue_link);

    lruc_free_node(lruc, node);

    return 1;
} 


//
lruc_info_t lruc_info(lruc_t lruc){
#ifdef _g_LRUC_INFO__
    return &lruc->info;
#else
    return NULL;
#endif
}

