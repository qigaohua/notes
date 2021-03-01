
 /*
  * MD5  openssl
  */

/*
// 初始化 MD5 Contex, 成功返回1,失败返回0
int MD5_Init(MD5_CTX *c);
// 循环调用此函数,可以将不同的数据加在一起计算MD5,成功返回1,失败返回0
int MD5_Update(MD5_CTX *c, const void *data, size_t len);
// 输出MD5结果数据,成功返回1,失败返回0
int MD5_Final(unsigned char *md, MD5_CTX *c);
// MD5_Init,MD5_Update,MD5_Final三个函数的组合,直接计算出MD5的值
unsigned char *MD5(const unsigned char *d, size_t n, unsigned char *md);
// 内部函数,不需要调用
void MD5_Transform(MD5_CTX *c, const unsigned char *b);

// gcc 编译加上-lcrypto *****
*/

#include <openssl/md5.h>
#include <string.h>
#include <stdio.h>

int md5()
{
#if 0
    MD5_CTX ctx;
    unsigned char outmd[16];
    int i = 0;

    memset(outmd,0,sizeof(outmd));
    MD5_Init(&ctx);
    // MD5_Update(&ctx,"hel",3);
    // MD5_Update(&ctx,"lo",2);
    MD5_Update(&ctx,"hello",5);
    MD5_Final(outmd,&ctx);
    for(; i < 16; i++)
        printf("%02X",outmd[i]);

    printf("\n");
    return 0;
#endif

     MD5_CTX ctx;
     unsigned char outmd[16];
     char buffer[1024];
     char filename[32];
     int len=0;
     int i;
     FILE * fp=NULL;
     memset(outmd,0,sizeof(outmd));
     memset(filename,0,sizeof(filename));
     memset(buffer,0,sizeof(buffer));
     printf("请输入文件名，用于计算MD5值:");
     scanf("%s",filename);
     fp=fopen(filename,"rb");
     if(fp==NULL) {
         printf("Can't open file\n");
         return 0;
     }

     MD5_Init(&ctx);
     while((len=fread(buffer,1,1024,fp))>0)
     {
         MD5_Update(&ctx,buffer,len);
         memset(buffer,0,sizeof(buffer));

     }
     MD5_Final(outmd,&ctx);

     for( i = 0; i < 16; i++)
         printf("%02X",outmd[i]);

     printf("\n");
     return 0;
}

/**************************************************************************/


/*
    //SHA1算法是对MD5算法的升级,计算结果为20字节(160位)，使用方法如下：

    //打开/usr/include/openssl/sha.h这个文件我们可以看到一些函数
    // 初始化 SHA Contex, 成功返回1,失败返回0
    int SHA_Init(SHA_CTX *c);
    // 循环调用此函数,可以将不同的数据加在一起计算SHA1,成功返回1,失败返回0
    int SHA_Update(SHA_CTX *c, const void *data, size_t len);
    // 输出SHA1结果数据,成功返回1,失败返回0
    int SHA_Final(unsigned char *md, SHA_CTX *c);
    // SHA_Init,SHA_Update,SHA_Final三个函数的组合,直接计算出SHA1的值
    unsigned char *SHA(const unsigned char *d, size_t n, unsigned char *md);
    // 内部函数,不需要调用
    void SHA_Transform(SHA_CTX *c, const unsigned char *data);

    //上面的SHA可以改为SHA1，SHA224，SHA256，SHA384，SHA512就可以实现多种加密了
*/

#include <openssl/sha.h>
int sha1()
{
     SHA_CTX ctx;
     unsigned char outmd[20];
     char buffer[1024];
     char filename[32];
     int len=0;
     int i;
     FILE * fp=NULL;
     memset(outmd,0,sizeof(outmd));
     memset(filename,0,sizeof(filename));
     memset(buffer,0,sizeof(buffer));
     printf("请输入文件名，用于计算SHA1值:");
     scanf("%s",filename);
     fp=fopen(filename,"rb");
     if(fp==NULL) {
         printf("Can't open file\n");
         return 0;
     }

     SHA1_Init(&ctx);
     while((len=fread(buffer,1,1024,fp))>0)
     {
         SHA1_Update(&ctx,buffer,len);
         memset(buffer,0,sizeof(buffer));
     }
     SHA1_Final(outmd,&ctx);

     for( i = 0; i < 20; i++)
         printf("%02X",outmd[i]);

     printf("\n");
     return 0;
}

/*************************************************************/

/*
 RSA 算法


 生成RSA的key，包括三部分：公钥指数、私钥指数和模数（这些需要先了解一下RSA算法的原理）

       将这三个数存下来，其中私钥指数和模数比较大，都是大素数
       构造RSA结构，可以构造公钥和私钥RSA结构
       利用构造的RSA结构的指针进行<公钥加密-私钥解密>或者<私钥加密-公钥解密>
       （注意加解密用到的密钥类型，不能用同一密钥进行加密和解密）


       RSA私钥产生函数
       RSA *RSA_generate_key(int num, unsigned long e,void (*callback)(int,int,void *), void *cb_arg);
       产生一个模为num位的密钥对，e为公开的加密指数，一般为65537（ox10001），
       假如后两个参数不为NULL，将有些调用。在产生密钥对之前，一般需要指定随机数种子

       加解密函数
       int RSA_public_encrypt (int flen, unsigned char *from, unsigned char *to, RSA *rsa, int padding);
       int RSA_private_decrypt(int flen, unsigned char *from, unsigned char *to, RSA *rsa, int padding);
       int RSA_private_encrypt(int flen, unsigned char *from, unsigned char *to, RSA *rsa, int padding);
       int RSA_public_decrypt (int flen, unsigned char *from, unsigned char *to, RSA *rsa, int padding);

       接口手册
       https://www.openssl.org/docs/man1.1.1/man3/
*/


#include <openssl/rsa.h>


#define RSA_KEY_LENGTH 1024
int generate_key_str()
{
    RSA *r = NULL;
    int bits = RSA_KEY_LENGTH;
    unsigned long e = RSA_F4;

    r = RSA_generate_key(bits, e, NULL, NULL);

    // 用作显示
    RSA_print_fp(stdout, r, 0);

    return 0;
}

// 模数
#define RSA_KEY_N			"A50BC427EF7A66E5222DC2B392F66EF156F78A96791A03A352132BEC26FB3E58"\
							"3CD795A85AE6DBEDEF71E3A18511F9E5B398CB76ACC4DFDEB7CEA20E06394DDC"\
							"FC1F8D82E692C95FD2BA09EC7EE8D1DB69B3506AFC4DF1FE1F45B0A8DC3BD968"\
							"AC1197955FD9FABEFC508A62C44E41830A846E67B54B0E406DA7C656E1A81481"
// 私钥指数
#define RSA_KEY_D			"290588A6A86612E7069CBB14E905294400EA9BDE9490FBB07F38D6A4E67771E2"\
							"7A94D6D4B66B44E499AC03F8E45B3872783614F4153305B59497DE3D801C8416"\
							"6FAD785F0F54CC22C978364405350A1D610172F90C30F4C6C475C549254AFC52"\
							"667A558C2ED198C9B634A0A0F49C3EDE05BA29293E75763DABAD7AE827FA6155"
// 公钥指数
#define RSA_KEY_E			"10001"

const unsigned char key_n[] = {
	0xa5, 0x0b, 0xc4, 0x27, 0xef, 0x7a, 0x66, 0xe5, 0x22, 0x2d, 0xc2, 0xb3, 0x92, 0xf6, 0x6e, 0xf1,
	0x56, 0xf7, 0x8a, 0x96, 0x79, 0x1a, 0x03, 0xa3, 0x52, 0x13, 0x2b, 0xec, 0x26, 0xfb, 0x3e, 0x58,
	0x3c, 0xd7, 0x95, 0xa8, 0x5a, 0xe6, 0xdb, 0xed, 0xef, 0x71, 0xe3, 0xa1, 0x85, 0x11, 0xf9, 0xe5,
	0xb3, 0x98, 0xcb, 0x76, 0xac, 0xc4, 0xdf, 0xde, 0xb7, 0xce, 0xa2, 0x0e, 0x06, 0x39, 0x4d, 0xdc,
	0xfc, 0x1f, 0x8d, 0x82, 0xe6, 0x92, 0xc9, 0x5f, 0xd2, 0xba, 0x09, 0xec, 0x7e, 0xe8, 0xd1, 0xdb,
	0x69, 0xb3, 0x50, 0x6a, 0xfc, 0x4d, 0xf1, 0xfe, 0x1f, 0x45, 0xb0, 0xa8, 0xdc, 0x3b, 0xd9, 0x68,
	0xac, 0x11, 0x97, 0x95, 0x5f, 0xd9, 0xfa, 0xbe, 0xfc, 0x50, 0x8a, 0x62, 0xc4, 0x4e, 0x41, 0x83,
	0x0a, 0x84, 0x6e, 0x67, 0xb5, 0x4b, 0x0e, 0x40, 0x6d, 0xa7, 0xc6, 0x56, 0xe1, 0xa8, 0x14, 0x81
};

const unsigned char key_e[] = { 0x01, 0x00, 0x01 };




int rsa_encrypt(const char *encrypt_str, char *result)
{
    int ret;
    RSA *rsa;
    unsigned char dst_buff[4096] = {0};

    // const char *str = "1234567890";
    if ( !encrypt_str || !result )
        return -1;

    rsa = RSA_new();

#if OPENSSL_VERSION_NUMBER < 0x10100000L
    /* OpenSSL 1.0.2 and below */
    BN_hex2bn(&rsa->n, RSA_KEY_N);
    BN_hex2bn(&rsa->d, RSA_KEY_D);
    BN_hex2bn(&rsa->e, RSA_KEY_E);
#else
    /* OpenSSL 1.1.0 and above */

    // RSA_set0_key(rsa, BN_bin2bn(key_n, sizeof key_n, NULL),
    //                   BN_bin2bn(key_e, sizeof key_e, NULL),
    //                   NULL);

    BIGNUM *n, *e, *d;
    BN_hex2bn(&n, RSA_KEY_N);
    BN_hex2bn(&e, RSA_KEY_E);
    BN_hex2bn(&d, RSA_KEY_D);
    RSA_set0_key(rsa, n, e, d);
#endif

    // 返回RSA模的位数，他用来判断需要给加密值分配空间的大小
    int dsize = RSA_size(rsa);
    ret = RSA_private_encrypt(strlen(encrypt_str), (unsigned char *)encrypt_str, dst_buff, rsa, RSA_PKCS1_PADDING);
    RSA_free(rsa);

    if (ret != dsize) {
        printf("RSA private encrypt failed.");
        return -2;
    }
    printf("dsize : %d\n", ret);

    int i = 0;
    for (; i < dsize; i++) {
        // printf("%02X", dst_buff[i]);
        snprintf(result + 2 * i, 4096, "%02X", dst_buff[i]);
    }

    return 0;
}


int rsa_decrypt(char *decrypt_str, char *result, int size)
{
    int i;
    RSA *rsa;
    char decrypt_buff[4096] = {0};

    if ( !decrypt_str || !result )
        return -1;

    if (decrypt_str[strlen(decrypt_str) - 1] == '\n') decrypt_str[strlen(decrypt_str)] = '\0';
    if (decrypt_str[strlen(decrypt_str) - 1] == '\r') decrypt_str[strlen(decrypt_str)] = '\0';

    int decrypt_len = 0;
    unsigned char ds[4096] = {0};
    for (i = 0; i < (int)strlen(decrypt_str); i += 2) {
        char buf[3] = {0} ;
        buf[0] = decrypt_str[i];
        buf[1] = decrypt_str[i + 1];
        buf[2] = 0;

        ds[i/2] = strtoul(buf, NULL, 16);
        decrypt_len ++;
    }
    printf("decrypt_len : %d\n", decrypt_len);

    rsa = RSA_new();

#if OPENSSL_VERSION_NUMBER < 0x10100000L
    /* OpenSSL 1.0.2 and below */
    BN_hex2bn(&rsa->n, RSA_KEY_N);
    BN_hex2bn(&rsa->e, RSA_KEY_E);
    // rsa->n = BN_bin2bn(key_n, sizeof key_n, NULL);
    // rsa->e = BN_bin2bn(key_e, sizeof key_e, NULL);
#else
    /* OpenSSL 1.1.0 and above */

    // RSA_set0_key(rsa, BN_bin2bn(key_n, sizeof key_n, NULL),
    //                   BN_bin2bn(key_e, sizeof key_e, NULL),
    //                   NULL);

    BIGNUM *n, *e;
    BN_hex2bn(&n, RSA_KEY_N);
    BN_hex2bn(&e, RSA_KEY_E);
    RSA_set0_key(rsa, n, e, d);
#endif

    int decrypt_out_len = RSA_public_decrypt(decrypt_len, ds,
                (unsigned char *)decrypt_buff, rsa, RSA_PKCS1_PADDING);

    RSA_free(rsa);

    if (decrypt_out_len <= 0) {
        perror("RSA public decrypt failed");
        return -2;
    }

    decrypt_buff[decrypt_out_len] = 0;
    // printf(">>%d, %s\n", decrypt_out_len,  decrypt_buff);

    if (decrypt_out_len + 1 > size)
        return -3;
    strncpy(result, decrypt_buff, decrypt_out_len + 1);

    return 0;
}



/*
 * 通过rsa密钥文件进行加解密
 */

#include <openssl/pem.h>
#include <openssl/err.h>
int my_encrypt(const char *input, int input_len,
        char *output, int *output_len, const char *pri_key_fn)
{
    RSA  *p_rsa = NULL;
    FILE *file = NULL;
    int ret = 0;

    if((file = fopen(pri_key_fn, "rb")) == NULL)
    {
        ret = -1;
        goto End;
    }

    if((p_rsa = PEM_read_RSAPrivateKey(file, NULL,NULL,NULL )) == NULL)
    {
        ret = -2;
        goto End;
    }

    if((*output_len = RSA_private_encrypt(input_len, (unsigned char*)input,
                    (unsigned char*)output, p_rsa, RSA_PKCS1_PADDING)) < 0)
    {
        ret = -4;
        goto End;
    }

End:
    if(p_rsa != NULL)
        RSA_free(p_rsa);
    if(file != NULL)
        fclose(file);

    return ret;
}

//解密
int my_decrypt(const char *input, int input_len,
        char *output, int *output_len, const char *pri_key_fn)
{
    RSA  *p_rsa = NULL;
    FILE *file = NULL;
    int ret = 0;

    file = fopen(pri_key_fn, "rb");
    if(!file)
    {
        ret = -1;
        goto End;
    }

    if((p_rsa = PEM_read_RSA_PUBKEY(file, NULL,NULL,NULL )) == NULL)
    {
        ret = -2;
        goto End;
    }

    if((*output_len=RSA_public_decrypt(input_len, (unsigned char*)input,
                    (unsigned char*)output, p_rsa, RSA_PKCS1_PADDING)) < 0)
    {
        ret = -3;
        goto End;
    }

End:
    if(p_rsa != NULL)
        RSA_free(p_rsa);
    if(file != NULL)
        fclose(file);

    return ret;
}






#include "../encoding/base64.h"
int main()
{
    // md5();

    // sha1();

    // generate_key_str();

    unsigned char r[4096] = {0};
    char r2[4096] = {0};
    size_t olen;

    RSA *rsa;
    unsigned char dst_buff[4096] = {0};

    rsa = RSA_new();

#if OPENSSL_VERSION_NUMBER < 0x10100000L
    /* OpenSSL 1.0.2 and below */
    BN_hex2bn(&rsa->n, RSA_KEY_N);
    BN_hex2bn(&rsa->d, RSA_KEY_D);
    BN_hex2bn(&rsa->e, RSA_KEY_E);
#else
    /* OpenSSL 1.1.0 and above */

    // RSA_set0_key(rsa, BN_bin2bn(key_n, sizeof key_n, NULL),
    //                   BN_bin2bn(key_e, sizeof key_e, NULL),
    //                   NULL);

    BIGNUM *n, *e, *d;
    n = BN_new();e = BN_new();d = BN_new();
    BN_hex2bn(&n, RSA_KEY_N);
    BN_hex2bn(&e, RSA_KEY_E);
    BN_hex2bn(&d, RSA_KEY_D);
    RSA_set0_key(rsa, n, e, d);
#endif

    int dsize = RSA_size(rsa);

    const char *str = "1234567890abcdefghijklmn";
    // 先对e加密，然后对加密结果编码进行传输
    int ret = RSA_private_encrypt(strlen(str), (unsigned char *)str, r, rsa, RSA_PKCS1_PADDING);
    RSA_free(rsa);
    if (ret != dsize) {return -1;}
    base64_encode(r2, sizeof r2, &olen, r, ret);
    printf("r2: %s\n", r2);

    /*...*/


    // 先解码再解密
    memset(r, 0, sizeof r);
    base64_decode(r, sizeof r, &olen,(unsigned char *)r2, strlen(r2));

    rsa = RSA_new();
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    /* OpenSSL 1.0.2 and below */
    BN_hex2bn(&rsa->n, RSA_KEY_N);
    BN_hex2bn(&rsa->e, RSA_KEY_E);
#else
    /* OpenSSL 1.1.0 and above */

    // RSA_set0_key(rsa, BN_bin2bn(key_n, sizeof key_n, NULL),
    //                   BN_bin2bn(key_e, sizeof key_e, NULL),
    //                   NULL);

    BIGNUM *n, *e;
    n = BN_new();e = BN_new();
    BN_hex2bn(&n, RSA_KEY_N);
    BN_hex2bn(&e, RSA_KEY_E);
    RSA_set0_key(rsa, n, e, NULL);
#endif

    char buff[1024] = {0};
    int decrypt_out_len = RSA_public_decrypt(olen, r,
                (unsigned char *)buff, rsa, RSA_PKCS1_PADDING);

    RSA_free(rsa);

    if (decrypt_out_len <= 0) {
        perror("RSA public decrypt failed");
        return -2;
    }

    printf(">str: %s\n", buff);

#if 0
    char r[4096] = {0};
    char r2[4096] = {0};
    rsa_encrypt("1234567890abcdefg", r);
    printf("r: %s\n", r);
    rsa_decrypt(r, r2, sizeof r2);
    printf("r2: %s\n", r2);
#endif
    return 0;
}

