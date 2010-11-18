#ifndef __g_LRUC_IMP_H__
#define __g_LRUC_IMP_H__ 

#define _g_LRUC_DEBUG__ 0
#define _g_LRUC_INFO__ 0

#include "lruc.h"
#include "./compat/sys/queue.h"

struct lruc_node_st{
#ifdef _g_LRUC_DEBUG_
    lruc_t lru;
#endif
    TAILQ_ENTRY(lruc_node_st) bucket_link;
    TAILQ_ENTRY(lruc_node_st) queue_link;

    struct lruc_node_list_st* bucket_head;

#ifdef _g_LRUC_DEBUG_
    struct lruc_margin_str margin;
#endif
};


//--------------------------------------------
//LRU_NODE_LIST
TAILQ_HEAD(lruc_node_list_st, lruc_node_st);

typedef struct lruc_node_list_st *lruc_node_list_t;


//--------------------------------------------
//LRU
struct lruc_st{
#ifdef _g_LRUC_INFO__
    struct lruc_info_st info;
#endif

#ifdef _g_LRUC_DEBUG_
    int in_walk;
#endif
    void* cookie;

    unsigned int nsize;
    unsigned int ksize;
    unsigned int vsize;

    unsigned int koffset;
    unsigned int voffset;

    unsigned int bsize;
    unsigned int max;

    unsigned int count;

    hash_f* hash;
    comp_f* comp;
    destory_f* destory;

    struct lruc_node_list_st* bucket;
    struct lruc_node_list_st fifo;
    
    lruc_alloc_t alloc;

#ifdef _g_LRUC_DEBUG_
    struct lruc_margin_str margin;
#endif
};

//--------------------------------------------
//LRU_NODE_ACCESS_MACRO
#define G_LRUC_KEY(lru, node, type) \
    ((type*)(((char*)node)+(lru->koffset))


#define G_LRUC_VALUE(lru, node, type) \
    ((type*)(((char*)node)+(lru->voffset)))


#define G_LRUC_COOKIE(lru, type) \
     ((type)(lru->cookie))


//--------------------------------------------
//LRU_DEBUG_MARGIN
#ifdef _g_LRUC_DEBUG_
#define DEBUG_MARGIN1 0xF0CA
#define DEBUG_MARGIN1 0x09E6

struct lruc_margin_str{
    int margin1;
    int margin2;
};

typedef struct lruc_margin_str* lruc_margin_t;

int lruc_margin_check(lruc_margin_t margin);
void lruc_margin_init(lruc_margin_t margin);
void lruc_debug_check(lruc_t lru);

#define G_LRUC_DEBUG_CHECK(lru) lruc_debug_check(lru)

#else

#endif


#endif //__g_LRUC_IMP_H__