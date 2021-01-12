#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "ac-bm.h"
// 宏定义
//#define CASE_SENSITIVE
//#define DEBUG_SEARCH

unsigned char mychar[100];

/*--------------------------------------------------------------------------
 * 函数名:
 *    ACTree_bulid
 *
 * 函数功能:
 *    创建ACtree模式树,包括各个节点,字符索引,及其他的信息,ptree指向根节点
 *
 * 参数说明:
 *    pattern_tree *ptree    : 指向ACTree的指针
 *    pattern_data *patterns : 关键字字串数组
 *    int npattern           : 关键字字串个数
 *
 * 返回值类型:   int
 *    0 : 创建成功
 *   -1 : 出错
 ----------------------------------------------------------------------------*/
int ACtree_build (pattern_tree *ptree,
        pattern_data *patterns,
        int npattern)
{
    int i ;
    pattern_tree_node *root = NULL, *parent = NULL ;
    unsigned char ch ;
    int max_pattern_len = 0, min_pattern_len = PATTERN_LEN ;
    if (NULL == ptree || NULL == patterns || npattern < 0)
    {
        goto err ;
    }

    root = (pattern_tree_node *) malloc (sizeof (pattern_tree_node)) ;
    if (NULL == root)
    {
        goto err ;
    }
    memset (root, 0, sizeof (pattern_tree_node)) ;
    root->label = -2 ;    // 树根标志
    root->depth = 0 ;     // 树根深度
    // 对输入的字串循环处理添加进ACTree
    for (i = 0 ; i < npattern ; i++)
    {
        int pat_len ;
        int ch_i ;
        pat_len = (patterns+i)->len ;
        if (pat_len == 0)
        {
            continue ;
        }
        else
        {
            if (pat_len > PATTERN_LEN)
            {
                pat_len = PATTERN_LEN ;
            }
            if (pat_len > max_pattern_len)
            {
                max_pattern_len = pat_len ;
            }
            if (pat_len < min_pattern_len)
            {
                min_pattern_len = pat_len ;
            }
            parent = root ;
            for (ch_i = 0 ; ch_i < pat_len ; ch_i++)
            {
                ch = ((patterns+i)->data)[ch_i] ;
#ifndef CASE_SENSITIVE
                ch = tolower(ch) ;
#endif
                // 对应的字符索引为NULL
                if (NULL == parent->childs[ch])
                {
                    break ;
                }
                parent = parent->childs[ch] ;
            }
            if (ch_i < pat_len)
            {
                // 在父节点下添加新节点
                for (; ch_i < pat_len ; ch_i++)
                {
                    pattern_tree_node *node = NULL ;
                    ch = ((patterns+i)->data)[ch_i] ;
#ifndef CASE_SENSITIVE
                    ch = tolower(ch) ;
#endif
                    node = (pattern_tree_node *) malloc (sizeof (pattern_tree_node)) ;
                    if (node == NULL)
                    {
                        goto err ;
                    }
                    memset (node, 0, sizeof(pattern_tree_node)) ;
                    node->depth = ch_i + 1 ;
                    node->ch = ch ;
                    node->label = -1 ;
                    // 将新节点添加到父节点的对应字符索引指针
                    parent->childs[ch] = node ;
                    // 不考虑大小写的分别
#ifndef CASE_SENSITIVE
                    if ((ch >= 'a') && (ch <= 'z'))
                    {
                        parent->childs[ch-32] = node ;
                    }
#endif
                    parent->nchild++ ;
                    parent->one_child = ch ;
                    node->parent = parent ;
                    parent = node ;
                }
            }
        }
        // lable 记录字串来自于第几个输入字串
        parent->label = i ;
    }
    ptree->pattern_list = patterns ;
    ptree->pattern_count = npattern ;
    ptree->root = root ;
    ptree->max_depth = max_pattern_len ;
    ptree->min_pattern_size = min_pattern_len ;
    return 0 ;
err:
    // 出错处理,释放申请的空间
    if (ptree->root != NULL)
    {
        _clean_tree (ptree->root) ;
        free (ptree->root) ;
        ptree->root = NULL ;
    }
    return -1 ;
}

/*--------------------------------------------------------------------------
 * 函数名:
 *    _print_tree
 *
 * 函数功能:
 *    打印当前节点及其所有后缀节点
 *
 * 参数说明:
 *    pattern_tree_node *root : 当前的节点的指针
 *
 * 返回值类型:  void
 *    无
 ----------------------------------------------------------------------------*/
void _print_tree (pattern_tree_node *root)
{
    int i ;
    if (NULL == root)
        return ;
    printf("ch:%2c   GSshift:%2d   label:%2d   depth:%2d   childs:", root->ch, root->GSshift, root->label, root->depth) ;
    for (i = 0 ; i < 256 ; i++)
    {
#ifndef CASE_SENSITIVE
        if ((i >= 'A') && (i <= 'Z'))
        {
            continue ;
        }
#endif
        if (NULL != root->childs[i])
        {
            printf("%c ", (root->childs[i])->ch) ;
        }
    }
    printf("\n") ;
    // 递归打印本节点的所有后缀节点信息
    for (i = 0 ; i < 256 ; i++)
    {
        if (NULL != root->childs[i])
        {
            _print_tree (root->childs[i]) ;
        }
    }
    return ;
}

/*--------------------------------------------------------------------------
 * 函数名:
 *    ACtree_print_tree
 *
 * 函数功能:
 *    打印整个树ACTree的所有节点字符信息
 *
 *  参数说明:
 *    pattern_tree *ptree    : 指向ACTree模式树的指针
 *
 * 返回值类型:  void
 *    无
 ----------------------------------------------------------------------------*/
//#if 0
void ACtree_print_tree (pattern_tree *ptree)
{
    printf ("--- ACTree information---\n") ;
    if (NULL == ptree)
    {
        return ;
    }
    if (NULL != ptree->root)
    {
        _print_tree (ptree->root) ;
    }
    return ;
}
//#endif

/*--------------------------------------------------------------------------
 * 函数名:
 *    ACtree_compute_BCshifts
 *
 * 函数功能:
 *    设置ptree的BCshift
 *
 *  参数说明:
 *    pattern_tree *ptree    : 指向ACTree模式树的指针
 *
 * 返回值:  int
 *    0 : 处理成功
 ----------------------------------------------------------------------------*/
int ACtree_compute_BCshifts (pattern_tree *ptree)
{
    int i, j = 0 ;
    for (i = 0 ; i < 256 ; i++)
        ptree->BCshift[i] = ptree->min_pattern_size ;
    for (i = ptree->min_pattern_size - 1 ; i > 0 ; i--)
    {
        for (j = 0 ; j < ptree->pattern_count ; j++)
        {
            unsigned char ch ;
            ch = (ptree->pattern_list+j)->data[i] ;

#ifndef CASE_SENSITIVE
            ch = tolower(ch) ;
#endif
            ptree->BCshift[ch] = i ;
#ifndef CASE_SENSITIVE
            if ((ch > 'a') && (ch <'z'))
            {
                ptree->BCshift[ch-32] = i ;
            }
#endif
        }
    }
    return 0 ;
}

/*--------------------------------------------------------------------------
 * 函数名:
 *    set_GSshift
 *
 * 函数功能:
 *    设置ACTree中关键字pat1的GCshift
 *
 * 参数说明:
 *    pattern_tree *ptree    : 指向ACTree模式树的指针
 *    unsigned char *pat     : 关键字pat1
 *    int depth              : 要改变GSshift字符的深度为depth
 *    int shift              : 拟进行的变化
 *
 * 返回值:  int
 *    0 : 处理成功
 *   -1 : 出错
 ----------------------------------------------------------------------------*/
int set_GSshift (pattern_tree *ptree, unsigned char *pat, int depth, int shift)
{
    int i ;
    pattern_tree_node *node ;
    if (NULL == ptree || NULL == ptree->root)
    {
        goto err ;
    }
    node = ptree->root ;
    for (i = 0 ; i < depth ; i++)
    {
        node = node->childs[pat[i]] ;
        if (NULL == node)
        {
            goto err ;
        }
    }

    // 取小位移
    node->GSshift = node->GSshift < shift ? node->GSshift : shift ;
    return 0 ;
err:
    return -1 ;
}

/*--------------------------------------------------------------------------
 * 函数名:
 *    compute_GSshift
 *
 * 函数功能:
 *    调整ACTree中关键字pat1因pat2而发生变化的GSshift
 *
 * 参数说明:
 *    pattern_tree *ptree    : 指向ACTree的指针
 *    unsigned char *pat1    : 关键字pat1
 *    int pat1_len           : 关键字pat1的字串长度
 *    unsigned char *pat2    : 关键字pat2
 *    int pat2_len           : 关键字pat2的字串长度
 *
 * 返回值:  int
 *    0 : 处理成功 或 未出错
 *   -1 : 出错
 ----------------------------------------------------------------------------*/
int compute_GSshift (pattern_tree *ptree,
        unsigned char *pat1,
        int pat1_len,
        unsigned char *pat2,
        int pat2_len)
{
    unsigned char first_char ;
    int i ;
    int pat1_index, pat2_index, offset ;
    if (NULL == pat1 || NULL == pat2 || pat1_len < 0 || pat2_len < 0)
    {
        goto err ;
    }
    if (pat1_len == 0 || pat2_len == 0)
    {
        return 0 ;
    }
    first_char = pat1[0] ;

#ifndef CASE_SENSITIVE
    first_char = tolower(first_char) ;
#endif
    // 从pat2第二个字符开始考虑, 为什么不是第一个字符？？ 因为第一个比较没有意义。这段代码主要作用于寻找
    //前缀在字符串中重复出现？为什么只考虑第二位？如果第二位不同直接跳转，如果在第三位出现不是找不到了？
    for (i = 1 ; i < pat2_len ; i++)
    {
#ifndef CASE_SENSITIVE
        if (tolower(pat2[i]) != first_char)
#else
            if (pat2[i] != first_char)
#endif
                break ;
    }
    set_GSshift (ptree, pat1, 1, i) ;   //计算每个模式串第一个字符的GSshift
    //ACtree_print_tree (ptree) ;   //测试
    i = 1 ;               //为什么又重新负值1？？
    while (1)                       //i用来的循环，用途？？
    {
        // 在pat2中寻找与pat1首字符相同字符的首出现位置，
#ifndef CASE_SENSITIVE
        while (i < pat2_len && tolower(pat2[i]) != first_char)
#else
            while (i < pat2_len && pat2[i] != first_char)          //找到与模式串第一位相等的地方
#endif
                i++ ;

        // pat2剩余字符中未发现有pat1首字符相同字符,结束函数
        if (i == pat2_len)
        {
            break ;
        }

        // 仍然发现有首字符相同字符
        pat2_index = i ;
        pat1_index = 0 ;
        offset = i ;                         //移动的距离？？

        // 如果偏移反而大于最短关键字长度,不予考虑,因为最大的允许跳转就是min_pattern_size
        if (offset > ptree->min_pattern_size)
        {
            break ;
        }

        while (pat2_index < pat2_len && pat1_index < pat1_len) // 重复循环直至两关键字中字符首次不同
        {
#ifndef CASE_SENSITIVE
            if (tolower(pat1[pat1_index]) != tolower(pat2[pat2_index]))
#else
                if (pat1[pat1_index] != pat2[pat2_index])
#endif
                    break ;
            pat1_index++ ;               //是比较位的字符的深度
            pat2_index++ ;
        }

        if (pat2_index == pat2_len) // 关键字pat1前缀是关键字pat2后缀
        {
            int j ;                    //j的用途？
            for (j = pat1_index ; j < pat1_len ; j++)
            {
                set_GSshift (ptree, pat1, j+1, offset) ;
                //ACtree_print_tree (ptree) ;   //测试
            }
        }
        else // pat1的前缀是pat2的子串,被pat2包含
        {
            set_GSshift (ptree, pat1, pat1_index+1, offset) ;          //字符所在的深度和序号差一位
            //ACtree_print_tree (ptree) ;   //测试
        }
        i++ ;
    }
    return 0 ;

err:
    return -1 ;
}

/*--------------------------------------------------------------------------
 * 函数名:
 *    ACtree_compute_GSshifts
 *
 * 函数功能:
 *    计算整个ACTree的GSshifts
 *
 * 参数说明:
 *    pattern_tree *ptree    : 指向ACTree的指针
 *
 * 返回值:  int
 *    0 : 无意义
 ----------------------------------------------------------------------------*/
int ACtree_compute_GSshifts (pattern_tree *ptree)
{
    int pat_i = 0, pat_j = 0 ;

    for (pat_i = 0 ; pat_i < ptree->pattern_count ; pat_i++)
    {
        for (pat_j = 0 ; pat_j < ptree->pattern_count ; pat_j++)
        {
            unsigned char *ppat_i = (ptree->pattern_list+pat_i)->data ;
            int patlen_i = (ptree->pattern_list+pat_i)->len ;
            unsigned char *ppat_j = (ptree->pattern_list+pat_j)->data ;
            int patlen_j = (ptree->pattern_list+pat_j)->len ;
            //只需要计算ppat_i就可以算出每个字符的GSshift；通过set_GSshift计算。
            compute_GSshift (ptree, ppat_i, patlen_i, ppat_j, patlen_j) ;
            //   printf ("\n函数 ACtree_compute_GSshifts---------------") ; // 测试使用
            //   printf ("\n----调用 compute_GSshift-------------------\n") ; // 测试使用
            //   ACtree_print_tree(ptree) ;  // 测试使用
        }
    }
    return 0 ;
}

/*--------------------------------------------------------------------------
 * 函数名:
 *    _init_GSshifts
 *
 * 函数功能:
 *    初始化当前节点及其所有后续节点的的GSshifts为参数shift
 *
 * 参数说明:
 *    pattern_tree_node *root    : 指向当前节点的指针
 *    int shift                  : 初始化值
 *
 *　返回值:  int
 *    0 : 无意义
 ----------------------------------------------------------------------------*/
int _init_GSshifts (pattern_tree_node *root, int shift)
{
    int i ;
    if (root->label != -2)
        root->GSshift = shift ;

    for (i = 0 ; i < 256 ; i++)
    {
#ifndef CASE_SENSITIVE
        if ((i >= 'A') && (i <= 'Z'))
        {
            continue ;
        }
#endif
        if (NULL != root->childs[i])
        {
            _init_GSshifts (root->childs[i], shift) ;
        }
    }
    return 0 ;
}

/*--------------------------------------------------------------------------
 * 函数名:
 *    ACtree_init_GSshifts
 *
 *　函数功能:
 *    初始化整个ACtree模式树的GSshifts
 *
 *  参数说明:
 *    pattern_tree *ptree : 指向ACTree的指针
 *
 * 返回值:  int
 *    0 : 无意义
 ----------------------------------------------------------------------------*/
int ACtree_init_GSshifts (pattern_tree *ptree)
{
    _init_GSshifts (ptree->root, ptree->min_pattern_size) ;
    return 0 ;
}

/*--------------------------------------------------------------------------
 * 函数名:
 *    ACtree_compute_shifts
 *
 * 函数功能:
 *    计算并调整ACtree的BCshift和GSshift
 *
 * 参数说明:
 *    pattern_tree *ptree    : 指向ACTree的指针
 *
 * 返回值:  int
 *    0 : 无意义
 ----------------------------------------------------------------------------*/
int ACtree_compute_shifts (pattern_tree *ptree)
{
    ACtree_compute_BCshifts (ptree) ;
    // printf ("\n函数 ACtree_compute_shifts----------------------") ; // 测试使用
    // printf ("\n----调用 ACtree_compute_BCshifts----------------\n") ; // 测试使用
    // ACtree_print_tree(ptree)  ; // 测试使用

    ACtree_init_GSshifts (ptree) ;
    // printf ("\n函数 ACtree_compute_shifts----------------------") ; // 测试使用
    // printf ("\n----调用 ACtree_init_GSshifts-------------------\n") ; // 测试使用
    // ACtree_print_tree(ptree) ;  // 测试使用
    ACtree_compute_GSshifts (ptree) ;
    // printf ("\n函数 ACtree_compute_shifts----------------------") ; // 测试使用
    // printf ("\n----调用 ACtree_compute_GSshifts----------------\n") ; // 测试使用
    // ACtree_print_tree(ptree) ;  // 测试使用
    return 0 ;
}

/*--------------------------------------------------------------------------
 * 函数名:
 *    acbm_init
 *
 * 函数功能:
 *    创建 ac_bm模式树,同时计算并调整BCshift和GSshift
 *
 * 参数说明:
 *    pattern_data *patterns    : 指向关键字字串数组的指针
 *    int npattern              : 关键字字串个数
 *
 * 返回值:  pattern_tree *
 *    ptree : 处理成功,返回ptree
 *    NULL  : 处理失败,返回空指针
 ----------------------------------------------------------------------------*/
pattern_tree *acbm_init (pattern_data *patterns, int npattern)
{
    pattern_tree *ptree = NULL ;

    ptree = (pattern_tree *) malloc (sizeof (pattern_tree)) ;
    if (NULL == ptree)
    {
        goto err ;
    }
    memset (ptree, 0, sizeof(pattern_tree)) ;
    ACtree_build (ptree, patterns, npattern) ;
    // printf ("\n函数 acbm_init----------------------------") ;  // 测试使用
    // printf ("\n----调用 ACtree_build---------------------\n") ; // 测试使用
    // ACtree_print_tree(ptree) ; // 测试使用
    ACtree_compute_shifts (ptree) ;
    // printf ("\n函数 acbm_init----------------------------") ; // 测试使用
    // printf ("\n----调用 ACtree_compute_shifts------------\n") ; // 测试使用
    // ACtree_print_tree(ptree) ;  // 测试使用
    return ptree ;
err:
    return NULL ;
}
/*--------------------------------------------------------------------------
 * 函数名:
 *    _clean_tree
 *
 * 函数功能:
 *    释放当前节点及其所有后续节点申请的存储空间
 *
 * 参数说明:
 *    pattern_tree_node *root : 指向当前节点的指针
 *
 * 返回值:  void
 *     无
 ----------------------------------------------------------------------------*/
void _clean_tree (pattern_tree_node *root)
{
    int i ;
    for (i = 0 ; i < 256 ; i++)
    {
#ifndef CASE_SENSITIVE
        if ((i >= 'A') && (i <= 'Z'))
        {
            continue ;
        }
#endif
        if (NULL != root->childs[i])
        {
            _clean_tree (root->childs[i]) ;
            free (root->childs[i]) ;
            root->childs[i] = NULL ;
        }
    }
    return ;
}

/*--------------------------------------------------------------------------
 * 函数名:
 *    acbm_clean
 *
 *  函数功能:
 *    释放整个ac_bm模式树申请的空间
 *
 * 参数说明:
 *    pattern_tree *ptree   : 指向ACTree的指针
 *
 * 返回值:  void
 *     无
 ----------------------------------------------------------------------------*/
void acbm_clean (pattern_tree *ptree)
{
    if (NULL == ptree)
    {
        return ;
    }
    if (NULL != ptree->root)
    {
        _clean_tree (ptree->root) ;
        free (ptree->root) ;
        ptree->root = NULL ;
    }
    free (ptree) ;
    return ;
}

/*--------------------------------------------------------------------------
 * 函数名:
 *    acbm_search
 *
 * 函数功能:
 *    搜索算法,搜索输入的文本text中包含的关键字的个数,并将搜索到的关键字
 *    在关键字数组patterns中的index位置以及在待匹配文本text中的偏移值按搜
 *    索到的先后顺序依次保存到数组matched_items.
 *
 * 参数说明:
 *    pattern_tree *ptree   : 指向ACTree的指针
 *    unsigned char *text   : 带匹配的文本
 *    int text_len          : 带匹配文本的长度
 *    matched_info_t matched_items[] : 保存搜索结果信息的数组
 *    int nmax_index        : 可保存匹配结果的最多个数
cur_index             : 用来匹配移动右移
 *   base_index            : 用来不匹配移动左移
 * 返回值:  int
 *    nmatched  : 匹配到的关键字的个数
 ----------------------------------------------------------------------------*/
int acbm_search (pattern_tree *ptree,
        unsigned char *text,
        int text_len,
        matched_info_t matched_items[],
        int nmax_index)
{
    int nmatched = 0 ;
    register int base_index = 0, cur_index = 0 ;
    register int real_shift = 0, gs_shift = 0, bc_shift = 0 ;
    pattern_tree_node *node = NULL ;

    if (text_len < ptree->min_pattern_size)
    {
        goto ret ;
    }

    base_index = text_len - ptree->min_pattern_size ;

    while (base_index >= 0)
    {
        cur_index = base_index ;
        node = ptree->root ;

#ifdef DEBUG_SEARCH
        printf ("Checking pattern tree at %d...", base_index) ;
#endif

        while (NULL != node->childs[text[cur_index]])
        {
            node = node->childs[text[cur_index]] ;

            if (node->label >= 0)
            {
                // 匹配到一个关键字,保存到matched_items中
#ifdef DEBUG_SEARCH
                printf ("Matched(%d) ", node->label) ;
#endif
                matched_items[nmatched].pattern_i = node->label ;
                matched_items[nmatched].offset = base_index ;
#ifdef DEBUG_SEARCH
                printf ("%s\n", text + matched_items[nmatched].offset) ;
#endif
                nmatched++ ;
                if (nmatched == nmax_index)
                {
                    goto ret ;
                }
            }
            cur_index++ ;
            if (cur_index >= text_len)
            {
                break ;
            }
        }

        if (node->nchild > 0)
        {
            // 跳字符,由GSshift和BCshift决定跳字符的位数
            gs_shift = node->childs[node->one_child]->GSshift ;
            if (cur_index < text_len)
            {
                bc_shift = ptree->BCshift[text[cur_index]]+base_index-cur_index ;
                //BCshift在不匹配时看的是文本的那个不匹配字符，它的含义是不匹配的那个字符在树中离根节点的距离,
                //cur_index-base_index文本中不匹配位与根节点的相对距离
                //bc_shift得到的就是文本中不匹配的那个字符移动到树中这个字符对应位置需要移动的距离

            }
            else
            {
                bc_shift = 1 ;
            }
            real_shift = gs_shift > bc_shift ? gs_shift : bc_shift ; // 取大者跳转
            base_index -= real_shift ;
#ifdef DEBUG_SEARCH
            printf ("Failed, GSshift:%d, BCshift:%d Realshift%d", gs_shift, bc_shift, real_shift) ;
#endif
        }
        else
        {
            // 1个匹配成功结束后,继续往前面处理
            base_index-- ;
#ifdef DEBUG_SEARCH
            printf ("Matched, shift %d", 1) ;
#endif
        }
#ifdef DEBUG_SEARCH
        printf ("\n") ;
#endif
    }

ret:
    return nmatched ;
}
/*--------------------------------------------------------------------------
 * 函数名:
 *    match_resualt_printf
 *
 * 函数功能:
 *    按次序打印搜索到的关键字结果
 *
 * 参数说明:
 *    unsigned char *text    : 输入的待搜索的文本
 *    pattern_data *patterns : 关键字数组
 *    int npattern           : 关键字个数
 *    matched_info_t matched_items[] : 保存搜索结果信息的数组
 *    int nmatched        : 搜索到的关键字个数
 *
 * 返回值:  int
 *    0 : 安全返回,无意义
 ----------------------------------------------------------------------------*/
int match_resualt_printf (unsigned char *text,
        pattern_data *patterns,
        int npattern,
        matched_info_t matched_items[],
        int nmatched)
{
    int i = 0 ;
    printf ("\n--- key worlds ---\n") ;
    for (i = 0 ; i < npattern ; i++)
    {
        if ((i+1)%6 == 0)
        {
            printf ("\n") ;
        }
        printf ("%2d %s\n", i+1, (patterns + i)->data) ;
    }
    printf ("\n--- input text ---\n %s\n", text) ;
    printf ("           1         2         3         4         5         6         \n") ;
    printf (" 0123456789012345678901234567890123456789012345678901234567890123456789\n") ;
    printf ("\n--- match resualt ---\n") ;
    for (i = 0 ; i < nmatched ; i++)
    {
        printf ("offset in text is    : %i\n", matched_items[i].offset) ;
        printf ("index in patterns is : %i\n", matched_items[i].pattern_i) ;
        printf ("key word is          : %s\n\n", (patterns + matched_items[i].pattern_i)->data) ;
    }
    return 0 ;
}
// 主函数入口
void main()
{
    clock_t start,finish;
    int i=0;
    int n;
    int npattern ; // 关键字个数,随时注意改变
    int length ;
    int text_len;// 待匹配文本字串长度,动态改变
    int success = 0 ;
    int nmatched = 0 ;
    pattern_tree *ptree ;
    pattern_data *patterns ;
    int nmax_index = MAX_ITEMS ;
    struct _matched_info matched_items[MAX_ITEMS] ;
    unsigned char keyword[PATTERN_LEN]; //单词
    unsigned char text[PATTERN_LEN]; //模式串
    printf("scanf the number of words-->\n");
    scanf("%d", &n);
    npattern=n;
    printf("scanf the words-->\n");
    patterns = ( pattern_data *) malloc (sizeof(  pattern_data) * npattern);
    if (NULL == patterns)
    {
        printf ("error\n") ;
        exit (1) ;
    }
    while(n --) {
        scanf("%s", keyword);
        length=strlen(keyword);
        strcpy ((patterns + i)->data, keyword) ;
        //(patterns + i)->data=keyword;
        (patterns + i)->len=length;
        i++;
    }
    ptree = ( pattern_tree *) malloc (sizeof( pattern_tree));
    if (NULL == ptree)
    {
        printf ("error\n") ;
        exit (1) ;
    }
    printf("scanf the text string-->\n");
    scanf("%s", text);
    text_len=strlen(text);
    start=clock();
    ptree = acbm_init (patterns, npattern) ;
    //  ACtree_build(ptree,patterns,npattern);
    //  ACtree_print_tree (ptree) ;  // 测试使用
    ACtree_compute_BCshifts(ptree);
    nmatched = acbm_search (ptree, text, text_len, matched_items, nmax_index) ;
    // ACtree_print_tree (ptree) ; // 测试使用
    success = match_resualt_printf (text, patterns, npattern, matched_items, nmatched) ;

    acbm_clean (ptree) ;
    finish=clock();
    printf("%f",(double)(finish-start)/CLOCKS_PER_SEC);
}




