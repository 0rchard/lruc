#ifndef __g_LRUC_H__
#define __g_LRUC_H__ 

#define _g_LRUC_DEBUG__ 0
#define _g_LRUC_INFO__ 0

//#include "./compat/sys/queue.h"


//
struct lru_node_st;
typedef struct lru_node_st *lru_node_t;

struct lru_st;
typedef struct lru_st* lru_t;

typedef unsigned int (hash_f)(void*);
typedef void (destory_f)(struct lru_node_st *node);
typedef int (comp_f)(void *arg1, void *arg2);

typedef void* (alloc_f)(void *context, unsigned int size);
typedef void (free_f)(void *context, void *);

//
//--------------------------------------------
#define LRU_IT_NEXT 0x0
#define LRU_IT_BREAK 0x1

#define LRU_IT_DEL 0x10
typedef int (walkcb_f)(struct lru_node_st *node, void* key, void* value);


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


//
//--------------------------------------------
lru_t lruc_new(lru_alloc_t alloc, 
        hash_f *hash, comp_f *comp, 
        destory_f *destory, 
        unsigned int ksize, unsigned int vsize, 
        unsigned int bsize, unsigned int max);

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

#endif //__g_LRUC_H__
