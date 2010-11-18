#ifndef __g_LRUC_IMP_H__
#define __g_LRUC_IMP_H__ 

#define _g_LRUC_DEBUG__ 0
#define _g_LRUC_INFO__ 0

#include "./compat/sys/queue.h"

#define IT_NEXT 0x0
#define IT_BREAK 0x1

#define IT_DEL 0x10

//
struct lru_node_st;
typedef unsigned int (hash_f)(void*);
typedef void (destory_f)(struct lru_node_st* node);
typedef int (comp_f)(void* arg1, void* arg2);

typedef void* (alloc_f)(void* context, unsigned int size);
typedef void (free_f)(void* context, void*);

//
typedef int (walkcb_f)(struct lru_node_st* node);

//--------------------------------------------
//LRU_NODE
//
struct lru_node_st;
struct lru_node_list_st;
struct lru_node_st{
#ifdef _g_LRUC_DEBUG_
    lru_t lru;
#endif
    TAILQ_ENTRY(lru_node_st) bucket_link;
    TAILQ_ENTRY(lru_node_st) queue_link;

    struct lru_node_list_st* bucket_head;

#ifdef _g_LRUC_DEBUG_
    struct lru_margin_str margin;
#endif
};

typedef struct lru_node_st* lru_node_t;


//--------------------------------------------
//LRU_NODE_LIST
TAILQ_HEAD(lru_node_list_st, lru_node_st);

typedef struct lru_node_list_st *lru_node_list_t;

//--------------------------------------------
//LRU_INFO
//
#ifdef _g_LRUC_INFO__
struct lru_info_st{
    int hit;
    int count;
};

typedef struct lru_info_st* lru_info_t;
#endif

//--------------------------------------------
//LRU_ALLOC
struct lru_alloc_st{
    void* context;
    alloc_f* alloc;
    free_f* free;
};

typedef struct lru_alloc_st* lru_alloc_t;

//--------------------------------------------
//LRU
struct lru_st{
#ifdef _g_LRUC_INFO__
    struct lru_info_st info;
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

    struct lru_node_list_st* bucket;
    struct lru_node_list_st fifo;
    
    lru_alloc_t alloc;

#ifdef _g_LRUC_DEBUG_
    struct lru_margin_str margin;
#endif
};

typedef struct lru_st* lru_t;

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

struct lru_margin_str{
    int margin1;
    int margin2;
};

typedef struct lru_margin_str* lru_margin_t;

int lruc_margin_check(lru_margin_t margin);
void lruc_margin_init(lru_margin_t margin);
void lruc_debug_check(lru_t lru);

#define G_LRUC_DEBUG_CHECK(lru) lruc_debug_check(lru)

#else

#endif



//
lru_t lruc_new(lru_alloc_t alloc, hash_f *hash, comp_f *comp, destory_f *destory, 
        unsigned int ksize, unsigned int vsize, unsigned int bsize, unsigned int max);

void lruc_free(lru_t lru);

//options
void lruc_set_cookie(lru_t lru, void* cookie);
void lruc_set_max_size(lru_t lru, int max);

//create a new node in heap
lru_node_t lruc_alloc_node(lru_t lru);

//which never insert into lru
void lruc_free_node(lru_t lru, lru_node_t node);

//iterate
lru_node_t lru_walk(lru_t lru, walkcb_f* walk);

//add a node into lru
int lruc_insert(lru_t lru, void* key, void*  value);
int lruc_insert_node(lru_t lru, lru_node_t node);

//find in lru
lru_node_t lruc_find(lru_t lru, void* key);

//should not be call in walk callback
void lruc_del(lru_t lru, void* key);
void lruc_del_node(lru_t lru, lru_node_t node);

#ifdef _g_LRUC_INFO__
//
lru_info_t lru_info(lru_t lru);
#endif

#endif //__g_LRUC_IMP_H__
