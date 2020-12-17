/**
 * @file netlink_driver.c
 * @brief netlink 测试
 * @author qigaohua, qigaohua168@163.com
 * @version 0.1
 * @date 2020-12-16
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <net/sock.h>
#include <linux/netlink.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("qigaohua");
MODULE_DESCRIPTION("netlink example");

struct sock *nlsk = NULL;
extern struct net init_net;

int send_usrmsg(int pid, int dir, const char *pbuf, uint16_t len)
{
    struct sk_buff *nl_skb;
    struct nlmsghdr *nlh;
    int ret;

    /* 创建sk_buff 空间 */
    nl_skb = nlmsg_new(len, GFP_ATOMIC);
    if(!nl_skb) {
        printk("netlink alloc failure\n");
        return -1;
    }

    /*
     * 设置netlink消息头部
     * @skb: socket buffer to store message in
     * @portid: netlink PORTID of requesting application
     * @seq: sequence number of message
     * @type: message type
     * @payload: length of message payload
     * @flags: message flags
     */
    nlh = nlmsg_put(nl_skb, 0, 0, dir, len, 0);
    if(nlh == NULL) {
        printk("nlmsg_put failaure \n");
        nlmsg_free(nl_skb);
        return -1;
    }
    NETLINK_CB(nl_skb).dst_group = 0;    /* not in mcast group */

    /* 拷贝数据发送 */
    memcpy(nlmsg_data(nlh), pbuf, len);

    /*
     * 单播消息，目标用户态pid
     * 需要特别注意的是: skb申请的空间会在这里面释放，
     * 所以不能重复调用此接口发送同一个skb，会造成严重后果
     */
    ret = netlink_unicast(nlsk, nl_skb, pid, MSG_DONTWAIT);
    if (ret < 0) {
        printk(KERN_ERR"netlink_unicast failed\n");
        return ret;
    }
    printk("kernel send msg: ret: %d msg_len: %d\tmsg:%s\n", ret, len, pbuf);

    return ret;
}


static void netlink_rcv_msg(struct sk_buff *skb)
{
    struct nlmsghdr *nlh = NULL;
    int len, umsg_len;
    char *umsg = NULL;
    char *kmsg = "hello users!!!";


    nlh = nlmsg_hdr(skb);
    len = skb->len;

    for (; NLMSG_OK(nlh, len); nlh = NLMSG_NEXT(nlh, len)) {
        umsg = NLMSG_DATA(nlh);
        umsg_len = nlh->nlmsg_len - NLMSG_LENGTH(0);

        // 若终端printk没有打印，用dmesg查看
        if (umsg > 0)
            printk("kernel recv msg: skb->len: %d\tnlh->nlmsg_len: %d\t msg_len:"
                    "%d\tmsg:%s\n", skb->len, nlh->nlmsg_len, umsg_len, umsg);

        send_usrmsg(nlh->nlmsg_pid, 0, kmsg, strlen(kmsg) + 1);
    }

}


int test_netlink_init(void)
{
    struct netlink_kernel_cfg cfg = {
        .input  = netlink_rcv_msg, /* set recv callback */
        .groups = 0,
    };

    /* create netlink socket */
    nlsk = (struct sock *)netlink_kernel_create(&init_net, NETLINK_USERSOCK, &cfg);
    if(nlsk == NULL) {
        printk("netlink_kernel_create error !\n");
        return -1;
    }
    printk("test_netlink_init\n");

    return 0;
}

void test_netlink_exit(void)
{
    if (nlsk){
        netlink_kernel_release(nlsk); /* release ..*/
        nlsk = NULL;
    }
    printk("test_netlink_exit!\n");
}

module_init(test_netlink_init);
module_exit(test_netlink_exit);
