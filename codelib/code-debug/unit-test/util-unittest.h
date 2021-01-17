#ifndef __UTIL_UNITTEST_H__
#define __UTIL_UNITTEST_H__

#define UNITTESTS
#ifdef UNITTESTS

#ifndef likely
#define likely(expr) __builtin_expect(!!(expr), 1)
#endif
#ifndef unlikely
#define unlikely(expr) __builtin_expect(!!(expr), 0)
#endif

/* we need this to stringify the defines which are supplied at compiletime see:
   http://gcc.gnu.org/onlinedocs/gcc-3.4.1/cpp/Stringification.html#Stringification */
#define xstr(s) str(s)
#define str(s) #s

#if CPPCHECK==1
    #define BUG_ON(x) if (((x))) exit(1)
#else
    #if defined HAVE_ASSERT_H && !defined NDEBUG
    #include <assert.h>
        #define BUG_ON(x) assert(!(x))
    #else
        #define BUG_ON(x) do {      \
            if (((x))) {            \
                fprintf(stderr, "BUG at %s:%d(%s)\n", __FILE__, __LINE__, __func__);    \
                fprintf(stderr, "Code: '%s'\n", xstr((x)));                             \
                exit(EXIT_FAILURE); \
            }                       \
        } while(0)
    #endif
#endif



typedef struct UtTest_
{
    const char *name;
    int(*TestFn)(void);

    struct UtTest_ *next;

} UtTest;

void UtRegisterTest(const char *name, int(*TestFn)(void));
uint32_t UtRunTests(const char *regex_arg);
void UtInitialize(void);
void UtCleanup(void);
int UtRunSelftest (const char *regex_arg);
void UtListTests(const char *regex_arg);
void UtRunModeRegister(void);

extern int unittests_fatal;

/**
 * \breif Fail a test.
 */
#define FAIL do {                                      \
        if (unittests_fatal) {                         \
            BUG_ON(1);                                 \
        } else {                                       \
            return 0;                                  \
        }                                              \
    } while (0)

/**
 * \brief Fail a test if expression evaluates to false.
 */
#define FAIL_IF(expr) do {                             \
        if (unittests_fatal) {                         \
            BUG_ON(expr);                              \
        } else if (expr) {                             \
            return 0;                                  \
        }                                              \
    } while (0)

/**
 * \brief Fail a test if expression to true.
 */
#define FAIL_IF_NOT(expr) do { \
        FAIL_IF(!(expr));      \
    } while (0)

/**
 * \brief Fail a test if expression evaluates to NULL.
 */
#define FAIL_IF_NULL(expr) do {                 \
        FAIL_IF(NULL == expr);                  \
    } while (0)

/**
 * \brief Fail a test if expression evaluates to non-NULL.
 */
#define FAIL_IF_NOT_NULL(expr) do { \
        FAIL_IF(NULL != expr);      \
    } while (0)

/**
 * \brief Pass the test.
 *
 * Only to be used at the end of a function instead instead of "return 1."
 */
#define PASS do { \
        return 1; \
    } while (0)

#endif

/**
 * \brief Pass the test if expression evaluates to true.
 *
 * Only to be used at the end of a function instead of returning the
 * result of an expression.
 */
#define PASS_IF(expr) do { \
        FAIL_IF(!(expr));  \
        PASS;              \
    } while (0)

#endif /* __UTIL_UNITTEST_H__ */

/**
 * @}
 */
