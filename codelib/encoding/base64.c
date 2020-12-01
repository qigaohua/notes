#include <stdint.h>
#include <stdlib.h>


static const unsigned char base64_enc_map[64] =
{
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
    'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
    'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd',
    'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x',
    'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', '+', '/'
};

static const unsigned char base64_dec_map[128] =
{
    127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
    127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
    127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
    127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
    127, 127, 127,  62, 127, 127, 127,  63,  52,  53,
     54,  55,  56,  57,  58,  59,  60,  61, 127, 127,
    127,  64, 127, 127, 127,   0,   1,   2,   3,   4,
      5,   6,   7,   8,   9,  10,  11,  12,  13,  14,
     15,  16,  17,  18,  19,  20,  21,  22,  23,  24,
     25, 127, 127, 127, 127, 127, 127,  26,  27,  28,
     29,  30,  31,  32,  33,  34,  35,  36,  37,  38,
     39,  40,  41,  42,  43,  44,  45,  46,  47,  48,
     49,  50,  51, 127, 127, 127, 127, 127
};

#define BASE64_SIZE_T_MAX   ( (size_t) -1 ) /* SIZE_T_MAX is not standard */

/*
 * Encode a buffer into base64 format
 */
int base64_encode( unsigned char *dst, size_t dlen, size_t *olen,
                   const unsigned char *src, size_t slen )
{
    size_t i, n;
    int C1, C2, C3;
    unsigned char *p;

    if( slen == 0 ) {
        *olen = 0;
        return( 0 );
    }

    n = slen / 3 + ( slen % 3 != 0 );

    if( n > ( BASE64_SIZE_T_MAX - 1 ) / 4 ) {
        *olen = BASE64_SIZE_T_MAX;
        return -1;
    }

    n *= 4;    // 编码后的字符数

    if( ( dlen < n + 1 ) || ( NULL == dst ) ) {
    	/*This will be the buf size that you need to malloc when
    	your dst is NULL*/
        *olen = n + 1;
        return -2;
    }

    n = ( slen / 3 ) * 3;

    for( i = 0, p = dst; i < n; i += 3 )
    {
        C1 = *src++;
        C2 = *src++;
        C3 = *src++;

        *p++ = base64_enc_map[(C1 >> 2) & 0x3F];
        *p++ = base64_enc_map[(((C1 &  3) << 4) + (C2 >> 4)) & 0x3F];
        *p++ = base64_enc_map[(((C2 & 15) << 2) + (C3 >> 6)) & 0x3F];
        *p++ = base64_enc_map[C3 & 0x3F];
    }

    if( i < slen )
    {
        C1 = *src++;
        C2 = ( ( i + 1 ) < slen ) ? *src++ : 0;

        *p++ = base64_enc_map[(C1 >> 2) & 0x3F];
        *p++ = base64_enc_map[(((C1 & 3) << 4) + (C2 >> 4)) & 0x3F];

        if( ( i + 1 ) < slen )
             *p++ = base64_enc_map[((C2 & 15) << 2) & 0x3F];
        else *p++ = '=';

        *p++ = '=';
    }

    *olen = p - dst;
    *p = 0;

    return( 0 );
}

/*
 * Decode a base64-formatted buffer
 */
int base64_decode( unsigned char *dst, size_t dlen, size_t *olen,
                   const unsigned char *src, size_t slen )
{
    size_t i, n;
    uint32_t j, x;
    unsigned char *p;

    /* First pass: check for validity and get output length */
    for( i = n = j = 0; i < slen; i++ )
    {
        /* Skip spaces before checking for EOL */
        x = 0;
        while( i < slen && src[i] == ' ' ) {
            ++i;
            ++x;
        }

        /* Spaces at end of buffer are OK */
        if( i == slen )
            break;

        if( ( slen - i ) >= 2 &&
            src[i] == '\r' && src[i + 1] == '\n' )
            continue;

        if( src[i] == '\n' )
            continue;

        /* Space inside a line is an error */
        if( x != 0 )
            return( -1 );

        if( src[i] == '=' && ++j > 2 )
            return( -1 );

        if( src[i] > 127 || base64_dec_map[src[i]] == 127 )
            return( -1 );

        if( base64_dec_map[src[i]] < 64 && j != 0 )
            return( -1 );

        n++;
    }

    if( n == 0 ) {
        *olen = 0;
        return( 0 );
    }

    /* The following expression is to calculate the following formula without
     * risk of integer overflow in n:
     *     n = ( ( n * 6 ) + 7 ) >> 3;
     */
    n = ( 6 * ( n >> 3 ) ) + ( ( 6 * ( n & 0x7 ) + 7 ) >> 3 );
    n -= j;

    if( dst == NULL || dlen < n )
    {
    	/*This will be the buf size that you need to malloc when
    	your dst is NULL*/
        *olen = n;
        return( -2);
    }

   for( j = 3, n = x = 0, p = dst; i > 0; i--, src++ )
   {
        if( *src == '\r' || *src == '\n' || *src == ' ' )
            continue;

        j -= ( base64_dec_map[*src] == 64 );
        x  = ( x << 6 ) | ( base64_dec_map[*src] & 0x3F );

        if( ++n == 4 )
        {
            n = 0;
            if( j > 0 ) *p++ = (unsigned char)( x >> 16 );
            if( j > 1 ) *p++ = (unsigned char)( x >>  8 );
            if( j > 2 ) *p++ = (unsigned char)( x       );
        }
    }

    *olen = p - dst;

    return( 0 );
}



#if 0
static const unsigned char _base64_encode_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const unsigned char _base64_decode_chars[] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,
    -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,
    -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1
};


static inline char* base64_encode(const unsigned char* in_str, int in_len, char* out_str)
{
    unsigned char c1, c2, c3;
    int i = 0;
    int len = in_len;
    int index = 0;
    while ( i<len ) {
        c1 = in_str[i++];
        if ( i == len ) {
            out_str[index++] = _base64_encode_chars[ c1>>2 ];
            out_str[index++] = _base64_encode_chars[ (c1&0x3)<<4 ];
            out_str[index++] = '=';
            out_str[index++] = '=';
            break;
        }
        c2 = in_str[i++];
        if ( i == len ) {
            out_str[index++] = _base64_encode_chars[ c1>>2 ];
            out_str[index++] = _base64_encode_chars[ ((c1&0x3)<<4) | ((c2&0xF0)>>4) ];
            out_str[index++] = _base64_encode_chars[ (c2&0xF)<<2 ];
            out_str[index++] = '=';
            break;
        }
        c3 = in_str[i++];
        out_str[index++] = _base64_encode_chars[ c1>>2 ];
        out_str[index++] = _base64_encode_chars[ ((c1&0x3)<<4) | ((c2&0xF0)>>4) ];
        out_str[index++] = _base64_encode_chars[ ((c2&0xF)<<2) | ((c3&0xC0)>>6) ];
        out_str[index++] = _base64_encode_chars[ c3&0x3F ];
    }
    out_str[index] = 0;
    return out_str;
}

static inline int base64_decode(const unsigned char* in_base64, int in_len,  unsigned char* out_data, int max_out_data_len)
{
    unsigned char c1, c2, c3, c4;
    int i = 0;
    int len = in_len;
    int index = 0;
    while ( i<len) {
        do { c1 = _base64_decode_chars[in_base64[i++] ]; } while ( i<len && c1 == 0xff);
        if ( c1 == 0xff) break;
        do { c2 = _base64_decode_chars[in_base64[i++] ]; } while ( i<len && c2 == 0xff);
        if ( c2 == 0xff ) break;
        if (index < max_out_data_len) out_data[index++] = (char) ((c1<<2) | ((c2&0x30)>>4)) ;
        do {
            c3 = in_base64[i++];
            if ( c3 == 61 ){ out_data[index] = 0; return index; } // meet with "=", break
            c3 = _base64_decode_chars[c3 ];
        } while ( i<len && c3 == 0xff);
        if ( c3 == 0xff ) break;
        if (index < max_out_data_len) out_data[index++] =  (char) ( ((c2&0XF)<<4) | ((c3&0x3C)>>2) );
        do {
            c4 = in_base64[i++]; if ( c4 == 61 ) { out_data[index] = 0; return index; } // meet with "=", break
            c4 = _base64_decode_chars[c4 ];
        } while ( i<len && c4 == 0xff );
        if ( c4 == 0xff ) break;
        if (index < max_out_data_len) out_data[index++] =  (char) ( ((c3&0x03)<<6) | c4 );
    }
    out_data[index] = 0;
    return index;
}
#endif
