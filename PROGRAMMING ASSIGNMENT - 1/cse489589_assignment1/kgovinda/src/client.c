#include "../include/global.h"
#include "../include/logger.h"

int login(char *cmd, int client_socket);
int connect_to_host(char *server_ip, int server_port, int client_socket);
int establish_client(int port_num);
int process_res_from_server(char *cmd);

int connect_to_host(char *server_ip, int server_port,int fdsocket){
    struct sockaddr_in remote_server_addr;
    bzero(&remote_server_addr, sizeof(remote_server_addr));
    remote_server_addr.sin_family = AF_INET;
    inet_pton(AF_INET, server_ip, &remote_server_addr.sin_addr);
    remote_server_addr.sin_port = htons(server_port);
	connect(fdsocket, (struct sockaddr*)&remote_server_addr, sizeof(remote_server_addr));
    return fdsocket;
}

int establish_client(int port_num){

	int c_port;
	printf("\nClient Running\n");
	c_port = port_num;
	struct client this_client;
	char host[128];
	char msg_port[MSG_SIZE];
	int head_socket, client_socket, selret, sock_index, server_fd, server_socket = 0;
	int yes = 1;
	socklen_t caddr_len;
	struct sockaddr_in client_addr, server_addr;
	struct hostent *hostinfo;

    /* Zero select FD sets */
    FD_ZERO(&master_list);
    FD_ZERO(&watch_list);
    /* Register STDIN */
    FD_SET(STDIN, &master_list);
	
	head_socket = STDIN;

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
            for(sock_index = 0; sock_index <= head_socket; sock_index += 1){

                if(FD_ISSET(sock_index, &watch_list)){

                    /* Check if new command on STDIN */
                    if (sock_index == STDIN){
                    	char *cmd = (char*) malloc(sizeof(char)*CMD_SIZE);
                    	memset(cmd, '\0', CMD_SIZE);
						if(fgets(cmd, CMD_SIZE-1, stdin) == NULL) //Mind the newline character that will be written to cmd
							exit(-1);

						printf("\nClient got: %s\n", cmd);
						cmd[strlen(cmd)-1]='\0';

						if(strncmp(cmd, "LOGIN", 5) == 0){
							client_socket = socket(AF_INET, SOCK_STREAM, 0);

    						if(client_socket < 0){

        					perror("Failed to create socket");

    						}
							char msg_temp[MSG_SIZE];
							
							memset(msg_port, '\0', MSG_SIZE);
							memset(msg_temp, '\0', MSG_SIZE);
							strcat(msg_port,"PORT_SEND ");
							sprintf(msg_temp,"%d", port_num);
							strcat(msg_port,msg_temp);
							login(cmd,client_socket);
							FD_SET(client_socket, &master_list);
							if(client_socket > head_socket) head_socket = client_socket;
							if(send(client_socket, msg_port, strlen(msg_port),0) == strlen(msg_port)) 
								printf("Done in port send!\n");
							fflush(stdout);												
						}
						else if(strncmp(cmd, "SEND", 4) == 0)
						{
							int count = 0;
							char *p;
							char *words[1024];

							p = strtok(cmd," ");
	
							while( p != NULL){

								words[count] = p;
								count++;
    							p = strtok (NULL, " ");

							}
							if(count<2){
								cse4589_print_and_log("[%s:ERROR]\n", cmd);
								cse4589_print_and_log("[%s:END]\n", cmd);	
								break;							
							}

							char target_client[128];
							bzero(target_client,128);
							char send_msg[MSG_SIZE];
							bzero(send_msg,MSG_SIZE);

							strcat(target_client,words[1]);
							if(count>=2){
								for(int i = 2;i<count;i++){
									strcat(send_msg,words[i]);
									strcat(send_msg," ");
								}
							}
							if(send_msg[strlen(send_msg)-1]==' '){
								send_msg[strlen(send_msg)-1]='\0';
							}
							

							char msg[BUFFER_SIZE];

							bzero(msg,BUFFER_SIZE);
							strcat(msg,"SEND ");
							strcat(msg,target_client);
							strcat(msg," ");
							strcat(msg,send_msg);
							
							int exists = 0;
							for(int i = 0;i<client_num;i++){
								if(strcmp(clients[i].ip_addr,target_client)==0){
									exists = 1;
									break;
								}
							}
							if(exists>0){
								if (send (client_socket, msg, strlen(msg), 0) > 0){
									printf("Done in SEND\n");
									cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
									cse4589_print_and_log("[%s:END]\n", cmd);
								}
							}else{
								cse4589_print_and_log("[%s:ERROR]\n", cmd);
								cse4589_print_and_log("[%s:END]\n", cmd);

							}


							

						}
						else if(strcmp(cmd, "AUTHOR") == 0){

							author_print(cmd);

						}
						else if(strcmp(cmd, "IP") == 0){

							
							ip_print(ip_addr, cmd);

						}
						else if(strcmp(cmd, "PORT") == 0){

							port_print(client_socket, cmd, port_num);


						}
						else if(strcmp(cmd, "REFRESH") == 0){

							char *str = "REFRESH";
							if(send(client_socket,str, strlen(str), 0) == strlen(str)){

								cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
								cse4589_print_and_log("[%s:END]\n", cmd);

							}

						}
						else if(strcmp(cmd, "LOGOUT") == 0){

							char *str = "LOGOUT";
							if(send(client_socket,str, strlen(str), 0) == strlen(str)){

								close(client_socket);
								FD_CLR(client_socket,&master_list);
								cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
								cse4589_print_and_log("[%s:END]\n", cmd);

							}

						}
						else if(strcmp("EXIT",cmd) == 0){

							close(client_socket);
							exit(0);

						}
						else if(strcmp(cmd, "LIST") == 0){
							cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
							
							for (int i = 0; i < client_num; i++){
							
									cse4589_print_and_log("%-5d%-35s%-20s%-8d\n",clients[i].no,clients[i].hostname,clients[i].ip_addr,clients[i].port_no);
																
							}
							cse4589_print_and_log("[%s:END]\n", cmd);  
						}
						else if(strncmp(cmd, "BROADCAST", 9) == 0){
							int count = 0;
							char *p;
							char *word[1024];
							char send_msg[MSG_SIZE];
							bzero(send_msg,MSG_SIZE);

							p = strtok(cmd," ");
			
							while( p != NULL){
								
								word[count] = p;
								count++;
    							p = strtok (NULL, " ");

							}
							if(count>=1){
								for(int i = 1; i < count; i++){
									strcat(send_msg,word[i]);
									strcat(send_msg," ");
								}
							}
							char msg[MSG_SIZE];
							bzero(msg,MSG_SIZE);
							strcat(msg,"BROADCAST");
							strcat(msg," ");
							strcat(msg,send_msg);

							if (send (client_socket, msg, strlen(msg), 0) > 0){
								printf("Done in BROADCAST\n");
								cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
								cse4589_print_and_log("[%s:END]\n", cmd);
							}
						}
						else if(strncmp(cmd, "BLOCK",5) == 0){
							char *p;
							char c_ip[INET_ADDRSTRLEN];
							int count = 0;
							bzero(c_ip,INET_ADDRSTRLEN);
							p = strtok(cmd," ");
							while(p != NULL){
								
								if(count==1)
									strcat(c_ip,p);
								count++;
    							p = strtok (NULL, " ");

							}
							char msg[MSG_SIZE];
							bzero(msg,MSG_SIZE);
							strcat(msg,"BLOCK_IP");
							strcat(msg," ");
							strcat(msg,c_ip);
							if (send (client_socket, msg, strlen(msg), 0) > 0){
								printf("Done in BLOCK\n");
								cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
								cse4589_print_and_log("[%s:END]\n", cmd);
							}

						}
						else if(strncmp(cmd, "UNBLOCK",5) == 0){
							char *p;
							char c_ip[INET_ADDRSTRLEN];
							int count = 0;
							bzero(c_ip,INET_ADDRSTRLEN);
							p = strtok(cmd," ");
							while(p != NULL){
								
								if(count==1)
									strcat(c_ip,p);
								count++;
    							p = strtok (NULL, " ");

							}
							char msg[MSG_SIZE];
							bzero(msg,MSG_SIZE);
							strcat(msg,"UNBLOCK_IP");
							strcat(msg," ");
							strcat(msg,c_ip);
							if (send (client_socket, msg, strlen(msg), 0) > 0){
								printf("Done in UNBLOCK\n");
								cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
								cse4589_print_and_log("[%s:END]\n", cmd);
							}

						} 
        				
					}
					else{
                        char *buffer = (char*) malloc(sizeof(char)*BUFFER_SIZE);
                        memset(buffer, '\0', BUFFER_SIZE);

                        int recv_bytes = recv(sock_index, buffer, BUFFER_SIZE, 0);
						char *cmd = buffer;
						if (recv_bytes > 0){
                        	process_res_from_server(cmd);
						}

						fflush(stdout);
                        free(buffer);
                    }
                }
        	}
		}
	}
	
	return 0;
}

int login(char *cmd,int client_socket){

	
	int server = 0;
	int count = 0;
	char *p;
	char *words[10];
	

	p = strtok(cmd," ");
	
	while( p != NULL){

		words[count] = p;
		count++;
    	p = strtok (NULL, " ");

	}
	if(count<2){

        cse4589_print_and_log("[%s:ERROR]\n", "LOGIN");
        cse4589_print_and_log("[%s:END]\n", "LOGIN");
        return 0;
    }
    if(isValidAddr(words[1],words[2])==0){
    	cse4589_print_and_log("[%s:ERROR]\n", "LOGIN");
        cse4589_print_and_log("[%s:END]\n", "LOGIN");
        return 0;
    }
	char *login_address = words[1];
	int login_port = atoi(words[2]);
	
	server = connect_to_host(login_address, login_port, client_socket);

	return server;
}

int process_res_from_server(char *msg){
	char *res,*token;
	token = strtok(msg, " ");
	char from_ip[INET_ADDRSTRLEN], c_msg[MSG_SIZE], send_buf[BUFFER_SIZE];
	bzero(from_ip,INET_ADDRSTRLEN);
	bzero(c_msg,MSG_SIZE);
	bzero(send_buf,BUFFER_SIZE);

    if(token != NULL)
    {      
        res = token;
        token = strtok(NULL, " ");
    }


	if (strcmp(res,"CLIST") == 0){
		client_num = 0;
    	char *temp[10];
		char *line, *p2;
		
    	while( token != NULL){

			temp[client_num] = token;
			client_num++;
    		token = strtok (NULL, " ");

		}
		for (int j = 0 ; j < client_num; j++){
			line = strtok(temp[j], ",");
			int i = 0;
			while(line != NULL){

				switch (i){

					 case 0: clients[j].no = atoi(line);
					 	clients[j].login_status = 1;
					 	i++;
					 	break;
								
					case 1: strcpy(clients[j].hostname,line);
						i++;
						break;
						
					case 2: strcpy(clients[j].ip_addr,line);
						i++;
						break;
								
					case 3: clients[j].port_no = atoi(line);
						i++;
						break;

                }	
			 	line=strtok(NULL,",");		
			 	fflush(stdout);				
			}
		}
		
	}else if(strcmp(res,"msg_send") == 0){
		char *temp[10];
		int i = 0;
    	while( token != NULL){

			temp[i] = token;
			i++;
    		token = strtok (NULL, " ");

		}
		if(i<1){
			cse4589_print_and_log("[RECEIVED:ERROR]\n");
        	cse4589_print_and_log("[RECEIVED:END]\n");
        	return 0;
		}
		if(i>=1){
			for(int j = 1; j < i; j++){
				strcat(c_msg,temp[j]);
				strcat(c_msg," ");
			}
		}
		if(c_msg[strlen(c_msg)-1]==' '){
			c_msg[strlen(c_msg)-1]='\0';
		}

		strcpy(from_ip,temp[0]);

		cse4589_print_and_log("[RECEIVED:SUCCESS]\n");
		cse4589_print_and_log("msg from:%s\n[msg]:%s\n", from_ip, c_msg);
		cse4589_print_and_log("[RECEIVED:END]\n");

	}
	else if(strcmp(res,"refresh") == 0){
    	char *temp[10];
		char *line, *p2;
		client_num = 0;
    	while( token != NULL){

			temp[client_num] = token;
			printf ("%s\n", temp[client_num]);
			client_num++;
    		token = strtok (NULL, " ");

		}
		for (int j = 0 ; j < client_num; j++){
			line = strtok(temp[j], ",");
			int i = 0;
			while(line != NULL){

				switch (i){

					 case 0: clients[j].no = atoi(line);
					 	clients[j].login_status = 1;
					 	i++;
					 	break;
								
					case 1: strcpy(clients[j].hostname,line);
						i++;
						break;
						
					case 2: strcpy(clients[j].ip_addr,line);
						i++;
						break;
								
					case 3: clients[j].port_no = atoi(line);
						i++;
						break;

                }	
			 	line=strtok(NULL,",");		
			 	fflush(stdout);				
			}
		}
	}
	else if(strcmp(res,"msg_broad") == 0){
		char *temp[10];
		int i = 0;
    	while( token != NULL){

			temp[i] = token;
			i++;
    		token = strtok (NULL, " ");

		}
		if(i<1){
			cse4589_print_and_log("[RECEIVED:ERROR]\n");
        	cse4589_print_and_log("[RECEIVED:END]\n");
        	return 0;
		}
		if(i>=1){
			for(int j = 1; j < i; j++){
				strcat(c_msg,temp[j]);
				strcat(c_msg," ");
			}
		}



		strcpy(from_ip,temp[0]);
		strcpy(c_msg,temp[1]);
		cse4589_print_and_log("[RECEIVED:SUCCESS]\n");
		cse4589_print_and_log("msg from:%s\n[msg]:%s\n", from_ip, c_msg);
		cse4589_print_and_log("[RECEIVED:END]\n");
	}
	else if(strcmp(res,"msg_buffered") == 0){
		char *temp[10];
		int i = 0;
    	while( token != NULL){

			temp[client_num] = token;
			printf ("%s\n", temp[client_num]);
			i++;
    		token = strtok (NULL, " ");

		}

		strcat(from_ip,temp[0]);
		strcat(c_msg,temp[1]);
		cse4589_print_and_log("[RECEIVED:SUCCESS]\n");
		cse4589_print_and_log("msg from:%s\n[msg]:%s\n", from_ip, c_msg);
		cse4589_print_and_log("[RECEIVED:END]\n");
 		cse4589_print_and_log("[%s:SUCCESS]\n", "LOGIN");
        cse4589_print_and_log("[%s:END]\n", "LOGIN");

	}else if(strcmp(res,"LOGINSUCCESS") == 0){

		cse4589_print_and_log("[%s:SUCCESS]\n", "LOGIN");
        cse4589_print_and_log("[%s:END]\n", "LOGIN");
	
	}

}