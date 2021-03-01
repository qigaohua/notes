#ifndef __MYSQLOP_H__
#define __MYSQLOP_H__ 
#include <inttypes.h>
#include <mysql/mysql.h>


#define SQL_ERROR_INFO_EXIT(fmt, ...) \
    do { \
        printf("[%s][%d][error]:" fmt"\n", __FILE__, __LINE__,  ##__VA_ARGS__); \
        syslog(LOG_ERR, "[%s][%d][error]:" fmt"\n", __FILE__, __LINE__,  ##__VA_ARGS__);\
        exit(-1); \
    } while(0);

#define SQL_ERROR_INFO(fmt, ...) \
    do { \
        printf("[%s][%d][error]:" fmt"\n", __FILE__, __LINE__,  ##__VA_ARGS__); \
        syslog(LOG_ERR, "[%s][%d][error]:" fmt"\n", __FILE__, __LINE__,  ##__VA_ARGS__);\
    } while(0);


#define SQL_LOGG_INFO(fmt, ...) \
    do { \
        printf("[%s][%d][debug]:" fmt"\n", __FILE__, __LINE__, ## __VA_ARGS__); \
        syslog(LOG_INFO, "[%s][%d][error]:" fmt"\n", __FILE__, __LINE__, ## __VA_ARGS__);\
    } while(0);


struct db_info {
    char db_host[64];
    char db_name[64];
    char db_username[64];
    char db_passwd[64];
    uint16_t db_port;
};
typedef struct db_info DB_INFO, *db_info_p;


MYSQL* get_mysql_handle(MYSQL **ppmysql, db_info_p db_conf);
MYSQL_RES* mysql_query_store_res (MYSQL *mysql, MYSQL_RES **res,  char *sql); 
int32_t mysql_query_batch (MYSQL *mysql, char **sql); 


#endif
