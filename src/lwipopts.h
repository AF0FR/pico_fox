#pragma once

#define NO_SYS                          1
#define LWIP_SOCKET                     0
#define LWIP_NETCONN                    0
#define LWIP_DHCP                       1
#define LWIP_DNS                        1
#define LWIP_TCP                        1
#define MEM_ALIGNMENT                   4
#define MEM_SIZE                        16000
#define MEMP_NUM_TCP_PCB                8
#define MEMP_NUM_TCP_PCB_LISTEN         4
#define TCP_MSS                         1460
#define TCP_WND                         (4 * TCP_MSS)
#define TCP_SND_BUF                     (4 * TCP_MSS)
#define TCP_QUEUE_OOSEQ                 1
#define LWIP_HTTPD_CGI                  1
#define LWIP_HTTPD_SSI                  1
#define LWIP_HTTPD_SSI_INCLUDE_TAG      0
#define LWIP_HTTPD_MAX_CGI_PARAMETERS   20
#define HTTPD_FSDATA_FILE               "pico_fsdata.inc"
#define LWIP_CHKSUM_ALGORITHM           3
#define LWIP_NETIF_HOSTNAME             1
#define LWIP_NETIF_STATUS_CALLBACK      1
#define LWIP_NETIF_LINK_CALLBACK        1
#define LWIP_STATS                      0
#define LWIP_PROVIDE_ERRNO              1
#define PICO_CYW43_ARCH_POLL             0
