//new file post mistake
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "cJSON.h"
#include <arpa/inet.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>

void cleanExit(){exit(0);} 

cJSON* recv_json(int accID) {
	char buffer[1020];
	int check = read(accID, buffer, sizeof(buffer));
	buffer[check] = '\0';
	
	cJSON *config_json = cJSON_Parse(buffer);
	return config_json;
	
}

int make_tcp_socket(int port) {
	int fdsocket = socket(PF_INET, SOCK_STREAM, PF_UNSPEC);
	if (fdsocket == -1) {
		perror("Socket creation failed");
		return 0;
	}
	
	int optval = 1;
	
	int setSockCheck = setsockopt(fdsocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    	
    	if (setSockCheck < 0) {
    		perror("Cannot reuse address");
    		return 1;
    	}
	struct sockaddr_in serverAddr;
	
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	
	int bindcheck = bind(fdsocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));
	
	if (bindcheck < 0) {
		perror("Cannot bind server to socket");
		return 1;
	} 
	
	int listenCheck = listen(fdsocket, 5);
	
	if (listenCheck < 0) {
		perror("Error listening");
		return 1;
	}
	
	struct sockaddr_in clientAddr;
	int client_socket, addr_len = sizeof(clientAddr);
	
	int accID = accept(fdsocket, (struct sockaddr*) &clientAddr, &addr_len);

	if (accID < 0) {
		perror("Error accepting connection");
		return 1;
	}
	
	close(fdsocket);
	signal(SIGTERM, cleanExit);
	signal(SIGINT, cleanExit);
	
	return accID;
	
}

int make_udp_socket(int port, cJSON *config_json) {
	int udp_sfd = socket(PF_INET, SOCK_DGRAM, PF_UNSPEC);
	
	if (udp_sfd == -1) {
		perror("Socket creation failed");
		return 0;
	}
	
	int optval = 1;
	
	int setSockCheck = setsockopt(udp_sfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	
	struct timeval timeout;
    		timeout.tv_sec = 3;
    		timeout.tv_usec = 0;
    		if (setsockopt(udp_sfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout)) == -1) {
        	perror("setsockopt failed");
        	exit(EXIT_FAILURE);
    	}
	
	struct sockaddr_in serverAddr;
	bzero(&serverAddr, sizeof(serverAddr));
	const char *ipaddrname = "server_ip_address"; 
	cJSON *server_ipaddr = cJSON_GetObjectItem(config_json, ipaddrname);
	
	serverAddr.sin_addr.s_addr = inet_addr(server_ipaddr->valuestring); 
   	serverAddr.sin_port = htons(port); 
    	serverAddr.sin_family = AF_INET;
    	
    	int bindCheck = bind(udp_sfd, (struct sockaddr *) &serverAddr, sizeof(serverAddr));
    	
    	if (bindCheck < 0) {
		perror("Cannot bind server to socket");
		return 1;
	}
    	
    	struct sockaddr_in clientAddr;
    	bzero(&clientAddr, sizeof(clientAddr));
    	char buffer[1020];
    	
    	int clientAddrLen = sizeof(clientAddr);
    	
    	int mesLength;
    	
    	int mesCount = 0;
    	
    	int lostPacketsTrain1 = 0;
    	int lostPacketsTrain2 = 0;
    	
    	char *measurement_time_name = "inter_measurement_time"; 
	cJSON *measurement_time_val = cJSON_GetObjectItem(config_json, measurement_time_name);
	int measure_time = measurement_time_val->valueint;

    	
    	char *udp_packet_train_size_name = "udp_packet_train_size"; 
	cJSON *udp_packet_train_size_val = cJSON_GetObjectItem(config_json, udp_packet_train_size_name);
	int udp_packet_train_size = udp_packet_train_size_val->valueint;
    	
    	clock_t start_time_train1, end_time_train1, start_time_train2, end_time_train2;
    	
		timeout.tv_sec = 3;
    			timeout.tv_usec = 0;
    			if (setsockopt(udp_sfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout)) == -1) {
        			perror("setsockopt failed");
        			exit(EXIT_FAILURE);
    			}
				
    	while (1) {
    	
    		if (mesCount == 0) {
    			start_time_train1 = clock();
    		}
    	
    		if (mesCount == udp_packet_train_size - 1) {

    			end_time_train1 = clock();
    			sleep(15);
    		}
    		
    		if (mesCount == udp_packet_train_size) {
    			start_time_train2 = clock();
    		}
    		
    		if (mesCount == udp_packet_train_size + udp_packet_train_size - 1) {
    			end_time_train2 = clock();
    			break;
    		}
    		
    		mesLength = recvfrom(udp_sfd, buffer, sizeof(buffer), 0, (struct sockaddr*) &clientAddr, &clientAddrLen);
    		
    		if (mesLength <= 0 && mesCount < udp_packet_train_size) {
    			lostPacketsTrain1++;
    		} else if (mesLength <= 0 && mesCount < udp_packet_train_size) {
    			lostPacketsTrain2++;
    		}
    		mesCount++;
    		
    		
    	}
    	
    	double dif_train_1, dif_train_2;
    	
    	dif_train_1 = ((double)(end_time_train1 - start_time_train1) / CLOCKS_PER_SEC) * 1000.0;
    	
    	dif_train_2 = ((double)(end_time_train2 - start_time_train2) / CLOCKS_PER_SEC) * 1000.0;
    	
    
    	double difference = dif_train_2 - dif_train_1;
    
    	bool hasComp = false;
    	if (difference > 100) {

    		hasComp = true;
    	}
	
    	close(udp_sfd);
    	
    	return hasComp;
    	
    	
}


int post_tcp_socket(int port, bool wasComp) {

	int fdsocket = socket(PF_INET, SOCK_STREAM, PF_UNSPEC);
	if (fdsocket == -1) {
		perror("Socket creation failed");
		return 0;
	}
	
	int optval = 1;
	
	int setSockCheck = setsockopt(fdsocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    	
    	if (setSockCheck < 0) {
    		perror("Cannot reuse address");
    		return 1;
    	}
	struct sockaddr_in serverAddr;
	
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	
	int bindcheck = bind(fdsocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));
	
	if (bindcheck < 0) {
		perror("Cannot bind server to socket");
		return 1;
	} 
	
	int listenCheck = listen(fdsocket, 5);
	
	if (listenCheck < 0) {
		perror("Error listening");
		return 1;
	}
	
	struct sockaddr_in clientAddr;
	int client_socket, addr_len = sizeof(clientAddr);
	
	int accID = accept(fdsocket, (struct sockaddr*) &clientAddr, &addr_len);

	

	if (accID < 0) {
		perror("Error accepting connection");
		return 1;
	}
	
	
	char compMessage[1020];
	
	if (wasComp == true) {
		char* detected = "Compression detected!";
		strcpy(compMessage, detected);
	} else {
		char* detected = "No compression was detected.";
		strcpy(compMessage, detected);
	}
	
	int sendCheck = send(accID, compMessage, strlen(compMessage), 0);
	if (sendCheck < 0) {
		perror("send fail");
		return 1;
	}
	
	close(fdsocket);
	
	
	return accID;
}


int main(int argc, char* argv[]) {
	int port = atoi(argv[1]);
	
	int accID = make_tcp_socket(port);
	accID;
	cJSON *config_json = recv_json(accID);
	if (config_json == NULL) {
		perror("Error parsing JSON");
		return 1;
	}
	
	const char *udp_destination_port_name = "udp_destination_port";
	cJSON *udp_destination_port_val =  cJSON_GetObjectItem(config_json, udp_destination_port_name);
	
	int udp_destination_port = udp_destination_port_val->valueint;
	
	bool wasComp = make_udp_socket(udp_destination_port, config_json);
	
	const char *post_prob_port_name = "tcp_post_probing_port";
	cJSON *post_prob_port_val =  cJSON_GetObjectItem(config_json, post_prob_port_name);
	
	int tcpfd = post_tcp_socket(post_prob_port_val->valueint, wasComp);

	cJSON_Delete(config_json);
	
}