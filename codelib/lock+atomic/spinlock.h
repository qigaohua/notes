#ifndef _SPINLOCK_H_
#define _SPINLOCK_H_

/**
 * @file
 *
 * Spinlocks
 *
 * This file defines an API for read-write locks, which are implemented
 * in an architecture-specific way. This kind of lock simply waits in
 * a loop repeatedly checking until the lock becomes available.
 *
 * All locks must be initialised before use, and only initialised once.
 *
 */

#include <sys/syscall.h>
#include <unistd.h>
#include <emmintrin.h>


static inline void pause_r(void)
{
#ifdef __x86_64__
    _mm_pause();

#elif __aarch64__
    asm volatile("yield" ::: "memory");
#endif

}


/**
 * The spinlock_t type.
 */
typedef struct {
    volatile int locked; /**< lock status 0 = unlocked, 1 = locked */
} spinlock_t;

/**
 * A static spinlock initializer.
 */
#define SPINLOCK_INITIALIZER { 0 }

/**
 * Initialize the spinlock to an unlocked state.
 *
 * @param sl
 *   A pointer to the spinlock.
 */
static inline void
spinlock_init(spinlock_t *sl)
{
    sl->locked = 0;
}

/**
 * Take the spinlock.
 *
 * @param sl
 *   A pointer to the spinlock.
 */
static inline void
spinlock_lock(spinlock_t *sl);

static inline void
spinlock_lock(spinlock_t *sl)
{
    while (__sync_lock_test_and_set(&sl->locked, 1))
        while(sl->locked)
            pause_r();
}

/**
 * Release the spinlock.
 *
 * @param sl
 *   A pointer to the spinlock.
 */
static inline void
spinlock_unlock (spinlock_t *sl);

static inline void
spinlock_unlock (spinlock_t *sl)
{
    __sync_lock_release(&sl->locked);
}

/**
 * Try to take the lock.
 *
 * @param sl
 *   A pointer to the spinlock.
 * @return
 *   1 if the lock is successfully taken; 0 otherwise.
 */
static inline int
spinlock_trylock (spinlock_t *sl);

static inline int
spinlock_trylock (spinlock_t *sl)
{
    return __sync_lock_test_and_set(&sl->locked,1) == 0;
}

/**
 * Test if the lock is taken.
 *
 * @param sl
 *   A pointer to the spinlock.
 * @return
 *   1 if the lock is currently taken; 0 otherwise.
 */
static inline int 
spinlock_is_locked (spinlock_t *sl)
{
    return sl->locked;
}

/**
 * Test if hardware transactional memory (lock elision) is supported
 *
 * @return
 *   1 if the hardware transactional memory is supported; 0 otherwise.
 */
static inline int tm_supported(void);

/**
 * Try to execute critical section in a hardware memory transaction,
 * if it fails or not available take the spinlock.
 *
 * NOTE: An attempt to perform a HW I/O operation inside a hardware memory
 * transaction always aborts the transaction since the CPU is not able to
 * roll-back should the transaction fail. Therefore, hardware transactional
 * locks are not advised to be used around rte_eth_rx_burst() and
 * rte_eth_tx_burst() calls.
 *
 * @param sl
 *   A pointer to the spinlock.
 */
static inline void
spinlock_lock_tm(spinlock_t *sl);

/**
 * Commit hardware memory transaction or release the spinlock if
 * the spinlock is used as a fall-back
 *
 * @param sl
 *   A pointer to the spinlock.
 */
static inline void
spinlock_unlock_tm(spinlock_t *sl);

/**
 * Try to execute critical section in a hardware memory transaction,
 * if it fails or not available try to take the lock.
 *
 * NOTE: An attempt to perform a HW I/O operation inside a hardware memory
 * transaction always aborts the transaction since the CPU is not able to
 * roll-back should the transaction fail. Therefore, hardware transactional
 * locks are not advised to be used around rte_eth_rx_burst() and
 * rte_eth_tx_burst() calls.
 *
 * @param sl
 *   A pointer to the spinlock.
 * @return
 *   1 if the hardware memory transaction is successfully started
 *   or lock is successfully taken; 0 otherwise.
 */
static inline int
spinlock_trylock_tm(spinlock_t *sl);

/**
 * The spinlock_recursive_t type.
 */
typedef struct {
    spinlock_t sl; /**< the actual spinlock */
    volatile int user; /**< core id using lock, -1 for unused */
    volatile int count; /**< count of time this lock has been called */
} spinlock_recursive_t;

/**
 * A static recursive spinlock initializer.
 */
#define SPINLOCK_RECURSIVE_INITIALIZER {SPINLOCK_INITIALIZER, -1, 0}

/**
 * Initialize the recursive spinlock to an unlocked state.
 *
 * @param slr
 *   A pointer to the recursive spinlock.
 */
static inline 
void spinlock_recursive_init(spinlock_recursive_t *slr)
{
    spinlock_init(&slr->sl);
    slr->user = -1;
    slr->count = 0;
}

/* require calling thread tid by gettid() */                                                    
int sys_gettid(void)                                                                        
{
    return (int)syscall(SYS_gettid);                                                            
}                                                                                               

/**
 * Take the recursive spinlock.
 *
 * @param slr
 *   A pointer to the recursive spinlock.
 */

static inline 
void spinlock_recursive_lock(spinlock_recursive_t *slr)
{
    int id = sys_gettid();

    if (slr->user != id) {
        spinlock_lock(&slr->sl);
        slr->user = id;
    }
    slr->count++;
}
/**
 * Release the recursive spinlock.
 *
 * @param slr
 *   A pointer to the recursive spinlock.
 */
static inline 
void spinlock_recursive_unlock(spinlock_recursive_t *slr)
{
    if (--(slr->count) == 0) {
        slr->user = -1;
        spinlock_unlock(&slr->sl);
    }

}

/**
 * Try to take the recursive lock.
 *
 * @param slr
 *   A pointer to the recursive spinlock.
 * @return
 *   1 if the lock is successfully taken; 0 otherwise.
 */
static inline 
int spinlock_recursive_trylock(spinlock_recursive_t *slr)
{
    int id = sys_gettid();

    if (slr->user != id) {
        if (spinlock_trylock(&slr->sl) == 0)
            return 0;
        slr->user = id;
    }
    slr->count++;
    return 1;
}


/**
 * Try to execute critical section in a hardware memory transaction,
 * if it fails or not available take the recursive spinlocks
 *
 * NOTE: An attempt to perform a HW I/O operation inside a hardware memory
 * transaction always aborts the transaction since the CPU is not able to
 * roll-back should the transaction fail. Therefore, hardware transactional
 * locks are not advised to be used around rte_eth_rx_burst() and
 * rte_eth_tx_burst() calls.
 *
 * @param slr
 *   A pointer to the recursive spinlock.
 */
static inline 
void spinlock_recursive_lock_tm(spinlock_recursive_t *slr);

/**
 * Commit hardware memory transaction or release the recursive spinlock
 * if the recursive spinlock is used as a fall-back
 *
 * @param slr
 *   A pointer to the recursive spinlock.
 */
static inline 
void spinlock_recursive_unlock_tm(spinlock_recursive_t *slr);

/**
 * Try to execute critical section in a hardware memory transaction,
 * if it fails or not available try to take the recursive lock
 *
 * NOTE: An attempt to perform a HW I/O operation inside a hardware memory
 * transaction always aborts the transaction since the CPU is not able to
 * roll-back should the transaction fail. Therefore, hardware transactional
 * locks are not advised to be used around rte_eth_rx_burst() and
 * rte_eth_tx_burst() calls.
 *
 * @param slr
 *   A pointer to the recursive spinlock.
 * @return
 *   1 if the hardware memory transaction is successfully started
 *   or lock is successfully taken; 0 otherwise.
 */
static inline 
int spinlock_recursive_trylock_tm(spinlock_recursive_t *slr);

#endif /* _SPINLOCK_H_ */
