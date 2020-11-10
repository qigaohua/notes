/**
 * @file bitset.h
 * @brief  位组
 * @author qigaohua, qigaohua168@163.com
 * @version 0.0.1
 * @date 2020-06-10
 */


#include <limits.h>        /* for CHAR_BIT */

#define BITMASK(b) (1 << ((b) % CHAR_BIT))
#define BITSLOT(b) ((b) / CHAR_BIT)
#define BITSET(a, b) ((a)[BITSLOT(b)] |= BITMASK(b))
#define BITCLEAR(a, b) ((a)[BITSLOT(b)] &= ~BITMASK(b))
#define BITTEST(a, b) ((a)[BITSLOT(b)] & BITMASK(b))
#define BITNSLOTS(nb) ((nb + CHAR_BIT - 1) / CHAR_BIT)

/*************************************
 使用：

 声明一个固定长度（50个bit）的位数组：
 char bitarray[BITNSLOTS(50)];

 设置位数组中的某一位：
 BITSET(bitarray, 23);

 检测某一位
 if(BITTEST(bitarray, 35)) ...

 求两个位数组的并集
 for(i = 0; i < BITNSLOTS(47); i++)
     array3[i] = array1[i] | array2[i];

 求两个位数组的交集
 for(i = 0; i < BITNSLOTS(47); i++)
	array3[i] = array1[i] & array2[i];


 ************************************/
