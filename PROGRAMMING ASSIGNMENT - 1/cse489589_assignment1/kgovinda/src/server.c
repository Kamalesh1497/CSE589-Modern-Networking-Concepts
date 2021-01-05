#include "../include/global.h"
#include "../include/logger.h"

int send_login_list(int client_socket);
int send_login_list_refresh(int client_socket);
int establish_server(int port_num);
int process_cmd_from_client(char *msg, int sock_index);
int add_to_clients(int fdaccept, struct sockaddr_in client_addr, int i, int client_port);

int establish_server(int port_num){

	printf("\nServer Running\n");
	int port, server_socket, head_socket, selret, sock_index, fdaccept = 0;
	int yes = 1;
	socklen_t caddr_len;
	struct sockaddr_in server_addr;
	struct client client_list[BACKLOG];
	

	// Socket
	server_socket = socket(AF_INET, SOCK_STREAM, 0);

    if(server_socket < 0)
		perror("Cannot create socket");

	port = port_num;
	memset(&server_addr, 0, sizeof(server_addr));

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);// the IP address of the machine on which the server is running
	server_addr.sin_port = htons(port);

	// Bind
	if(bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0 )
    	perror("Bind failed");

 	setsockopt(server_socket,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int));   

	// Listen
	if(listen(server_socket, BACKLOG) < 0)
    	perror("Unable to listen on port");
	
	/* Zero select FD sets */
    FD_ZERO(&master_list);
    FD_ZERO(&watch_list);
    
    /* Register the listening socket */
    FD_SET(server_socket, &master_list);
    /* Register STDIN */
    FD_SET(STDIN, &master_list);
	
	head_socket = server_socket;
	char* ip_addr;
	ip_addr = get_ip();
    while(TRUE){
        memcpy(&watch_list, &master_list, sizeof(master_list));
        /* select() system call. This will BLOCK */
        selret = select(head_socket + 1, &watch_list, NULL, NULL, NULL);
        if(selret < 0)
            perror("select failed.");

        /* Check if we have sockets/STDIN to process */
        if(selret > 0){
            /* Loop through socket descriptors to check which ones are ready */
            for(sock_index=0; sock_index<=head_socket; sock_index+=1){

                if(FD_ISSET(sock_index, &watch_list)){

                    /* Check if new command on STDIN */
                    if (sock_index == STDIN){
                    	char *cmd = (char*) malloc(sizeof(char)*CMD_SIZE);

                    	memset(cmd, '\0', CMD_SIZE);
						if(fgets(cmd, CMD_SIZE-1, stdin) == NULL) //Mind the newline character that will be written to cmd
							exit(-1);

						printf("\nServer got: %s\n", cmd);
						cmd[strlen(cmd)-1]='\0';
						
						//Process PA1 commands here ...
						if(strcmp(cmd, "AUTHOR") == 0){

							author(cmd);

						}
						else if(strcmp(cmd, "IP") == 0){

							ip(ip_addr, cmd);

						}
						else if(strcmp(cmd, "PORT") == 0){

							port(server_socket, cmd, port_num);

						}
						else if(strcmp(cmd, "LIST") == 0){
							cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
							for (int i = 0; i < client_num; i++){

								if(clients[i].login_status == 1){
						
									cse4589_print_and_log("%-5d%-35s%-20s%-8d\n",clients[i].no,clients[i].hostname,clients[i].ip_addr,clients[i].port_no);

								}
							}
							cse4589_print_and_log("[%s:END]\n", cmd);  
						}
						else if(strcmp(cmd, "STATISTICS") == 0){

							cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
							for (int i = 0; i < client_num; i++){
								char status[25];
								bzero(status,25);
								if(clients[i].socket_id > 0){
									if(clients[i].login_status == 1){
										strcpy(status,"logged-in");
									}
									else if(clients[i].login_status == 0){
										strcpy(status,"logged-out");
									}
									else{
										continue;
									}
									cse4589_print_and_log("%-5d%-35s%-8d%-8d%-8s\n", clients[i].no, clients[i].hostname, clients[i].send_msg_num, clients[i].recv_msg_num, status);
								}
							}
							cse4589_print_and_log("[%s:END]\n", cmd);  
						}
						else if(strncmp(cmd, "BLOCKED", 7)==0){

							char *p;
							p = strtok(cmd," ");
							if(p!=NULL){
								p = strtok(NULL," ");
							}else{
								cse4589_print_and_log("[%s:ERROR]\n", cmd);
								cse4589_print_and_log("[%s:END]\n", cmd);
								return 0;

							}
							char check_b_ip[INET_ADDRSTRLEN];
							strcpy(check_b_ip,p);
							struct client blocklist[15];
							int n = 0;
							
							for (int i = 0; i < client_num; i++){
								if(strcmp(clients[i].ip_addr,check_b_ip)==0){
									if(clients[i].block_num>0){
										for(int j = 0; j < clients[i].block_num;j++){
											printf("%s\n", clients[i].blockedList[j]);
											for (int m = 0; m < client_num; m++){
												if(strcmp(clients[m].ip_addr,clients[i].blockedList[j])==0){
													strcpy(blocklist[n].hostname, clients[m].hostname);
													strcpy(blocklist[n].ip_addr, clients[m].ip_addr);
													blocklist[n].port_no = clients[m].port_no;
													n++;
													
												}
											}
										}
									}
								}
								
							}
							struct client temp;
    						for (int i = 0; i < n; i++){
    							for (int j = i + 1; j < n; j++){
      								if(blocklist[i].port_no>blocklist[j].port_no){
       									temp = blocklist[i];
       									blocklist[i] = blocklist[j];
       									blocklist[j] = temp;
      
      								}
     							}
    						}
    						cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
    						for(int i = 0; i < n; i++){

    							cse4589_print_and_log("%-5d%-35s%-20s%-8d\n",i+1,blocklist[i].hostname,blocklist[i].ip_addr,blocklist[i].port_no);

    						}

							cse4589_print_and_log("[%s:END]\n", cmd);  

						}

						free(cmd);
                    }
                    /* Check if new client is requesting connection */
                    else if(sock_index == server_socket){
                    	char *buffer = (char*) malloc(sizeof(char)*BUFFER_SIZE);
						struct sockaddr_in client_addr;
                        caddr_len = sizeof(client_addr);
                        fdaccept = accept(server_socket, (struct sockaddr *)&client_addr, &caddr_len);
                        if(fdaccept < 0){
                            perror("Accept failed.");
						}
						printf("\nRemote Host connected!\n the fd is: %d\n",fdaccept);    
						 /* Add to watched socket list */
						FD_SET(fdaccept, &master_list);

						client_num = add_to_clients(fdaccept, client_addr, client_num, 0);
						
						free(buffer);
						if(fdaccept > head_socket) head_socket = fdaccept;
                    }
                    /* Read from existing clients */
                    else{
                        /* Initialize buffer to receieve response */
                        char *buffer = (char*) malloc(sizeof(char)*BUFFER_SIZE);
                        memset(buffer, '\0', BUFFER_SIZE);

                        if(recv(sock_index, buffer, BUFFER_SIZE, 0) <= 0){
                            close(sock_index);
                            printf("Remote Host terminated connection!\n");
                            client_num--;

                            /* Remove from watched list */
                            FD_CLR(sock_index, &master_list);
                        }
                        else {
                        	//Process incoming data from existing clients here ...
                        	char *cmd = buffer;
	                       	printf("\nClient sent me: %s\n", cmd);
							process_cmd_from_client(cmd, sock_index);	                        	
							fflush(stdout);


                        }

                        free(buffer);
                    }
                }
            }
        }
    }

    return 0;
}

int add_to_clients(int fdaccept, struct sockaddr_in client_addr, int i, int client_port){
	for(int j = 0; j < client_num; j++){
		if(strcmp(clients[j].ip_addr,inet_ntoa(client_addr.sin_addr))==0){
			return i;

			break;
		}
	}

	char host[1024];
	char service[20];
	clients[i].socket_id = fdaccept;
	clients[i].no = i + 1;
	getnameinfo(&client_addr, sizeof client_addr, host, sizeof host, service, sizeof service, 0);
	strcpy(clients[i].hostname, host);
	strcpy(clients[i].ip_addr, inet_ntoa(client_addr.sin_addr));
	clients[i].login_status = 1;
	clients[i].send_msg_num = 0;
	clients[i].recv_msg_num = 0;
	clients[i].block_num = 0;

	return i+1;
}

int send_login_list(int client_socket){

	char login_msg[1024];
	char temp[1024];
	memset(login_msg,'\0',1024);
	memset(temp,'\0',1024);

	strcat(login_msg,"CLIST ");
	for(int i = 0;i < client_num;i++){
		if(clients[i].login_status>0){

			sprintf(temp, "%d", clients[i].no);
			strcat(login_msg, temp);
			memset(temp,'\0',1024);
			strcat(login_msg, ",");
			strcat(login_msg, clients[i].hostname);
			strcat(login_msg, ",");
			strcat(login_msg, clients[i].ip_addr);
			strcat(login_msg, ",");
			sprintf(temp, "%d", clients[i].port_no);
			strcat(login_msg, temp);
			strcat(login_msg, " ");

		}
	}

	int len = send(client_socket, login_msg, sizeof(login_msg), 0);
	if (len > 0)
		printf("Done in list send\n");
	fflush(stdout);

	return 0;
}

int send_login_list_refresh(int client_socket){

	char login_msg[1024];
	char temp[1024];
	memset(login_msg,'\0',1024);
	memset(temp,'\0',1024);

	strcat(login_msg,"refresh ");
	for(int i = 0;i < client_num;i++){
		if(clients[i].login_status>0){

			sprintf(temp, "%d", clients[i].no);
			strcat(login_msg, temp);
			memset(temp,'\0',1024);
			strcat(login_msg, ",");
			strcat(login_msg, clients[i].hostname);
			strcat(login_msg, ",");
			strcat(login_msg, clients[i].ip_addr);
			strcat(login_msg, ",");
			sprintf(temp, "%d", clients[i].port_no);
			strcat(login_msg, temp);
			strcat(login_msg, " ");

		}
	}

	int len = send(client_socket, login_msg, sizeof(login_msg), 0);
	if (len > 0)
		printf("Done in refresh list send\n");
	fflush(stdout);

	return 0;
}

int process_cmd_from_client(char *msg, int sock_index){

	char *sender_ip;
	int sender_no;
    for(int i =0; i<client_num; i++)
    {
        if(sock_index ==clients[i].socket_id)
        { 
           sender_ip = clients[i].ip_addr;
           sender_no = i;
           break;
        }
    }	
    
    char *cmd, c_ip[INET_ADDRSTRLEN], *token, c_msg[MSG_SIZE], send_buf[BUFFER_SIZE];

    bzero(send_buf,BUFFER_SIZE);
    bzero(c_msg,MSG_SIZE);
    bzero(c_ip,INET_ADDRSTRLEN);

    token = strtok(msg, " ");

    if(token != NULL)
    {      
        cmd = token;
        token = strtok(NULL, " ");
    }
    else{
    	cmd = msg;
    }

    printf("We get this CMD:%s\n", cmd);

    if (strcmp("SEND",cmd)==0){
    	int target_socket = 0;
    	int count = 0;
		char *words[10];

		while( token != NULL){

			words[count] = token;
			count++;
    		token = strtok (NULL, " ");

		}
		if(count<1){
			cse4589_print_and_log("[RELAYED:ERROR]\n");
        	cse4589_print_and_log("[RELAYED:END]\n");
        	return 0;
		}

		if(count>=1){
			for(int i = 1; i < count; i++){
				strcat(c_msg,words[i]);
				strcat(c_msg," ");
			}
		}
		if(c_msg[strlen(c_msg)-1]==' '){
			c_msg[strlen(c_msg)-1]='\0';
		}
		strcpy(c_ip,words[0]);
		strcat(send_buf,"msg_send ");
		strcat(send_buf,sender_ip);		
        strcat(send_buf," ");
        strcat(send_buf,c_msg);

        fflush(stdout);
        int n = 0;
        int if_login = check_if_login(c_ip);
        int if_block = check_if_block(c_ip,sender_ip);
		if(if_login == 1 && if_block == 0){
			target_socket = search_client_by_ip(c_ip);
			if(target_socket > 0){
				if(send(target_socket, send_buf, strlen(send_buf),0)>0){
					n++;
					for (int i =0; i< client_num;i++){
						if(strcmp(clients[i].ip_addr,c_ip)==0){
							clients[i].recv_msg_num++;
							clients[sender_no].send_msg_num++;
							break;
						}
					}
					fflush(stdout);
				}
			}
		}else if(if_login == 0 && if_block == 0){
			if( target_socket = search_client_by_ip(c_ip)>0){
				for (int i =0; i< client_num;i++){
					if(strcmp(clients[i].ip_addr,c_ip)==0&&clients[i].buffered_msg<100){
						strcpy(clients[i].msg_buffer[clients[i].buffered_msg],send_buf);
						n++;
						clients[i].buffered_msg++;
						clients[sender_no].send_msg_num++;
						break;
					}
				}
			}
		}
		else{
        	cse4589_print_and_log("[RELAYED:ERROR]\n");
        	cse4589_print_and_log("[RELAYED:END]\n");
		}

		if(n > 0){
			cse4589_print_and_log("[RELAYED:SUCCESS]\n");
			cse4589_print_and_log("msg from:%s, to:%s\n[msg]:%s\n", sender_ip, c_ip, c_msg);
			cse4589_print_and_log("[RELAYED:END]\n");
		}else{
			cse4589_print_and_log("[RELAYED:ERROR]\n");
			cse4589_print_and_log("[RELAYED:END]\n");
		}

		bzero(send_buf,BUFFER_SIZE);


    }
    else if (strcmp("BROADCAST",cmd) == 0){
    	int target_socket = 0;
    	char broadcast_msg[MSG_SIZE];
    	bzero(broadcast_msg,MSG_SIZE);
    	if( token != NULL){

			strcpy(broadcast_msg,token);
    		token = strtok (NULL, " ");

		}
		
		strcat(send_buf,"msg_broad ");
		strcat(send_buf,sender_ip);		
        strcat(send_buf," ");
        strcat(send_buf,broadcast_msg);
        int count = 0;
		for(int i = 0; i < client_num; i++){
			if(i != sender_no){
				target_socket = clients[i].socket_id;
				if(clients[i].login_status == 1 && check_if_block(clients[i].ip_addr,sender_ip) == 0){
					if(send(target_socket, send_buf, strlen(send_buf),0)>0){
						count++;
						clients[i].recv_msg_num++;
						clients[sender_no].send_msg_num++;
						fflush(stdout);
					}
				}
				else if(clients[i].login_status == 0 && check_if_block(clients[i].ip_addr,sender_ip) == 0){
					for (int m =0; m< client_num;m++){
						if(strcmp(clients[m].ip_addr,clients[i].ip_addr)==0&&clients[m].buffered_msg<100){
							strcpy(clients[m].msg_buffer[clients[m].buffered_msg],send_buf);
							count++;
							clients[m].buffered_msg++;
							clients[sender_no].send_msg_num++;
						}
					}
				}
			}
		}
		if(count>0){
			cse4589_print_and_log("[RELAYED:SUCCESS]\n");
			cse4589_print_and_log("msg from:%s, to:%s\n[msg]:%s\n", sender_ip, "255.255.255.255", broadcast_msg);
			cse4589_print_and_log("[RELAYED:END]\n");
		}else{
			cse4589_print_and_log("[RELAYED:ERROR]\n");
			cse4589_print_and_log("[RELAYED:END]\n");
		}
	}
	else if (strcmp("LOGOUT",cmd) == 0){

    	for(int i = 0;i<client_num;i++)
        {
            if(sock_index == clients[i].socket_id)
            {
            	
                clients[i].login_status = 0;
                close(sock_index);
            	FD_CLR(sock_index,&master_list);
                break;
            }
        }

    }
    else if (strcmp("REFRESH",cmd) == 0){
    	for(int i = 0;i<client_num;i++)
        {
            if(sock_index == clients[i].socket_id)
            {
                sort_list();
                send_login_list_refresh(sock_index);
				break;
            }
        }
    }
    else if(strcmp("PORT_SEND",cmd) == 0){
		char temp[MSG_SIZE];	

		strcpy(temp,token);		
		int client_port = atoi(temp);
		
		for(int i = 0; i< client_num; i++){
		
			if(clients[i].socket_id == sock_index){
				clients[i].port_no = client_port;
				clients[i].login_status = 1;
				break;
			}
		
		}
		//add_to_clients(sock_index, client_addr, client_num, client_port);                   
        /* Send Login List to this Client*/
        sort_list();
		send_login_list(sock_index);
		if(clients[sender_no].buffered_msg > 0){
			int buf_num = clients[sender_no].buffered_msg;
			for (int k = 0; k < buf_num; k++){
				if(send(sock_index,clients[sender_no].msg_buffer[k],strlen(clients[sender_no].msg_buffer[k]),0)>0){
					bzero(clients[sender_no].msg_buffer[k],BUFFER_SIZE);
					clients[sender_no].buffered_msg--;
					clients[sender_no].recv_msg_num++;
					usleep(5000);
				}
			}
			usleep(5000);
			send(sock_index,"LOGINSUCCESS",12,0);
		}else{
			usleep(5000);
			send(sock_index,"LOGINSUCCESS",12,0);
		}

	}
	else if(strcmp("BLOCK_IP",cmd) == 0){
		char c_ip[INET_ADDRSTRLEN];
		bzero(c_ip,INET_ADDRSTRLEN);
		if( token != NULL){

			strcat(c_ip,token);
    		token = strtok (NULL, " ");

		}
		for(int i = 0; i < client_num; i++){
			if (clients[i].socket_id == sock_index){
				strcat(clients[i].blockedList[clients[i].block_num],c_ip);
				clients[i].block_num++;
				break;
			}
		}

	}else if(strcmp("UNBLOCK_IP",cmd) == 0){
		char c_ip[INET_ADDRSTRLEN];
		bzero(c_ip,INET_ADDRSTRLEN),INET_ADDRSTRLEN;
		if( token != NULL){

			strcat(c_ip,token);
    		token = strtok (NULL, " ");

		}
		for(int i = 0; i < client_num; i++){
			if (clients[i].socket_id == sock_index){
				for(int j = 0; j<clients[i].block_num;j++){
					if (strcmp(clients[i].blockedList[j],c_ip)==0 ){
						bzero(clients[i].blockedList[j],INET_ADDRSTRLEN);
						
						break;
					}
				}
			}
			break;
		}
	}
}