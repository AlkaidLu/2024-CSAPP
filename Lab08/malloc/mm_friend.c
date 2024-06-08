/*
 * Simple, 32-bit and 64-bit clean allocator based on implicit free
 * lists, first-fit placement, and boundary tag coalescing, as described
 * in the CS:APP3e text. Blocks must be aligned to doubleword (8 byte)
 * boundaries. Minimum block size is 16 bytes.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#include "mm.h"
#include "memlib.h"

#define DEBUG
/* If you want debugging output, use the following macro.  When you hand
 * in, remove the #define DEBUG line. */
#ifdef DEBUG
# define DBG_PRINTF(...) printf(__VA_ARGS__)
# define CHECKHEAP(verbose) mm_checkheap(verbose)
#else
# define DBG_PRINTF(...)
# define CHECKHEAP(verbose)
#endif

/*
 * If NEXT_FIT defined use next fit search, else use first-fit search
 */
#define NEXT_FIT
#define ALIGNMENT 8
#define ALIGN(size) (((size)+(ALIGNMENT-1))&~0x7)

/* $begin mallocmacros */
/* Basic constants and macros */
#define WSIZE 4 /* Word and header/footer size (bytes) */            // line:vm:mm:beginconst
#define DSIZE 8                                                      /* Double word size (bytes) */
#define LIST_MAX 22
#define CHUNKSIZE (1 << LIST_MAX) /* Extend heap by this amount (bytes) */ // line:vm:mm:endconst



#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) < (y)? (x) : (y))
/* Pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc)) // line:vm:mm:pack

/* Read and write a word at address p */
#define GET(p) (*(unsigned int *)(p))              // line:vm:mm:get
#define PUT(p, val) (*(unsigned int *)(p) = (val)) // line:vm:mm:put

/* Read the size and allocated fields from address p */
#define GET_SIZE(p) (GET(p) & ~0x7) // line:vm:mm:getsize
#define GET_ALLOC(p) (GET(p) & 0x1) // line:vm:mm:getalloc

/* Given block ptr bp, compute address of its header and footer */
//bp指向有效载荷的开始处
#define HDRP(bp) ((char *)(bp)-WSIZE)                        // line:vm:mm:hdrp
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp))-2*DSIZE) // line:vm:mm:ftrp

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) (FTRP(bp)+WSIZE) // line:vm:mm:nextblkp
#define PREV_BLKP(bp) ((char *)(bp)-GET_SIZE(((char *)(bp)-DSIZE)))   // line:vm:mm:prevblkp
/* $end mallocmacros */

#define PRED_PTR(bp) ((char *)(bp))
#define SUCC_PTR(bp) ((char *)(bp) + WSIZE)
#define PRED(bp) (*(char **)(bp))
#define SUCC(bp) (*(char **)(SUCC_PTR(bp)))
#define SET_PTR(p, bp) (*(unsigned int *)(p) = (unsigned int)(bp))

/* Global variables */
static unsigned int *seg_listp = 0;
static char *heap_listp = 0; /* Pointer to first block */
#define GET_HEAD(num) ((unsigned int *)(long)(GET(seg_listp +num-5)))

/* Function prototypes for internal helper routines */
static void *extend_heap(size_t words);
static void place(void *bp, size_t asize);
static void *find_fit(size_t asize);
static void *coalesce(void *bp);
static void printblock(void *bp);
static void checkheap(int verbose);
static void checkblock(void *bp);
int get_num(size_t size);
void insert_node(void* bp);
void delete_node(void* bp);
team_t team = {
    /* Team name */
    "zly",
    /* First member's full name */
    "zly",
    /* First member's email address */
    "1403805367@qq.com",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/*
 * mm_init - Initialize the memory manager
 */
/* $begin mminit */
int mm_init(void)
{

    /* Create the initial empty heap */
    if ((heap_listp = mem_sbrk(4 * WSIZE)) == (void *)-1) // line:vm:mm:begininit
        return -1;
    int i;
    seg_listp=(unsigned int *)heap_listp;
    for(i=0;i<LIST_MAX-4;i++){
        PUT(seg_listp +i,NULL);
    }
    PUT(heap_listp+(LIST_MAX-4)*WSIZE, 0);                            /* Alignment padding */
    PUT(heap_listp + ((1+LIST_MAX-4) * WSIZE), PACK(DSIZE, 1)); /* Prologue header */
    PUT(heap_listp + ((2+LIST_MAX-4) * WSIZE), PACK(DSIZE, 1)); /* Prologue footer */
    PUT(heap_listp + ((3+LIST_MAX-4) * WSIZE), PACK(0, 1));     /* Epilogue header */
    heap_listp += ((3+LIST_MAX-4) * WSIZE);                     // line:vm:mm:endinit
    PUT(seg_listp+LIST_MAX-5,heap_listp);
    if(extend_heap(CHUNKSIZE/WSIZE) == NULL)
        return -1;
    return 0;
}

/*
 * mm_malloc - Allocate a block with at least size bytes of payload
 */
/* $begin mmmalloc */
void *mm_malloc(size_t size)
{
    size_t asize;      /* Adjusted block size */
    size_t extendsize; /* Amount to extend heap if no fit */
    char *bp=NULL;

    /* $end mmmalloc */
    if (heap_listp == 0)
    {
        mm_init();
    }
    /* $begin mmmalloc */
    /* Ignore spurious requests */
    if (size == 0)
        return NULL;

    /* Adjust block size to include overhead and alignment reqs. */
    /* 最小块的大小是2^5=32字节：8字节用于满足对齐要求，8字节用于放头部和脚部 ,8字节用于前驱后继,再加8个字节为了满足块大小为2的指数倍*/
    if (size <= 2 * DSIZE)     // line:vm:mm:sizeadjust1
        asize = 4 * DSIZE; // line:vm:mm:sizeadjust2
    /* 超过8字节的请求：加上开销字节，向上舍入到最接近8的整数倍 */
    else
        asize = ALIGN(size)+2*DSIZE; // line:vm:mm:sizeadjust3

    /* Search the free list for a fit */
    if ((bp = find_fit(asize)) != NULL)
    {                     // line:vm:mm:findfitcall
        place(bp, asize); // line:vm:mm:findfitplace
        return bp;
    }

    /* No fit found. Get more memory and place the block */
    extendsize=CHUNKSIZE;
    while(extendsize<asize){
        extendsize*=2;
    }
    if ((bp = extend_heap(extendsize / WSIZE)) == NULL)
        return NULL;  // line:vm:mm:growheap2
    place(bp, asize); // line:vm:mm:growheap3
    return bp;
}
/* $end mmmalloc */

/*
 * mm_free - Free a block
 */
/* $begin mmfree */
void mm_free(void *bp)
{
    /* $end mmfree */
    if (bp == 0)
        return;

    /* $begin mmfree */
    size_t size = GET_SIZE(HDRP(bp));
    /* $end mmfree */
    if (heap_listp == 0)
    {
        mm_init();
    }
    /* $begin mmfree */

    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    while(bp=coalesce(bp)!=NULL){
    }
     // 合并
}

/* $end mmfree */
/*
 * coalesce - Boundary tag coalescing. Return ptr to coalesced block
 */
/* $begin coalesce */
static void *coalesce(void *bp)
{
    if(bp==heap_listp){
        return bp;
    }
    size_t size = GET_SIZE(HDRP(bp));
    int i = get_num(size);
    size_t prev_alloc = (!GET_ALLOC(FTRP(PREV_BLKP(bp))) &&
                    (GET_SIZE(FTRP(PREV_BLKP(bp))) == size) && 
                    (size_t)PREV_BLKP(bp) >> (i + 1));
    size_t next_alloc = (!GET_ALLOC(HDRP(NEXT_BLKP(bp))) &&
                    (GET_SIZE(FTRP(NEXT_BLKP(bp))) == size) && 
                    (size_t)bp >> (i + 1));
   
    if (prev_alloc && !next_alloc)
    { /* 可以和前一个合并 */
        delete_node(PREV_BLKP(bp));
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
        
    }

    else if (!prev_alloc && next_alloc)
    { /*能和下一个合并 */
        delete_node(NEXT_BLKP(bp));
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }
    else
    { /* 都不能合并*/
        return NULL;
    }
    /* $end mmfree */
    /* $begin mmfree */
    insert_node(bp);
    return bp;
}
/* $end coalesce */

/*
 * mm_realloc - Naive implementation of realloc
 */
void *mm_realloc(void *ptr, size_t size)
{
    size_t oldsize;
    void *newptr;

    /* If size == 0 then this is just free, and we return NULL. */
    if (size == 0)
    {
        mm_free(ptr);
        return 0;
    }

    /* If oldptr is NULL, then this is just malloc. */
    if (ptr == NULL)
    {
        return mm_malloc(size);
    }

    newptr = mm_malloc(size);

    /* If realloc() fails the original block is left untouched  */
    if (!newptr)
    {
        return 0;
    }

    /* Copy the old data. */
    oldsize = GET_SIZE(HDRP(ptr));
    if (size < oldsize)
        oldsize = size;
    memcpy(newptr, ptr, oldsize);

    /* Free the old block. */
    mm_free(ptr);

    return newptr;
}

/*
 * mm_checkheap - Check the heap for correctness
 */
void mm_checkheap(int verbose)
{
    checkheap(verbose);
}

/*
 * The remaining routines are internal helper routines
 */

/*
 * extend_heap - Extend heap with free block and return its block pointer
 */
/* $begin mmextendheap */
static void *extend_heap(size_t words)
{
    char *bp;
    size_t size;

    /* Allocate an even number of words to maintain alignment */
    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE; // line:vm:mm:beginextend
    
    if ((long)(bp = mem_sbrk(size)) == -1)
        return NULL; // line:vm:mm:endextend

    /* Initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, 0)); /* Free block header */           // line:vm:mm:freeblockhdr
    PUT(FTRP(bp), PACK(size, 0)); /* Free block footer */           // line:vm:mm:freeblockftr
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); /* New epilogue header */ // line:vm:mm:newepihdr

    return bp;
}
/* $end mmextendheap */

/*
 * place - Place block of asize bytes at start of free block bp
 *         and split if remainder would be at least minimum block size
 */
/* $begin mmplace */
/* $begin mmplace-proto */
static void place(void *bp, size_t asize)
/* $end mmplace-proto */
{
    size_t csize = GET_SIZE(HDRP(bp));
    delete_node(bp);
    if (csize >= 2* asize)
    {
        while(csize >= 2* asize){
            PUT(HDRP(bp), PACK(csize/2, 1));
            PUT(FTRP(bp), PACK(csize/2, 1));
            bp = NEXT_BLKP(bp);
            PUT(HDRP(bp), PACK(csize/2, 0));
            PUT(FTRP(bp), PACK(csize/2, 0));
            insert_node(bp);
            csize/=2;
        }
    }
    else
    {
        PUT(HDRP(bp), PACK(csize, 1));
        PUT(FTRP(bp), PACK(csize, 1));
    }
}
/* $end mmplace */
int get_num(size_t size){
    int i;
    for(i = 5; i <=LIST_MAX; i++){
        if(size <= (1 << i))
            return i;
    }
    return i;
}
/*
 * find_fit - Find a fit for a block with asize bytes
 */
/* $begin mmfirstfit */
/* $begin mmfirstfit-proto */
static void *find_fit(size_t asize) {
    int num = get_num(asize);
    unsigned int *bp;
    for (int i = num; i <= LIST_MAX; i++) {
        if (GET_HEAD(i)) {
            bp = GET_HEAD(i);
            return (void *)bp; // 返回找到的内存块指针
        }
    }
    return NULL; // 如果没有找到合适的内存块，则返回NULL
}


void insert_node(void* bp){
    size_t size=GET_SIZE(HDRP(bp));
    int num=get_num(size);
    if(GET_HEAD(num)==NULL){
        fprintf(stderr, "GET_HEAD_PTR(%d) returned NULL\n", num);
        exit(1);
    }
    if(*GET_HEAD(num)==NULL){
        SET_PTR(GET_HEAD(num),bp);
        SET_PTR(PRED(bp),NULL);
        SET_PTR(SUCC(bp),NULL);
    }
    else{
        SET_PTR(SUCC(bp),GET_HEAD(num));
        SET_PTR(GET_HEAD(num),bp);
        SET_PTR(PRED(bp),NULL);
    }

}
/* $end mmfirstfit */
void delete_node(void *bp){
    size_t size=GET_SIZE(HDRP(bp));
    int num=get_num(size);
    if(SUCC(bp)==NULL&&PRED(bp)==NULL){
        SET_PTR(GET_HEAD(num),NULL);
    }
    else{
        SET_PTR(GET_HEAD(num),SUCC(bp));
        SET_PTR(PRED(SUCC(bp)),NULL);
    }
}








