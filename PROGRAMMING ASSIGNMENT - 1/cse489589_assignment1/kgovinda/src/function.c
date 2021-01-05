#include "../include/global.h"
#include "../include/logger.h"

int author(char *cmd);
int ip(char* ip_addr, char *cmd);
int port(int socket,char *cmd, int the_port);

int author(char *cmd){

	char your_ubit_name[10] = "kgovinda";

	cse4589_print_and_log("[%s:SUCCESS]\n", cmd); 
	cse4589_print_and_log("I, %s, have read and understood the course academic integrity policy.\n", your_ubit_name);
	cse4589_print_and_log("[%s:END]\n", cmd);
	
	return 0;
}

int ip(char* ip_addr, char *cmd){

	cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
	cse4589_print_and_log("IP:%s\n", ip_addr);
	cse4589_print_and_log("[%s:END]\n", cmd);

	return 0;
}

int port(int socket,char *cmd, int the_port){

	cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
	cse4589_print_and_log("PORT:%d\n", the_port);
	cse4589_print_and_log("[%s:END]\n", cmd);
    return 0;
}