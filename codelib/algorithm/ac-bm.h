 #ifndef _AC_BM_H
#define _AC_BM_H
#define PATTERN_LEN 1024
#define MAX_ITEMS 20

/// 模式树节点结构
typedef struct _pattern_tree_node
{
 int label ;         // 标识, -2 根节点, -1 中间节点, n 第n个字串尾节点
 int depth ;           // 节点深度

 unsigned char ch ;       // 节点对应的字符
 int GSshift ;        // 好字串位移
 int BCshift ;
 unsigned char one_child ;     // 其中的一个子索引字符
 struct _pattern_tree_node *childs[256] ; // 256个字符的对应节点指针
 int nchild ;                             // 子节点个数
 struct _pattern_tree_node *parent ;      // 父节点
} pattern_tree_node ;

// 关键字数据结构
typedef struct _pattern_data
{
 unsigned char data[PATTERN_LEN] ; // 关键字字串
 int len ;                         // 关键字字串长
} pattern_data ;

// 模式树结构
typedef struct _pattern_tree
{
 pattern_tree_node *root ;  // 树根节点
 int max_depth ;     // 最大字串深度
 int min_pattern_size ;          // 最短的字串长度
 int BCshift[256] ;             // 256个字符中坏字符的shift
 pattern_data *pattern_list ;  // 指向节点数组第一个字串的指针
 int pattern_count ;             // 包含的字串个数
} pattern_tree ;

// 匹配信息结构体
typedef struct _matched_info
{
 int pattern_i ;               // 关键字在关键字数组中的index
 unsigned long offset ;        // 在待匹配文本text中的偏移值
} matched_info_t ;
// 创建 模式树
int ACtree_build (pattern_tree *ptree,
      pattern_data *patterns,
      int npattern) ;
// 打印 当前节点及其所有后缀节点
void _print_tree (pattern_tree_node *root) ;
// 打印 整个模式树
void ACtree_print_tree (pattern_tree *ptree);
// 打印 搜索结果
int match_resualt_printf_ex (unsigned char *text,
        pattern_data *patterns,
        int npattern,
        matched_info_t matched_items[],
        int nmatched);
// 释放 模式树空间
void _clean_tree(pattern_tree_node *root) ;
// 设置 模式树的BCshift
int ACtree_compute_BCshifts (pattern_tree *ptree) ;
// 创建 ac_bm模式树
pattern_tree *acbm_init (pattern_data *patterns, int npattern) ;
// 释放 ac_bm模式树空间
void acbm_clean (pattern_tree *ptree) ;
// ac_bm搜索算法改进版
int acbm_search (pattern_tree *ptree,
     unsigned char *text,
     int text_len,
     matched_info_t matched_items[],
     int nmax_index);
// 计算并调整ACTree模式树的BCshift和GSshift
int ACtree_compute_shifts(pattern_tree *ptree) ;
// 计算并调整ACTree模式树的GSshift
int ACtree_compute_GSshifts(pattern_tree *ptree) ;
// 初始化ACTree的GSshift
int ACtree_init_GSshifts(pattern_tree *ptree) ;
// 初始化当前节点的GSshift
int _init_GSshifts(pattern_tree_node *root, int shift) ;
// 调整ACTree中关键字pat1的GSshift
int set_GSshift (pattern_tree *ptree,
     unsigned char *pat,
     int depth,
     int shift) ;
// 调整ACTree中关键字pat1因pat2而发生变化的GSshift
int compute_GSshift(pattern_tree *ptree,
        unsigned char *pat1,
     int pat1_len,
     unsigned char *pat2,
     int pat2_len) ;
#endif
