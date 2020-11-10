#ifndef _ERRORS_H
#define _ERRORS_H


#ifdef __cplusplus
extern "C" {
#define CLOSE_EXTERN }
#else
#define CLOSE_EXTERN
#endif

/*
 * Returns a textual description of the last error condition returned by
 * an  function.
 */

const char *avro_strerror(void);

void
avro_set_error(const char *fmt, ...);

void
avro_prefix_error(const char *fmt, ...);

CLOSE_EXTERN
#endif
