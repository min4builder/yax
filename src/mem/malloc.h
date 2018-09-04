#ifndef _LIBALLOC_H
#define _LIBALLOC_H

#include "lock.h"
#include "mem/virt.h"

/** \defgroup ALLOCHOOKS liballoc hooks 
 *
 * These are the OS specific functions which need to 
 * be implemented on any platform that the library
 * is expected to work on.
 */

/** @{ */

/* This lets you prefix malloc and friends */
#define PREFIX(func)		func

#ifdef __cplusplus
extern "C" {
#endif

extern Lock liballoc_lock;

/** This function is supposed to lock the memory data structures. It
 * could be as simple as disabling interrupts or acquiring a spinlock.
 * It's up to you to decide. 
 *
 * \return 0 if the lock was acquired successfully. Anything else is
 * failure.
 */
#define liballoc_lock() locklock(&liballoc_lock)

/** This function unlocks what was previously locked by the liballoc_lock
 * function.  If it disabled interrupts, it enables interrupts. If it
 * had acquiried a spinlock, it releases the spinlock. etc.
 *
 * \return 0 if the lock was successfully released.
 */
#define liballoc_unlock() lockunlock(&liballoc_lock)

/** This is the hook into the local system which allocates pages. It
 * accepts an integer parameter which is the number of pages
 * required.  The page size was set up in the liballoc_init function.
 *
 * \return NULL if the pages were not allocated.
 * \return A pointer to the allocated memory.
 */
#define liballoc_alloc(n) vpgkmap((n) * l_pageSize, PROT_READ | PROT_WRITE)

/** This frees previously allocated memory. The void* parameter passed
 * to the function is the exact same value returned from a previous
 * liballoc_alloc call.
 *
 * The integer value is the number of pages to free.
 *
 * \return 0 if the memory was successfully freed.
 */
#define liballoc_free(p, l) vpgunmap((p), (l) * l_pageSize)

#define l_pageSize 4096
       

extern void    *PREFIX(malloc)(size_t);				/* The standard function. */
extern void    *PREFIX(realloc)(void *, size_t);		/* The standard function. */
extern void    *PREFIX(calloc)(size_t, size_t);		/* The standard function. */
extern void     PREFIX(free)(void *);					/* The standard function. */


#ifdef __cplusplus
}
#endif


/** @} */

#endif


