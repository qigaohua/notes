/**
 * @file sem.c
 * @brief 信号量，用于进程之间的同步
 * @author qigaohua, qigaohua168@163.com
 * @version 0.1
 * @date 2020-12-17
 */


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>


#define logw(fmt, ...) \
    fprintf(stderr, "%s:%d "fmt"\r\n", __FILE__, __LINE__, ##__VA_ARGS__);

#define CHECK_ARGS(a, r) \
    if (a) {  \
        logw("Error value of arguments"); \
        return r; \
    }


union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *arry;
};

int ipc_semget(key_t key, int num_sems, int sem_flags)
{
    int semid;

    /*
     * arg 2: 指定需要的信号量数目，它的值几乎总是1
     * arg 3: sem_flags是一组标志，当想要当信号量不存在时创建一个新的信号量，
     *        可以和值IPC_CREAT做按位或操作。设置了IPC_CREAT标志后，即使给出
     *        的键是一个已有信号量的键，也不会产生错误。而IPC_CREAT | IPC_EXCL
     *        则可以创建一个新的，唯一的信号量，如果信号量已存在，返回一个错误
     */
    semid = semget(key, num_sems, sem_flags);
    if (-1 == semid) {
        logw("Call semget() failed: %s", strerror(errno));
        return -1;
    }

    return semid;
}


// 用于初始化信号量，在使用信号量前必须这样做
int ipc_set_semvalue(int semid, int value)
{
    union semun sem_union;

    sem_union.val = value; // 通常是1
    if(semctl(semid, 0, SETVAL, sem_union) == -1) {
        logw("Call semctl() failed: %s", strerror(errno));
        return -1;
    }

    return 0;
}


//删除信号量
void ipc_semdel(int semid)
{
    /*
     * arg 2: 信号量元素序号(数组坐标)，通常只有1个信号量，这里值为0
     * arg 3: SETVAL：用来把信号量初始化为一个已知的值。这个值通过
     *        union semun中的val成员设置，其作用是在信号量第一次使
     *        用前对它进行设置，见上个函数。
     *        IPC_RMID：用于删除一个已经无需继续使用的信号量标识符
     */
    if(semctl(semid, 0, IPC_RMID) == -1) {
        logw("Call semctl() failed: %s", strerror(errno));
    }
}


//对信号量做减1操作，即等待P（sv）
int ipc_sem_p(int semid)
{
    struct sembuf sem_b;

    sem_b.sem_num = 0;
    sem_b.sem_op = -1;  //P()
    sem_b.sem_flg = SEM_UNDO;

    /*
     * arg 2:
     *      struct sembuf{
     *          // 除非使用一组信号量，否则它为0，一般从0,1,...num_secs-1
     *          short sem_num;
     *          // 信号量在一次操作中需要改变的数据，通常是两个数，一个是-1，即P（等待）操作，
     *          // 一个是+1，即V（发送信号）操作。
     *          short sem_op;
     *          // 通常为SEM_UNDO,使操作系统跟踪信号，
     *          // 并在进程没有释放该信号量而终止时，操作系统释放信号量
     *          short sem_flg;
     *     };
     * arg 3: 信号量操作结构体个数，默认为1
     */

    if(semop(semid, &sem_b, 1) == -1) {
        logw("Call semop() failed: %s", strerror(errno));
        return -1;
    }

    return 0;
}


//这是一个释放操作，它使信号量变为可用，即发送信号V（sv）
int ipc_sem_v(int semid)
{
    struct sembuf sem_b;

    sem_b.sem_num = 0;
    sem_b.sem_op = 1;//V()
    sem_b.sem_flg = SEM_UNDO;
    if(semop(semid, &sem_b, 1) == -1) {
        logw("Call semop() failed: %s", strerror(errno));
        return -1;
    }

    return 0;
}

