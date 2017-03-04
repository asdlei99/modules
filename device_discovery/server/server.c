/**
 *  @brief  device discovery
 *  @date     20170227
 **/

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <net/route.h>
#ifdef __linux__
#include <linux/sockios.h>
#include <net/ethernet.h>
#define __KERNEL__
#include <linux/ethtool.h>
#undef __KERNEL__
#include <resolv.h>
#include <fcntl.h>
#include <linux/netlink.h>
#endif

#define MCAST_PORT      3702
#define MCAST_ADDR      "239.255.255.250"

#define BCAST_PORT      3703
#define BCAST_ADDR      "255.255.255.255"

#define ROUTE_FILE      "/proc/net/route"

#define RTF_UP      0x0001      /* Route usable.  */

static char cur_interface[32];

/*
   Kernel IP routing table
   Destination     Gateway         Genmask         Flags Metric Ref    Use Iface
   169.254.0.0     *               255.255.0.0     U     204    0        0 wlan0
   169.254.0.0     *               255.255.0.0     U     205    0        0 wlan1
   192.168.0.0     *               255.255.255.0   U     0      0        0 wlan1
   224.0.0.0       *               240.0.0.0       U     0      0        0 wlan0


   Iface    Destination Gateway     Flags   RefCnt  Use Metric  Mask        MTU Window  IRTT
   wlan0    0000FEA9    00000000    0001    0   0   204 0000FFFF    0   0   0
   wlan1    0000FEA9    00000000    0001    0   0   205 0000FFFF    0   0   0
   wlan1    0000A8C0    00000000    0001    0   0   0   00FFFFFF    0   0   0
   wlan0    000000E0    00000000    0001    0   0   0   000000F0    0   0   0
   */

static int get_default_iface(char *iface)
{
    signed char devname[64];
    unsigned long d = 0;
    unsigned long g = 0;
    unsigned long m = 0;
    int r = 0;
    int flgs = 0;
    int ref = 0;
    int use = 0;
    int metric = 0;
    int mtu = 0;
    int win = 0;
    int ir = 0;
    int ret = -1;
    FILE *fp = NULL;

    memset(devname, 0, sizeof(devname));
    fp = fopen(ROUTE_FILE, "r");
    if(NULL == fp)
    {
        printf("open file failed %d\n", errno);
        return -1;
    }

    if (fscanf(fp, "%*[^\n]\n") < 0)
    { /* Skip the first line. */
        fclose(fp);
        return -1;        /* Empty or missing line, or read error. */
    }

    while (1)
    {
        r = fscanf(fp, "%63s%lx%lx%X%d%d%d%lx%d%d%d\n",
                devname, &d, &g, &flgs, &ref, &use, &metric, &m, &mtu, &win, &ir);

        if (r != 11)
        {
            //printf("get_gateway fscanf error and r=%d,errno=%d\n", r, errno);
            break;
        }

        /* Skip interfaces that are down. */
        if (!(flgs & RTF_UP))
            continue;

        /* skip destination 169.254.0.0 */
        if ((d & 0xff) == 0xA9)
            continue;

        /* skip 224.0.0.0 or 239.0.0.0 */
#if 0
        if ((d & 0xff) == 0xE0 || (d & 0xff) == 0xDF)
            continue;
#endif

        /* must be 0.0.0.0 to be default */
        if (d != 0)
            continue;

        ret = 1;
        break;
    }

    fclose(fp);

    if (ret == 1)
        strcpy(iface, (char*)devname);

    return 0;
}

static int sendudp_enable_broadcast(int skt)
{
    int enable = 1;
    return setsockopt(skt, SOL_SOCKET, SO_BROADCAST, &enable, sizeof(enable));
}

static int bcast_create_socket(void)
{
    struct sockaddr_in localaddr;
    int ret;
    int s;

    s = socket(AF_INET,SOCK_DGRAM,0);
    if(s < 0){
        perror("socket error");
        return -1;
    }

    /* enable broadcast */
    sendudp_enable_broadcast(s);

    localaddr.sin_family = AF_INET;
    localaddr.sin_port = htons(BCAST_PORT);
    localaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    ret = bind(s,(struct sockaddr*)&localaddr,sizeof(localaddr));
    if(ret < 0){
        close(ret);
        perror("bcast bind error");
        return -1;
    }

    return s;
}

static void bcast_close_socket(int s)
{
    close(s);
}

static int mcast_create_socket(void)
{
    struct sockaddr_in localaddr;
    int ret;
    int s;
    int ttl = 10;
    int loop = 0;

    s = socket(AF_INET,SOCK_DGRAM,0);
    if(s < 0){
        perror("socket error");
        return -1;
    }

    localaddr.sin_family = AF_INET;
    localaddr.sin_port = htons(MCAST_PORT);
    localaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    ret = bind(s,(struct sockaddr*)&localaddr,sizeof(localaddr));
    if(ret < 0){
        perror("mcast bind error");
        close(s);
        return -1;
    }

    if(setsockopt(s,IPPROTO_IP,IP_MULTICAST_TTL,&ttl,sizeof(ttl)) < 0){
        perror("IP_MULTICAST_TTL");
        close(s);
        return -1;
    }

    if(setsockopt(s,IPPROTO_IP,IP_MULTICAST_LOOP,&loop,sizeof(loop)) < 0){
        perror("IP_MULTICAST_LOOP");
        close(s);
        return -1;
    }

    return s;
}

static int mcast_add_memship(int mcast_skt)
{
    struct ip_mreq mreq;
    int ret;
    int s;

    s = mcast_skt;

    printf("Active interface change to %s\n", cur_interface);
    printf("mcast leave....\n");

    memset(&mreq, 0, sizeof(mreq));
    mreq.imr_multiaddr.s_addr = inet_addr(MCAST_ADDR);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    ret = setsockopt(s, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq));
    if (ret) {
        perror("IP_DROP_MEMBERSHIP");
    }

    memset(&mreq, 0, sizeof(mreq));
    mreq.imr_multiaddr.s_addr = inet_addr(MCAST_ADDR);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    ret = setsockopt(s, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
    if (ret) {
        perror("IP_ADD_MEMBERSHIP");
    }

    return -1;
}

static void mcast_close_socket(int s)
{
    struct ip_mreq mreq;
    int ret;

    memset(&mreq, 0, sizeof(mreq));
    mreq.imr_multiaddr.s_addr = inet_addr(MCAST_ADDR);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    ret = setsockopt(s,IPPROTO_IP,IP_DROP_MEMBERSHIP,&mreq,sizeof(mreq));
    if(ret < 0){
        perror("IP_DROP_MEMBERSHIP");
    }
    close(s);
}

static int discovery_handle(int s)
{
    struct sockaddr_in fromaddr;
    char buffer[2048];
    char *backbuff = "this is server";
    socklen_t len = sizeof(fromaddr);
    int size;
    int try_times = 1;

    memset(buffer,0,sizeof(buffer));

    size = recvfrom(s, buffer, 2048, 0, (struct sockaddr*)&fromaddr, &len);
    if(size < 0){
        perror("recvfrom ");
        return -1;
    }
        printf("Discovery message : %s\n", buffer);
        printf("Discovery message from : %s\n", inet_ntoa(fromaddr.sin_addr));
    if (strstr(buffer, "this is client")) {
        printf("Discovery message : %s\n", buffer);
        printf("Discovery message from : %s\n", inet_ntoa(fromaddr.sin_addr));

        while (try_times--) {
            sendto(s, backbuff, strlen(backbuff), 0, (struct sockaddr*)&fromaddr, sizeof(fromaddr));
        }
    }

    return 0;
}

int disocvery_server(void)
{
    int mcast_skt;
    int bcast_skt;
    int s;
    int ret;
    char old_interface[32];
    struct timeval  select_timeout;
    fd_set read_set;
    unsigned int times = 0;

    memset(old_interface, 0, sizeof(old_interface));
    memset(cur_interface, 0, sizeof(cur_interface));

    mcast_skt = mcast_create_socket();
    bcast_skt = bcast_create_socket();

    if (mcast_skt > bcast_skt)
        s = mcast_skt;
    else
        s = bcast_skt;

    while(1){
        select_timeout.tv_sec = 5;
        select_timeout.tv_usec = 0;

        FD_ZERO(&read_set);
        FD_SET(mcast_skt, &read_set);
        FD_SET(bcast_skt, &read_set);

        /* check if default iface exist */
        cur_interface[0] = '\0';
        get_default_iface(cur_interface);
        if (cur_interface[0] != '\0' && strcmp(cur_interface, old_interface) != 0) {
            mcast_add_memship(mcast_skt);
            strcpy(old_interface, cur_interface);
        }

        ret = select((s+1), &read_set, NULL, NULL, &select_timeout);
        if(ret <= 0) {
            continue;
        }

        if (FD_ISSET(mcast_skt, &read_set)) {
            //printf("Recevied Mcast Discovery Message....\n");
            discovery_handle(mcast_skt);
        } else if (FD_ISSET(bcast_skt, &read_set)) {
            //printf("Recevied Bcast Discovery Message....\n");
            discovery_handle(bcast_skt);
        }
    }

    mcast_close_socket(mcast_skt);
    bcast_close_socket(bcast_skt);
    return 0;
}

int main(int argc,char*argv[])
{
    disocvery_server();

    return 0;
}

