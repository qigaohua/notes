#ifndef __BASE64_H_
#define __BASE64_H_ 

#include <stdlib.h>


int base64_encode( unsigned char *dst, size_t dlen, size_t *olen,const unsigned char *src, size_t slen );
int base64_decode( unsigned char *dst, size_t dlen, size_t *olen,const unsigned char *src, size_t slen );


#endif
