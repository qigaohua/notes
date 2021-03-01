#include <stdint.h>
#include <linux/types.h>

/*
 * 获取CPU指令周期数,可以用来测试程序性能
 */
uint64_t rte_rdtsc(void)
{
    union {
        uint64_t tsc_64;
        struct {
            uint32_t lo_32;
            uint32_t hi_32;
        };
    } tsc;

    asm volatile("rdtsc" :
            "=a" (tsc.lo_32),
            "=d" (tsc.hi_32));

    return tsc.tsc_64;
}
