#include <stdio.h>
#include <pcap.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <net/ethernet.h>
#include <time.h>

#define COLOR_RESET   "\033[0m"
#define COLOR_BLACK   "\033[30m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_WHITE   "\033[37m"
#define COLOR_BOLD    "\033[1m"


void packet_handler(u_char *user_data, const struct pcap_pkthdr *pkthdr, const u_char *packet) {
    // 时间格式化
    struct tm *ltime;
    char timestr[16];
    time_t local_tv_sec = pkthdr->ts.tv_sec;
    ltime = localtime(&local_tv_sec);
    strftime(timestr, sizeof timestr, "%H:%M:%S", ltime);
    int milliseconds = pkthdr->ts.tv_usec / 1000;

    struct ether_header *eth_header = (struct ether_header *)packet;
    uint16_t eth_type = ntohs(eth_header->ether_type);

    char src_ip[INET6_ADDRSTRLEN] = "";
    char dst_ip[INET6_ADDRSTRLEN] = "";
    int src_port = 0, dst_port = 0;
    char proto_str[8] = "";
    char desc[64] = "";

    int is_ipv6_addr = (eth_type == ETHERTYPE_IPV6);

    const u_char *transport_header = NULL;

    if (eth_type == ETHERTYPE_IP) {
        // IPv4
        struct ip *ip_header = (struct ip *)(packet + sizeof(struct ether_header));
        inet_ntop(AF_INET, &(ip_header->ip_src), src_ip, sizeof(src_ip));
        inet_ntop(AF_INET, &(ip_header->ip_dst), dst_ip, sizeof(dst_ip));

        int ip_header_len = ip_header->ip_hl * 4;
        transport_header = packet + sizeof(struct ether_header) + ip_header_len;

        if (ip_header->ip_p == IPPROTO_TCP) {
            struct tcphdr *tcp_header = (struct tcphdr *)transport_header;
            src_port = ntohs(tcp_header->source);
            dst_port = ntohs(tcp_header->dest);
            snprintf(proto_str, sizeof(proto_str), "TCP");
            snprintf(desc, sizeof(desc), "Seq=%u Ack=%u",
                     ntohl(tcp_header->seq), ntohl(tcp_header->ack_seq));
        } else if (ip_header->ip_p == IPPROTO_UDP) {
            struct udphdr *udp_header = (struct udphdr *)transport_header;
            src_port = ntohs(udp_header->source);
            dst_port = ntohs(udp_header->dest);
            snprintf(proto_str, sizeof(proto_str), "UDP");
            snprintf(desc, sizeof(desc), "Len=%d", ntohs(udp_header->len));
        } else if (ip_header->ip_p == IPPROTO_ICMP) {
            snprintf(proto_str, sizeof(proto_str), "ICMP");
            snprintf(desc, sizeof(desc), "Type=%d Code=%d",
                     transport_header[0], transport_header[1]);
        } else {
            snprintf(proto_str, sizeof(proto_str), "IP");
        }

    } else if (eth_type == ETHERTYPE_IPV6) {
        // IPv6
        struct ip6_hdr *ip6_header = (struct ip6_hdr *)(packet + sizeof(struct ether_header));
        inet_ntop(AF_INET6, &(ip6_header->ip6_src), src_ip, sizeof(src_ip));
        inet_ntop(AF_INET6, &(ip6_header->ip6_dst), dst_ip, sizeof(dst_ip));

        int ip6_header_len = sizeof(struct ip6_hdr);
        transport_header = packet + sizeof(struct ether_header) + ip6_header_len;

        uint8_t next_header = ip6_header->ip6_nxt;

        if (next_header == IPPROTO_TCP) {
            struct tcphdr *tcp_header = (struct tcphdr *)transport_header;
            src_port = ntohs(tcp_header->source);
            dst_port = ntohs(tcp_header->dest);
            snprintf(proto_str, sizeof(proto_str), "TCP");
            snprintf(desc, sizeof(desc), "Seq=%u Ack=%u",
                     ntohl(tcp_header->seq), ntohl(tcp_header->ack_seq));
        } else if (next_header == IPPROTO_UDP) {
            struct udphdr *udp_header = (struct udphdr *)transport_header;
            src_port = ntohs(udp_header->source);
            dst_port = ntohs(udp_header->dest);
            snprintf(proto_str, sizeof(proto_str), "UDP");
            snprintf(desc, sizeof(desc), "Len=%d", ntohs(udp_header->len));
        } else if (next_header == IPPROTO_ICMPV6) {
            snprintf(proto_str, sizeof(proto_str), "ICMPv6");
            snprintf(desc, sizeof(desc), "Type=%d Code=%d",
                     transport_header[0], transport_header[1]);
        } else {
            snprintf(proto_str, sizeof(proto_str), "IPv6");
        }

    } else {
        // 其他协议可以选择忽略或打印简要信息
        return; // 这里不打印非 IP/IPv6 包
    }

    printf(COLOR_GREEN   "%s.%03d  "    COLOR_RESET
           COLOR_BLUE    "%s " COLOR_BOLD "%d  "          COLOR_RESET
           COLOR_CYAN    "%s " COLOR_BOLD "%d  "          COLOR_RESET
           COLOR_YELLOW  "%s  "          COLOR_RESET
           COLOR_MAGENTA "%d  "          COLOR_RESET
           "%s\n",
           timestr, milliseconds,
           src_ip, src_port,
           dst_ip, dst_port,
           proto_str,
           pkthdr->caplen,
           desc);

}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "用法: %s <网络接口> <过滤规则>\n", argv[0]);
        return 1;
    }

    char *dev = argv[1];  // 网络接口，例如 eth0
    char *filter_exp = argv[2];  // 用户自定义的过滤规则

    // 打开设备进行抓包
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t *handle = pcap_open_live(dev, BUFSIZ, 1, 1000, errbuf);
    if (handle == NULL) {
        fprintf(stderr, "无法打开设备 %s: %s\n", dev, errbuf);
        return 1;
    }

    // 编译过滤规则
    struct bpf_program fp;
    if (pcap_compile(handle, &fp, filter_exp, 0, PCAP_NETMASK_UNKNOWN) == -1) {
        fprintf(stderr, "无法编译过滤规则 %s: %s\n", filter_exp, pcap_geterr(handle));
        return 1;
    }

    // 设置过滤器
    if (pcap_setfilter(handle, &fp) == -1) {
        fprintf(stderr, "无法设置过滤器 %s: %s\n", filter_exp, pcap_geterr(handle));
        return 1;
    }

    // 开始抓包，并且每次捕获到数据包时调用 packet_handler
    if (pcap_loop(handle, 0, packet_handler, NULL) < 0) {
        fprintf(stderr, "抓包失败: %s\n", pcap_geterr(handle));
        return 1;
    }

    // 关闭抓包会话
    pcap_close(handle);
    return 0;
}
