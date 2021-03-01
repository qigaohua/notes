#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>

#include <mysql/mysql.h>
#include <mysql/mysqld_error.h>

#include "mysql_op.h"


MYSQL* get_mysql_handle(MYSQL **ppmysql, db_info_p db_conf)
{
    *ppmysql = mysql_init(NULL);
    if (*ppmysql == NULL) 
        SQL_ERROR_INFO_EXIT("mysql_init failed\n");

    if (NULL == mysql_real_connect(*ppmysql, db_conf->db_host, db_conf->db_username, 
                db_conf->db_passwd, db_conf->db_name, db_conf->db_port, NULL, 0)) 
        SQL_ERROR_INFO_EXIT("mysql connect failed[%s]\n", mysql_error(*ppmysql));

    return *ppmysql;
}

MYSQL_RES* mysql_query_store_res (MYSQL *mysql, MYSQL_RES **res,  char *sql) 
{
    int32_t ret;

    ret = mysql_query(mysql, sql);
    if(ret) SQL_ERROR_INFO_EXIT("mysql query failed[%s]\n", sql);

    *res = mysql_store_result(mysql);
    if (*res == NULL) SQL_ERROR_INFO_EXIT("mysql store result failed\n");

    return *res;
}

/* mysql 批量操作 */
int32_t mysql_query_batch (MYSQL *mysql, char **sql) 
{
#define MAX_SQL_SIZE 1000
    int32_t i, ret, count = 0;
    
    if(!sql) return -1;

    // 关闭自动提交
    mysql_autocommit(mysql, 0);
 
    int32_t sql_size = sizeof(sql) / sizeof(sql[0]);
    for (i = 0; i < sql_size; i++) {
        ret = mysql_query(mysql, sql[i]);
        if (ret != 0) { SQL_ERROR_INFO_EXIT("mysql query failed[%s]\n", sql[i]); } 
        else count ++;

        /* 提交 */
        if (count > MAX_SQL_SIZE) {
            mysql_commit(mysql);
            count = 0;
        }
    }

    if (count > 0)
        mysql_commit(mysql);

    // 恢复原来设置
    mysql_autocommit(mysql, 1);

    return 0;
}
#if 0
int main(void *db)
{
    int32_t  num, limit;
    MYSQL *mysql;
    MYSQL_ROW row;
    MYSQL_RES *res;
    char buf[1024] = {0};

    if (!db) return -1;
    mysql = (MYSQL*)db;

    for (limit = 0; ; limit += 1000) {
        snprintf(buf, sizeof(buf), "");

        /*
           ret = mysql_query(mysql, buf);
           if(ret) ERROR_INFO_EXIT("mysql query failed[%s]\n", buf);

           res = mysql_store_result(mysql);
           if (res == NULL) ERROR_INFO_EXIT("mysql store result failed\n");
        */
        mysqlQueryStoreRes(mysql, &res, buf); 

        num = mysql_num_rows(res);
        if (num == 0) break;

        while((row = mysql_fetch_row(res))) {

        }
        mysql_free_result(res);
    }

    return 0;
}
#endif
