#include "../include/global.h"
#include "../include/logger.h"

char* get_ip(void);
int sort_list();
int check_if_login(char ip[INET_ADDRSTRLEN]);
int check_if_block(char ip[INET_ADDRSTRLEN],char sender_ip[INET_ADDRSTRLEN]);
int search_client_by_ip(char ip[INET_ADDRSTRLEN]);
int isValidAddr(char addr[INET_ADDRSTRLEN], char port[CMD_SIZE]);

char* get_ip(void)
{
       int s;
       struct ifconf conf;
       struct ifreq *ifr;
       char buff[BUFFER_SIZE];
       int num;
       int i;

       s = socket(PF_INET, SOCK_DGRAM, 0);
       conf.ifc_len = BUFFER_SIZE;
       conf.ifc_buf = buff;

       ioctl(s, SIOCGIFCONF, &conf);
       num = conf.ifc_len / sizeof(struct ifreq);
       ifr = conf.ifc_req;

       for(i=0;i < num;i++)
       {
            struct sockaddr_in *sin = (struct sockaddr_in *)(&ifr->ifr_addr);
            ioctl(s, SIOCGIFFLAGS, ifr);
           if(((ifr->ifr_flags & IFF_LOOPBACK) == 0) && (ifr->ifr_flags & IFF_UP))
            {
                char* ipaddr = inet_ntoa(sin->sin_addr);
                return ipaddr;
            }
            ifr++;
       }
}

int isValidAddr(char addr[INET_ADDRSTRLEN], char port[CMD_SIZE]){
    int ret = 1;
    for(int i=0; i<CMD_SIZE;i++){
        if(addr[i] == '\0') break;
        if(addr[i] == '.') continue;
        int t = addr[i] - '0';
        if(t<0 || t>9) {
            ret = 0;
            break;
        }
    }

    for(int i=0; i<CMD_SIZE;i++){
        if(port[i] == '\0') break;
        int t = port[i] - '0';
        if(t<0 || t>9) {
            ret = 0;
            break;
        }
    }
    return ret;
}

int check_if_block(char ip[INET_ADDRSTRLEN],char sender_ip[INET_ADDRSTRLEN]){

	for(int i = 0; i< client_num; i++){
		
		if(strcmp(clients[i].ip_addr,ip) == 0){

			for (int j = 0; j < clients[i].block_num; i++){

				if(strcmp(clients[i].blockedList[j],sender_ip) == 0){
					return 1;
					break;
				}

			}
			break;
		}


	}
	return 0;
}

int check_if_login(char ip[INET_ADDRSTRLEN]){

	for(int i = 0; i< client_num; i++){
		
		if(strcmp(clients[i].ip_addr,ip) == 0 && clients[i].login_status == 1){
			return 1;
			break;
		}
		
	}
	return 0;
}

int sort_list(){
	struct client temp;
	int temp_no;
    for (int i = 0; i < client_num; i++){
    	for (int j = i + 1; j < client_num; j++){
      		if(clients[i].port_no>clients[j].port_no){
       			temp = clients[i];
       			clients[i] = clients[j];
       			clients[j] = temp;
				clients[i].no = i+1;
				clients[j].no = j+1;
      		}
     	}
    }	
}

int search_client_by_ip(char ip[INET_ADDRSTRLEN]){
	int socket = 0;
	for(int i = 0; i< client_num; i++){
		
		if(strcmp(clients[i].ip_addr,ip) == 0){
			socket = clients[i].socket_id;
			return socket;
			break;
		}	
	}	
}