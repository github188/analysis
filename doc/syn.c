#include <stdio.h>   
#include <stdlib.h>   
#include <string.h>   
#include <getopt.h>   
 
#include <unistd.h>
#include <netinet/ip.h>   
#include <netinet/tcp.h>   
  
/* 数据包最大长度，无负载 */  
#define MAX_PKG_SIZE 80 /* ip header = 20 ~ 40, tcp header = 20 ~ 40; max = 80 */   
/* 计算TCP校验和的最大使用长度 www.linuxidc.com*/  
#define MAX_PSD_SIZE 52 /* psd header = 12, tcp header = 20 ~ 40; max = 52 */   
/* 数据包构造参数 */  
struct st_args{  
    struct sockaddr_in saddr; /* 源地址 */  
    struct sockaddr_in daddr; /* 目的地址 */  
};  
  
/* 用于计算TCP校验和的伪头部 */  
struct psdhdr{  
    uint32_t saddr; /* ip头部源地址 */  
    uint32_t daddr; /* ip头部目的地址 */  
    uint8_t mbz; /* 补全字段，需为0 */  
    uint8_t protocol; /* ip头部协议 */  
    uint16_t tcpl; /* tcp长度，包括头部和数据部分 */  
};  
  
  
/* 
 * 采用汇编计算ip头部校验和 
 * @param[in]: iph, ip头指针;  
 * @param[in]: ihl, ip头长度(4的倍数) 
 * 
 * @return 16位校验和 
 * */  
static inline uint16_t ip_fast_csum(const void *iph, unsigned int ihl)  
{  
    unsigned int sum;  
  
    asm("  movl (%1), %0\n"  
        "  subl $4, %2\n"  
        "  jbe 2f\n"  
        "  addl 4(%1), %0\n"  
        "  adcl 8(%1), %0\n"  
        "  adcl 12(%1), %0\n"  
        "1: adcl 16(%1), %0\n"  
        "  lea 4(%1), %1\n"  
        "  decl %2\n"  
        "  jne  1b\n"  
        "  adcl $0, %0\n"  
        "  movl %0, %2\n"  
        "  shrl $16, %0\n"  
        "  addw %w2, %w0\n"  
        "  adcl $0, %0\n"  
        "  notl %0\n"  
        "2:"  
    /* Since the input registers which are loaded with iph and ihl 
       are modified, we must also specify them as outputs, or gcc 
       will assume they contain their original values. */  
        : "=r" (sum), "=r" (iph), "=r" (ihl)  
        : "1" (iph), "2" (ihl)  
        : "memory");  
    return (uint16_t)sum;  
}  
  
/* 
 * 计算校验和 
 * @param[in]: buffer, 待计算数据指针 
 * @param[in]: size, 数据长度 
 * 
 * @return 校验和 
 * */  
uint16_t csum(uint16_t *buffer, int size)  
{  
    unsigned long cksum = 0;  
  
    while(size>1)  
    {  
        cksum += *buffer++;  
        size -= sizeof(uint16_t);  
    }  
  
    if(size)  
    {  
        cksum += *(unsigned char*)buffer;  
    }  
  
    cksum = (cksum>>16) + (cksum&0xffff);  
    cksum += (cksum>>16);   
      
    return (uint16_t)(~cksum);  
}  
/* 
 * 调试用的函数，用于输出数据 
 * */  
void data_dump(uint8_t *pdata, int len)  
{  
    int i = 0;  
    printf("len = %d\n", len);  
    for(i = 0;  i < len; ++i)  
    {  
        printf("%02X ", *(pdata + i));  
        if((i + 1) % 4 == 0)  
            printf("\n");  
    }  
}  
  
/* 
 * 数据包发送函数，www.linuxidc.com只构造了ip头+tcp头大小的长度； 
 * ip头和tcp都无选项部分，tcp无负载数据 
 * @param[in]: parg, 构造数据包是采用的一些参数 
 *  
 * @return -1, 发送失败；0, 发送成功 
 * */  
int send_pkg(struct st_args* parg)  
{  
    uint8_t datagram[MAX_PKG_SIZE] = {0};  
    uint8_t psdheader[MAX_PSD_SIZE] = {0};  
      
    struct iphdr *iph = (struct iphdr*)datagram;  
    struct tcphdr *tcph = (struct tcphdr*)(datagram + sizeof(struct iphdr));  
    struct tcp_options *tcpopt = (struct tcp_options*)(datagram + sizeof(struct iphdr)  
                                 + sizeof(struct tcphdr));  
    struct psdhdr *psdh = (struct psdhdr*)psdheader;  
    struct tcphdr *tcph_psd = (struct tcphdr*)(psdheader + sizeof(struct psdhdr));  
      
    int sockfd = -1, ret = 0;  
    int optval = 1;  
    const int *poptval = &optval;  
      
    sockfd = socket(PF_INET, SOCK_RAW, IPPROTO_TCP);  
    if(sockfd < 0)  
    {  
        perror("create socket failed!\n");  
        goto err_out;  
    }  
      
    iph->ihl = 5; /* header length, 5 * 4 = 20 Bytes */  
    iph->version = 4;   /* version, ipv4 */  
    iph->tos = 0; /* type of service, gernarel */  
    iph->tot_len = sizeof(struct iphdr) + sizeof(struct tcphdr); /* total length, iph + tcph = 32 Bytes */  
    iph->id = htons(54321); /* identifcation */  
    iph->frag_off = htons(0x02 << 13); /* fragment offset field */  
    iph->ttl = 64; /* time to live */  
    iph->protocol = 6; /* protocol, tcp */  
    iph->check = 0; /* checksum */  
    iph->saddr = parg->saddr.sin_addr.s_addr; /* source address */  
    iph->daddr = parg->daddr.sin_addr.s_addr; /* dest address */  
      
    tcph->source = parg->saddr.sin_port; /* source port */  
    tcph->dest = parg->daddr.sin_port; /* dest port */  
    tcph->seq = random(); /* current sended packet sequence number */  
    tcph->ack_seq = 0; /* expect received next packet sequence number */  
    tcph->doff = sizeof(struct tcphdr) / 4; /* data position in the packet */  
    tcph->syn = 1; /* syn packet */  
    tcph->window = htons(65535); /* size of tcp window, FreeBSD uses this value */  
    tcph->check = 0; /* checksum */  
    tcph->urg_ptr = 0; /* urgent data position */  
      
    psdh->saddr = iph->saddr;  
    psdh->daddr = iph->daddr;  
    psdh->mbz = 0;  
    psdh->protocol = iph->protocol;  
    psdh->tcpl = htons(sizeof(struct tcphdr));  
    //data_dump(psdheader, sizeof(struct psdhdr));   
  
    memcpy(tcph_psd, tcph, sizeof(struct tcphdr));  
      
    tcph->check = csum((uint16_t*)psdheader, sizeof(struct psdhdr) + sizeof(struct tcphdr));  
    /* iph->check == 0时, 内核会自动计算校验和 */  
    //iph->check = ip_fast_csum(datagram, iph->ihl);   
  
    if(setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, poptval, sizeof(optval)) < 0)  
    {  
        perror("setsockopt failed!");  
        goto err_out;  
    }  
      
    ret = sendto(sockfd, datagram, iph->tot_len, 0,   
            (struct sockaddr*)&(parg->daddr), sizeof(parg->daddr));  
    if(ret < 0)  
    {  
        perror("sendto socket failed!");  
        goto err_out;  
    }  
  
    //data_dump(datagram, 40);   
    close(sockfd);  
    return 0;  
      
err_out:  
    if(sockfd != -1)  
        close(sockfd);  
    return -1;  
}  
  
int main(int argc, char **argv)  
{  
    struct st_args args;  
      
#define MAX_IP_SIZE 16   
    uint8_t sip[MAX_IP_SIZE] = "192.168.56.253";  
    uint8_t dip[MAX_IP_SIZE] = "192.168.56.101";  
    uint16_t sport = 55555;  
    uint16_t dport = 80;  
    int8_t arg = 0;  
  
    struct option lopts[] = {  
        {"saddr", required_argument, 0, 's'},  
        {"sport", required_argument, 0, 'p'},  
        {"daddr", required_argument, 0, 'd'},  
        {"dport", required_argument, 0, 'f'}  
    };  
    while((arg = getopt_long(argc, argv, "s:p:d:f:", lopts, NULL)) != -1)  
    {  
        switch(arg)  
        {  
        case 's':  
            memcpy(sip, optarg, MAX_IP_SIZE);  
            break;  
        case 'p':  
            sport = atoi(optarg);  
            break;  
        case 'd':  
            memcpy(dip, optarg, MAX_IP_SIZE);  
            break;  
        case 'f':  
            dport = atoi(optarg);  
            break;  
        default:  
            break;  
        }  
    }  
      
    memset(&args, 0, sizeof(struct st_args));  
    inet_pton(AF_INET, sip, (void*)&args.saddr.sin_addr);  
    args.saddr.sin_port = htons(sport);  
  
    inet_pton(AF_INET, dip, (void*)&args.daddr.sin_addr);  
    args.daddr.sin_port = htons(dport);  
  
    send_pkg(&args);  
    return 0;  
}  
