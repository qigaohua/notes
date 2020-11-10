#ifndef __BLOOM_FILTER_H_
#define __BLOOM_FILTER_H_


#ifndef __KERNEL__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>


#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

typedef unsigned int UINT;

typedef int INT;
ypedef char CHAR;
typedef unsigned char BYTE;
typedef short SHORT;
typedef unsigned short USHORT;
typedef long long INT64;
typedef unsigned long long UINT64;

#define BL_CALLOC(n) calloc(1, n)
#define BL_FREE(n) free(n)

#endif

#pragma pack(push, 4)

typedef struct __bloom__{
    UINT64 capacity;/**/
    UINT64 bits_num; /* number of bits in bitmap*/
    UINT64 k_num; /* number of hash functions*/
    UINT64 counts_per_func;

    UINT64 *hashes;

    UINT64 use_bytes;
    UINT64 item_count;

    /*bitmap data follows the head*/
    BYTE bitmap[0];
} zxfw_bloom_t;

#pragma pack(pop)

/*
* MurmurHash3 was written by Austin Appleby, and is placed in the public
* domain. The author hereby disclaims copyright to this source code.

* Note - The x86 and x64 versions do _not_ produce the same results, as the
* algorithms are optimized for their respective platforms. You can still
* compile and run any of them on any platform, but your performance with the
* non-native version will be less than optimal.
*/

#define FORCE_INLINE inline static
#define SALT_CONSTANT 0x97c29b3a

FORCE_INLINE UINT64 rotl64 (UINT64 x, CHAR r)
{
    return (x << r) | (x >> (64 - r));
}

#define ROTL64(x,y) rotl64(x,y)

#define BIG_CONSTANT(x) (x##LLU)

#define getblock(x, i) (x[i])

//-----------------------------------------------------------------------------
// Finalization mix - force all bits of a hash block to avalanche

FORCE_INLINE UINT64 fmix64(UINT64 k)
{
    k ^= k >> 33;
    k *= BIG_CONSTANT(0xff51afd7ed558ccd);
    k ^= k >> 33;
    k *= BIG_CONSTANT(0xc4ceb9fe1a85ec53);
    k ^= k >> 33;

    return k;
}

//-----------------------------------------------------------------------------

static void MurmurHash3_x64_128 ( const void * key, const INT len,
        const UINT64 seed, void * out )
{
    const BYTE * tail;
    UINT64 k1;
    UINT64 k2;
    const BYTE * data = (const BYTE*)key;
    const INT nblocks = len / 16;

    UINT64 h1 = seed;
    UINT64 h2 = seed;

    UINT64 c1 = BIG_CONSTANT(0x87c37b91114253d5);
    UINT64 c2 = BIG_CONSTANT(0x4cf5ad432745937f);

    INT i;

    //----------
    // body

    const UINT64 * blocks = (const UINT64 *)(data);

    for(i = 0; i < nblocks; i++) {
        UINT64 k1 = getblock(blocks,i*2+0);
        UINT64 k2 = getblock(blocks,i*2+1);

        k1 *= c1; k1  = ROTL64(k1,31); k1 *= c2; h1 ^= k1;

        h1 = ROTL64(h1,27); h1 += h2; h1 = h1*5+0x52dce729;

        k2 *= c2; k2  = ROTL64(k2,33); k2 *= c1; h2 ^= k2;

        h2 = ROTL64(h2,31); h2 += h1; h2 = h2*5+0x38495ab5;
    }

    //----------
    // tail

    tail = (const BYTE*)(data + nblocks*16);

    k1 = 0;
    k2 = 0;

    switch(len & 15) {
        case 15: k2 ^= ((UINT64)tail[14]) << 48;
        case 14: k2 ^= ((UINT64)tail[13]) << 40;
        case 13: k2 ^= ((UINT64)tail[12]) << 32;
        case 12: k2 ^= ((UINT64)tail[11]) << 24;
        case 11: k2 ^= ((UINT64)tail[10]) << 16;
        case 10: k2 ^= ((UINT64)tail[ 9]) << 8;
        case  9: k2 ^= ((UINT64)tail[ 8]) << 0;
                 k2 *= c2; k2  = ROTL64(k2,33); k2 *= c1; h2 ^= k2;

        case  8: k1 ^= ((UINT64)tail[ 7]) << 56;
        case  7: k1 ^= ((UINT64)tail[ 6]) << 48;
        case  6: k1 ^= ((UINT64)tail[ 5]) << 40;
        case  5: k1 ^= ((UINT64)tail[ 4]) << 32;
        case  4: k1 ^= ((UINT64)tail[ 3]) << 24;
        case  3: k1 ^= ((UINT64)tail[ 2]) << 16;
        case  2: k1 ^= ((UINT64)tail[ 1]) << 8;
        case  1: k1 ^= ((UINT64)tail[ 0]) << 0;
                 k1 *= c1; k1  = ROTL64(k1,31); k1 *= c2; h1 ^= k1;
    }

    //----------
    // finalization

    h1 ^= len; h2 ^= len;

    h1 += h2;
    h2 += h1;

    h1 = fmix64(h1);
    h2 = fmix64(h2);

    h1 += h2;
    h2 += h1;

    ((UINT64*)out)[0] = h1;
    ((UINT64*)out)[1] = h2;
}

//-----------------------------------------------------------------------------
/*
 * Perform the actual hashing for `key`
 *
 * Only call the hash once to get a pair of initial values (h1 and
 * h2). Use these values to generate all hashes in a quick loop.
 *
 * See paper by Kirsch, Mitzenmacher [2006]
 * http://www.eecs.harvard.edu/~michaelm/postscripts/rsa2008.pdf
 */

static void hash_func(zxfw_bloom_t *bf, const CHAR *key, UINT len, UINT64 *hashes)
{
    INT i;
    UINT64 h1;
    UINT64 h2;
    UINT64 checksum[4];

    MurmurHash3_x64_128(key, len, SALT_CONSTANT, checksum);
    h1 = checksum[0];
    h2 = checksum[1];

    for (i = 0; i < bf->k_num; i++)
    {
        hashes[i] = (h1 + i * h2) % bf->counts_per_func;
    }
}

static inline INT bitmap_getbit(BYTE *bitmap, UINT64 idx)
{
    return bitmap[idx >> 3] & 1 << (idx % 8);
}

/*
 * Used to set a bit in the bitmap, and as a side affect,
 * mark the page as dirty if we are in the PERSISTENT mode
 */
static inline void bitmap_setbit(BYTE *bitmap, UINT64 idx)
{
    BYTE byte = bitmap[idx >> 3];
    byte |=  1 << (idx % 8);
    bitmap[idx >> 3] = byte;
}

#ifndef __KERNEL__

/*
 * capacity: 容量
 * error_rate: 允许的错误率
 * 分析url: http://blog.csdn.net/gdutliuyun827/article/details/17377349
 * 公式根据: http://pages.cs.wisc.edu/~cao/papers/summary-cache/node8.html，给出了:
 * *1. 根据位数组大小m，加入的url个数n，计算出出错概率最小时的哈希函数个数k
 * *2. 根据哈希函数个数k，位数组大小m，url个数n，计算出错误率
 * *3. 逆推公式(比如这里的根据url个数capacity, 错误率error_rate，确定各种大小)
 */
FORCE_INLINE zxfw_bloom_t *zxfw_bloom_filter_new(UINT64 capacity, double error_rate)
{
    zxfw_bloom_t bf_tmp = {0}, *bf;

    bf_tmp.capacity = capacity;
	// 根据错误率确定哈希函数个数
    bf_tmp.k_num = (UINT64)ceil(log(1 / error_rate) / log(2));
	// 每个哈希函数会占的位数，即位数组被平均分段了k_num份，每份的大小counts_per_func，
	// 每个哈希函数只在自己范围内操作位
    bf_tmp.counts_per_func = (UINT64)ceil(capacity * fabs(log(error_rate)) / (bf_tmp.k_num * pow(log(2), 2)));
	// 占用的总位数
    bf_tmp.bits_num = bf_tmp.k_num * bf_tmp.counts_per_func;
	// 位数组大小，转字节大小，因为是字节数组存的，取上(+1)
    bf_tmp.use_bytes = bf_tmp.bits_num / 8 + 1;
	// 不是k_num个哈希函数指针，哈希函数确定的为MurmurHash3_x64_128，
	// hashes只是用来存每次各哈希函数结果
    bf_tmp.hashes = (UINT64 *)BL_CALLOC(bf_tmp.k_num * sizeof(UINT64));


	// 动态的位数组大小，至于bf_tmp.hashes动态部分，因为上面已经calloc过了
	// 所以可以看做只是zxfw_bloom_t的一个成员变量即可，在下面memcpy中拷贝了过去
	// 除此外，位数组也可以像hashes一样在上面calloc，这里直接作为成员变量在下面memcpy就行了
    if (NULL == (bf = (zxfw_bloom_t *)BL_CALLOC(bf_tmp.use_bytes + sizeof(zxfw_bloom_t))))
	{
		fprintf (stderr, "BL_CALLOC[%lluM] fail\n", (bf_tmp.use_bytes + sizeof(zxfw_bloom_t))>>20);
		exit (1) ;
	}

    // printf("memory size: %lluM\n", (bf_tmp.use_bytes + sizeof(zxfw_bloom_t)) / 1024 / 1024);

    memcpy(bf, &bf_tmp, sizeof(zxfw_bloom_t));

    return bf;
}
#endif

FORCE_INLINE INT zxfw_bloom_filter_save(zxfw_bloom_t *bf, const CHAR *filename)
{
	FILE *fp = fopen(filename, "w") ;
	if (!fp)	return -1 ;

	fwrite (bf, bf->use_bytes+sizeof(zxfw_bloom_t), 1, fp) ;
	fclose (fp) ;

	return 1 ;
}

FORCE_INLINE zxfw_bloom_t *zxfw_bloom_filter_load(const CHAR *filename)
{
	FILE *fp = fopen(filename, "r") ;
	if (!fp)	return NULL ;

	// 获取文件长度
	fseek (fp, 0L, SEEK_END) ;
	long filesize = ftell(fp) ;
	fseek (fp, 0L, SEEK_SET) ;

    zxfw_bloom_t *bf = (zxfw_bloom_t *)BL_CALLOC(filesize) ;
	if (fread(bf, filesize, 1, fp)) ;
	fclose (fp) ;

    bf->hashes = (UINT64 *)BL_CALLOC(bf->k_num * sizeof(UINT64));

	return bf ;
}

FORCE_INLINE INT zxfw_bloom_filter_add(zxfw_bloom_t *bf, const CHAR *str, UINT len)
{
    UINT64 idx, i;
    UINT64 *hashes = bf->hashes;

	// 获取k_num个哈希函数MurmurHash3_x64_128结果
    hash_func(bf, str, len, hashes);

    for (i = 0; i < bf->k_num; i++)
	{
		// 换算为各个哈希函数所在段范围内
        idx = hashes[i] + i * bf->counts_per_func;
        bitmap_setbit(bf->bitmap, idx);
    }
    bf->item_count++;

    return 0;
}

/**
* @retval 1 : found
* @retval 0 : not found
*/
FORCE_INLINE INT __bloom_filter_search(zxfw_bloom_t *bf, UINT64 *hashes)
{
    INT i;
    UINT64 idx;

    for (i = 0; i < bf->k_num; i++)
    {
		// 换算为各个哈希函数所在段范围内
        idx = hashes[i] + i * bf->counts_per_func;
        if (0 == bitmap_getbit(bf->bitmap, idx))	return 0;
    }

	// 全部为1才返回找到
    return 1;
}

/**
* @
*
*/
FORCE_INLINE INT zxfw_bloom_filter_search(zxfw_bloom_t *bf, const CHAR *key, UINT key_len)
{
    UINT64 *hashes = bf->hashes;

    hash_func(bf, key, key_len, hashes);

    return __bloom_filter_search(bf, hashes);
}

/**
* Reset bloom_filter in order to add keys afressh, the capacity and error_rate remain the same.
*/
FORCE_INLINE void zxfw_bloom_filter_reset(zxfw_bloom_t *bf)
{
    INT i;

    for (i = 0; i < bf->use_bytes; i++)
        bf->bitmap[i] = 0;
    bf->item_count = 0;
}

FORCE_INLINE void zxfw_bloom_filter_destroy(zxfw_bloom_t *bf)
{
	if (bf->hashes)	BL_FREE(bf->hashes) ;
	// bf.bitmap不用单独free，因为他用的是bf的后面所有剩余内存，不是单独分配
    if (bf)	BL_FREE(bf);
}


#if 0
// test
int main()
{
	int ret;
        // 参数：
        // 库容量， 最大在库中添加多少条url
        // 允许的错误率
        //
        // 两个参数影响内存占用
	zxfw_bloom_t *bf = zxfw_bloom_filter_new(10000000, 0.000001);
        zxfw_bloom_filter_add(bf, "abcd", 4);
        zxfw_bloom_filter_add(bf, "12345678", 8);
        zxfw_bloom_filter_add(bf, "mnbvcx", 6);
        zxfw_bloom_filter_add(bf, "asdfghjkl", 9);
        zxfw_bloom_filter_add(bf, "1qbchdhfujmmmmmmma", 18);

	ret = zxfw_bloom_filter_search(bf, "mnbvcx", 6);
        printf("ret = %d\n", ret);


	ret = zxfw_bloom_filter_search(bf, "1222222", 7);
        printf("ret = %d\n", ret);


	zxfw_bloom_filter_destroy(bf);

	return 0;
}
#endif
#endif
