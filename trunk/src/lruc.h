#ifndef __g_LRUC_H__
#define __g_LRUC_H__ 

#define _g_LRUC_DEBUG__ 0
#define _g_LRUC_INFO__ 0

//#include "./compat/sys/queue.h"


//
struct lruc_node_st;
typedef struct lruc_node_st *lruc_node_t;


struct lruc_st;
typedef struct lruc_st* lruc_t;

typedef unsigned int (hash_f)(void*);
typedef void (destory_f)(struct lruc_node_st *node);
typedef int (comp_f)(void *arg1, void *arg2);

typedef void* (alloc_f)(void *context, unsigned int size);
typedef void (free_f)(void *context, void *);

//
//--------------------------------------------
#define LRU_IT_NEXT 0x0
#define LRU_IT_BREAK 0x1

#define LRU_IT_DEL 0x10
typedef int (walkcb_f)(struct lruc_node_st *node, void* key, void* value);


//--------------------------------------------
//LRU_INFO
//
#ifdef _g_LRUC_INFO__
struct lruc_info_st{
    int hit;
    int count;
};

typedef struct lruc_info_st *lruc_info_t;
#endif

//--------------------------------------------
//LRU_ALLOC
struct lruc_alloc_st{
    void* context;
    alloc_f* alloc;
    free_f* free;
};

typedef struct lruc_alloc_st *lruc_alloc_t;


//
//--------------------------------------------
lruc_t lruc_new(lruc_alloc_t alloc, 
        hash_f *hash, comp_f *comp, 
        destory_f *destory, 
        unsigned int ksize, unsigned int vsize, 
        unsigned int bsize, unsigned int max);

void lruc_free(lruc_t lru);

//options
void lruc_set_cookie(lruc_t lru, void* cookie);
void lruc_set_max_size(lruc_t lru, int max);

//create a new node in heap
lruc_node_t lruc_alloc_node(lruc_t lru);

//which never insert into lru
void lruc_free_node(lruc_t lru, lruc_node_t node);

//iterate
lruc_node_t lrucwalk(lruc_t lru, walkcb_f* walk);

//add a node into lru
int lruc_insert(lruc_t lru, void* key, void*  value);
int lruc_insert_node(lruc_t lru, lruc_node_t node);

//find in lru
lruc_node_t lruc_find(lruc_t lru, void* key);

//should not be call in walk callback
void lruc_del(lruc_t lru, void* key);
void lruc_del_node(lruc_t lru, lruc_node_t node);

#ifdef _g_LRUC_INFO__
//
lruc_info_t lruc_info(lruc_t lru);
#endif

#endif //__g_LRUC_H__
