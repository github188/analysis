#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <linux/if_arp.h>
#include <errno.h>


typedef unsigned char byte_t;
typedef int bool_t;
static int const FALSE = 0;
static int const TRUE = 1;

#define MIN_ETH_FRAME_LEN       64 // 最小以太网帧长度
#define ETH_CHECK_CODE_LEN      4 // 4位校验码长度

// 以太网头部
#define MAC_ADDR_LEN            6 // mac地址长度
#define ETH_FRAME_TYPE_LEN      2 // 以太网帧类型长度
typedef struct {
    byte_t ma_dest_mac[MAC_ADDR_LEN]; // 目标mac地址
    byte_t ma_src_mac[MAC_ADDR_LEN]; // 源mac地址
    byte_t ma_frame_type[ETH_FRAME_TYPE_LEN]; // 以太网帧类型
} eth_head_st;

// arp帧结构
#define ARP_HD_TYPE_LEN         2 // arp硬件类型长度，硬件类型指网络类型，
                                  // 0x0001表示硬件地址为以太网接口
#define ARP_PROTO_TYPE_LEN      2 // arp协议类型长度，0x0800表示
                                  // 高层协议为ip协议
#define ARP_OP_TYPE_LEN         2 // arp操作方式长度
#if !defined(MAC_ADDR_LEN)
    #define MAC_ADDR_LEN        6 // mac地址长度
#endif // MAC_ADDR_LEN
#define IP_ADDR_LEN             4 // ip地址长度
typedef struct {
    byte_t ma_hd_type[ARP_HD_TYPE_LEN]; // 硬件网络类型
    byte_t ma_proto_type[ARP_PROTO_TYPE_LEN]; // arp协议类型
    byte_t m_hd_addr_len; // 硬件地址长度，单位为字节，mac为6字节，48位
    byte_t m_proto_addr_len; // 高层协议地址长度，单位为字节，ip为4字节，32位
    byte_t ma_op_type[ARP_OP_TYPE_LEN]; // arp操作方式
    byte_t ma_from_mac[MAC_ADDR_LEN]; // 发送方mac地址
    byte_t ma_from_ip[IP_ADDR_LEN]; // 发送方ip地址
    byte_t ma_to_mac[MAC_ADDR_LEN]; // 接收方mac地址
    byte_t ma_to_ip[IP_ADDR_LEN]; // 接收方ip地址
} arp_frame_st;


// arp包结构
#define ARP_FILL_LEN            (MIN_ETH_FRAME_LEN - ETH_CHECK_CODE_LEN \
                                     - sizeof(eth_head_st) \
                                     - sizeof(arp_frame_st))
typedef struct {
    eth_head_st m_eth_head; // 以太网头
    arp_frame_st m_arp_frame; // arp帧
    byte_t m_blank[ARP_FILL_LEN];
} arp_packet_st;


// is_net_endian
// 功能：判断主机字节序是否为网络字节序
bool_t is_net_endian(void)
{
    bool_t rslt = FALSE;
    union {
        int x;
        char y[sizeof(int)];
    } eyes = {
        .y = {'L', 'Z', 'S', 'B'},
    };

    if ('L' == *((char *)&eyes.x)) {
        rslt = FALSE;
    } else {
        rslt = TRUE;
    }

    return rslt;
}

// hton
// 功能：转换网络字节序
void hton(void *p_buf, size_t n)
{
    byte_t *p = (byte_t *)p_buf;
    int MAX_SWAP = n / 2;

    assert(NULL != p_buf);
    assert(n > 0);

    if (!is_net_endian()) {
        int i = 0;
        int j = 0;

        for (i = 0; i < MAX_SWAP; ++i) {
            j = n - i - 1;
            p[i] ^= p[j];
            p[j] ^= p[i];
            p[i] ^= p[j];
        }
    }
}

// send_arp
// 功能：发送arp包
int send_arp(byte_t dmac[],
             byte_t dip[],
             byte_t smac[],
             byte_t sip[])
{
    static byte_t const ETH_FRAME_TYPE[] = {
        0x06, 0x08,
    };
    static byte_t const ARP_HD_TYPE[] = {
        0x01, 0x00,
    };
    static byte_t const ARP_PROTO_TYPE[] = {
        0x00, 0x08,
    };
    static byte_t const ARP_OP_TYPE[] = {
        ARPOP_REQUEST & 0xFF,
        (ARPOP_REQUEST >> 8) & 0xFF,
    };
    int rslt = 0;
    int fd = 0;
    arp_packet_st arp_packet = {
        {
            {0},
        },
        {
            {0},
        },
    };

    assert(NULL != dmac);
    assert(NULL != dip);
    assert(NULL != smac);
    assert(NULL != sip);

    // 创建套接字
    fd = socket(AF_INET, SOCK_PACKET, htons(ETH_P_ARP));
    if (-1 == fd) {
        perror("[ERROR] socket error");
        rslt = -1;
        goto SOCKET_ERR;
    }

    // arp包结构初始化，以太头
    memcpy(arp_packet.m_eth_head.ma_dest_mac, dmac, MAC_ADDR_LEN);
    hton(arp_packet.m_eth_head.ma_dest_mac, MAC_ADDR_LEN);
    memcpy(arp_packet.m_eth_head.ma_src_mac, smac, MAC_ADDR_LEN);
    hton(arp_packet.m_eth_head.ma_src_mac, MAC_ADDR_LEN);
    memcpy(arp_packet.m_eth_head.ma_frame_type,
           &ETH_FRAME_TYPE, ETH_FRAME_TYPE_LEN);
    hton(arp_packet.m_eth_head.ma_frame_type, ETH_FRAME_TYPE_LEN);

    // arp包结构初始化，arp帧
    memcpy(arp_packet.m_arp_frame.ma_hd_type, ARP_HD_TYPE, ARP_HD_TYPE_LEN);
    hton(arp_packet.m_arp_frame.ma_hd_type, ARP_HD_TYPE_LEN);

    memcpy(arp_packet.m_arp_frame.ma_proto_type,
           ARP_PROTO_TYPE, ARP_PROTO_TYPE_LEN);
    hton(arp_packet.m_arp_frame.ma_proto_type, ARP_HD_TYPE_LEN);

    arp_packet.m_arp_frame.m_hd_addr_len = MAC_ADDR_LEN;
    arp_packet.m_arp_frame.m_proto_addr_len = IP_ADDR_LEN;

    memcpy(arp_packet.m_arp_frame.ma_op_type, ARP_OP_TYPE, ARP_OP_TYPE_LEN);
    hton(arp_packet.m_arp_frame.ma_op_type, ARP_OP_TYPE_LEN);

    memcpy(arp_packet.m_arp_frame.ma_from_mac, smac, MAC_ADDR_LEN);
    hton(arp_packet.m_arp_frame.ma_from_mac, MAC_ADDR_LEN);
    memcpy(arp_packet.m_arp_frame.ma_from_ip, sip, IP_ADDR_LEN);
    hton(arp_packet.m_arp_frame.ma_from_ip, IP_ADDR_LEN);

    memcpy(arp_packet.m_arp_frame.ma_to_mac, dmac, MAC_ADDR_LEN);
    hton(arp_packet.m_arp_frame.ma_to_mac, MAC_ADDR_LEN);
    memcpy(arp_packet.m_arp_frame.ma_to_ip, dip, IP_ADDR_LEN);
    hton(arp_packet.m_arp_frame.ma_to_ip, IP_ADDR_LEN);

    while (1) {
        struct sockaddr target;
        int n = 0;

        strcpy(target.sa_data, "vboxnet0");
        n = sendto(fd, &arp_packet, sizeof(arp_packet_st), 0,
                       (struct sockaddr *)&target, sizeof(target));

        if (-1 == n) {
            fprintf(stderr, "[%d]", errno);
            fprintf(stderr, "[%d]", ENOTCONN);
            perror("[ERROR] sendto failed");
            rslt = -1;
            break;
        }

        sleep(1);
    }

    // 错误处理
    do {
        close(fd);
    SOCKET_ERR:
        break;
    } while (0);

    return rslt;
}

int main(int argc, char *argv[])
{
    int rslt = 0;
    char x = 0x12;

    fprintf(stdout, "net endian? %s\n", is_net_endian() ? "yes" : "no");
    fprintf(stdout, "0x%08x : ", x);
    hton(&x, sizeof(x));
    fprintf(stdout, "0x%08x\n", x);
    fprintf(stdout, "sizeof eth_head_st: %d\n", sizeof(eth_head_st));
    fprintf(stdout, "sizeof arp_frame_st: %d\n", sizeof(arp_frame_st));
    fprintf(stdout, "sizeof arp_packet_st: %d\n", sizeof(arp_packet_st));

    do {
        byte_t smac[MAC_ADDR_LEN] = {
            //0x4E, 0x55, 0xBB, 0x27, 0x00, 0x08,
            0xFF, 0xEE, 0xDD, 0xCC, 0xBB, 0xAA,
        };
        byte_t sip[IP_ADDR_LEN] = {
            //0x65, 0x38, 0xA8, 0xC0,
            0x01, 0x38, 0xA8, 0xC0,
        };
        byte_t dmac[MAC_ADDR_LEN] = {
            //0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
            0x4E, 0x55, 0xBB, 0x27, 0x00, 0x08,
        };
        byte_t dip[IP_ADDR_LEN] = {
            0x65, 0x38, 0xA8, 0xC0,
        };

        send_arp(dmac, dip, smac, sip);
    } while (0);

    return rslt;
}
