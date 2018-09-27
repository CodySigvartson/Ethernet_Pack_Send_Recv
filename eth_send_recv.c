/********************************
Programmer	Last Modified		Description
---------	-------------		---------------
Cody Sigvartson	9_20_18			Initial development
Cody Sigvartson 9_27_18			Fixed IPv4 Packet type bug

Program description:
This program builds and sends an ethernet frames over sockets. This is a simple ethernet frame
with no error detection (ie. CRC)
********************************/

#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/ether.h>

#define BUF_SIZ		65536
#define SEND 0
#define RECV 1

void send_message(char if_name[], struct sockaddr_ll sk_addr, char hw_addr[], char payload[]){
	// create socket
	int sockfd = -1;
	if((sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL)))<0){
		perror("socket() failed!");
	}

	printf("Sending with sockfd: %d\n",sockfd);

	// connect to interface name
	struct ifreq if_hwaddr;
	memset(&if_hwaddr,0,sizeof(struct ifreq));
	strncpy(if_hwaddr.ifr_name, if_name, IFNAMSIZ-1);
	if(ioctl(sockfd, SIOCGIFHWADDR, &if_hwaddr) < 0){
		perror("SIOCGIFHWADDR");
	}

	// build ethernet frame
	struct ether_header frame;
	memset(&frame,0,sizeof(struct ether_header));
	memcpy(frame.ether_dhost, hw_addr, 6);
	memcpy(frame.ether_shost, if_hwaddr.ifr_hwaddr.sa_data, 6);
	frame.ether_type = htons(ETH_P_IP);


	struct ifreq if_idx;
	memset(&if_idx,0,sizeof(struct ifreq));
	strncpy(if_idx.ifr_name, if_name, IFNAMSIZ-1);
	if(ioctl(sockfd, SIOCGIFINDEX, &if_idx) < 0){
		perror("SIOCGIFINDEX");
	}

	// pack frame header
	unsigned char buff[BUF_SIZ];
	char *eth_header = (char *)&frame;
	strncpy(buff,eth_header,strlen(eth_header)+1);
	strncat(&buff[14],payload,strlen(payload)+1);


	sk_addr.sll_ifindex = if_idx.ifr_ifindex;
	sk_addr.sll_halen = ETH_ALEN;
	int byteSent = sendto(sockfd, buff, strlen(payload)+strlen(eth_header)+1, 0, (struct sockaddr*)&sk_addr, sizeof(struct sockaddr_ll));
	printf("%d bytes sent!\n", byteSent);
}

void recv_message(char if_name[], struct sockaddr_ll sk_addr){
	int sockfd = -1;
	if((sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL)))<0){
		perror("socket() failed!");
	}

	struct ifreq if_idx;
	memset(&if_idx,0,sizeof(struct ifreq));
	strncpy(if_idx.ifr_name, if_name, IFNAMSIZ-1);
	if(ioctl(sockfd, SIOCGIFINDEX, &if_idx) < 0){
		perror("SIOCGIFINDEX");
	}

	if(ioctl(sockfd, SIOCGIFHWADDR, &if_idx) < 0){
		perror("SIOCGIFINDEX");
	}

	unsigned char buff[BUF_SIZ];
	int sk_addr_size = sizeof(struct sockaddr_ll);
	printf("Receiving...\n");
	int recvLen = recvfrom(sockfd, buff, BUF_SIZ, 0 , (struct sockaddr*)&sk_addr, &sk_addr_size);
	
	printf("%d bytes received!\n", recvLen);

	unsigned char src_mac[6];
	memcpy(src_mac, &buff[6], 6);
	
	unsigned char payload[BUF_SIZ];
	memcpy(payload, &buff[14], BUF_SIZ-sizeof(struct ether_header));

	printf("Message: %s\n",payload);
	printf("Source MAC: [%X][%X][%X][%X][%X][%X]\n",src_mac[0],src_mac[1],src_mac[2],src_mac[3],src_mac[4],src_mac[5]);
}

int main(int argc, char *argv[])
{
	int mode;
	char buff[BUF_SIZ];
	char hw_addr[6];
	char interfaceName[IFNAMSIZ];
	memset(buff, 0, BUF_SIZ);
	
	int correct=0;
	if (argc > 1){
		if(strncmp(argv[1],"Send", 4)==0){
			if (argc == 5){
				mode=SEND; 
				sscanf(argv[3], "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &hw_addr[0], &hw_addr[1], &hw_addr[2], &hw_addr[3], &hw_addr[4], &hw_addr[5]);
				strncpy(buff, argv[4], BUF_SIZ);
				printf("Sending payload: %s\n", buff);
				correct=1;
			}
		}
		else if(strncmp(argv[1],"Recv", 4)==0){
			if (argc == 3){
				mode=RECV;
				correct=1;
			}
		}
		strncpy(interfaceName, argv[2], IFNAMSIZ);
	 }
	 if(!correct){
		fprintf(stderr, "./455_proj2 Send <InterfaceName>  <DestHWAddr> <Message>\n");
		fprintf(stderr, "./455_proj2 Recv <InterfaceName>\n");
		exit(1);
	 }

	struct sockaddr_ll sk_addr;
	memset(&sk_addr, 0, sizeof(struct sockaddr_ll));


	if(mode == SEND){
		send_message(interfaceName, sk_addr, hw_addr, buff);
	}
	else if (mode == RECV){
		recv_message(interfaceName, sk_addr);
	}

	return 0;
}
