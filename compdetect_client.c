#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include "cJSON.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h> 
#include <stdbool.h>
#define DEFAULT_SIZE 1024

FILE *config_file;

void cleanExit(){exit(0);}

void client_deliver_json(int socketfd, char *jsonBuffer) {
	send(socketfd, jsonBuffer, strlen(jsonBuffer), 0);
}



char* get_config_file(char *config_filename) {
	FILE *fp = fopen(config_filename, "r");
    	if (fp == NULL) {
        	perror("Error opening file");
        	return NULL;
    	}
    	
    	// Reading the contents of the file into a buffer using fseek
    	
    	//Get file size to make buffer
   	fseek(fp, 0, SEEK_END);
   	long file_size = ftell(fp);
    	fseek(fp, 0, SEEK_SET);
    	
    	//Allocate memory for buffer
    	char *json_buffer = (char *)malloc(file_size + 1);
    	
    	if (json_buffer == NULL) {
        	perror("Buffer malloc failed.");
        	fclose(fp);
        	return NULL;
    	}
    	
    	//Reading file using byte size check
    	size_t bytes_read = fread(json_buffer, 1, file_size, fp);
    	
    	//Byte size check
   	if (bytes_read != file_size) {
        	perror("Error reading file");
        	fclose(fp);
        	free(json_buffer);
        	return NULL;
   	}
   	
   	//Closing file now that we have buffer
   	fclose(fp);
    
	//Add a null terminator char to buffer
    	json_buffer[file_size] = '\0'; 
    	
    	return json_buffer;
}

int make_tcp_socket(cJSON *config_json) {
	const char *ipaddrname = "server_ip_address";
    	cJSON *server_ipaddr = cJSON_GetObjectItem(config_json, ipaddrname);
    	
	int fdsocket = socket(PF_INET, SOCK_STREAM, PF_UNSPEC);
	if (fdsocket == -1) {
		perror("Socket creation failed");
		return 0;
	}
	
	int optval = 1;
	
	struct sockaddr_in serverAddr;
	
	const char *tcp_pre_probing_port_name = "tcp_pre_probing_port";
    	cJSON *tcp_pre_probing_val = cJSON_GetObjectItem(config_json, tcp_pre_probing_port_name);
    	int tcp_pre_probing_port = tcp_pre_probing_val->valueint;
	
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(tcp_pre_probing_port);
	serverAddr.sin_addr.s_addr = inet_addr(server_ipaddr->valuestring);
	
	int conCheck = connect(fdsocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));
	
	if (conCheck < 0) {
		perror("Connect failed");
		return 1;
	}
	
	return fdsocket;
}

int make_udp_sock(cJSON *config_json) {
	const char *ipaddrname = "server_ip_address";
    	cJSON *server_ipaddr = cJSON_GetObjectItem(config_json, ipaddrname);
    	
	int udpsocket = socket(PF_INET, SOCK_DGRAM, PF_UNSPEC);
	if (udpsocket == -1) {
		perror("Socket creation failed");
		return 0;
	}
	
	
	int enable = 1;
    if (setsockopt(udpsocket, IPPROTO_IP, IP_MTU_DISCOVER, &enable, sizeof(enable)) < 0) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

	if (setsockopt(udpsocket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0) {
        perror("setsockopt SO_REUSEADDR failed");
        exit(EXIT_FAILURE);
    }
	
	const char *udp_source_port_name = "udp_source_port";
    	cJSON *udp_source_val = cJSON_GetObjectItem(config_json, udp_source_port_name);
    	int udp_source_port = udp_source_val->valueint;
	
	
	
	struct sockaddr_in clientAddr;
	
	bzero(&clientAddr, sizeof(clientAddr));
	
	clientAddr.sin_family = AF_INET;
	clientAddr.sin_port = htons(udp_source_port);
	clientAddr.sin_addr.s_addr = INADDR_ANY;
	
	int bindCheck = bind(udpsocket, (struct sockaddr *) &clientAddr, sizeof(clientAddr));
	
	if (bindCheck < 0) {
		perror("Bind check failed");
		return 1;
	}
	
	struct sockaddr_in serverAddr;
	bzero(&serverAddr, sizeof(serverAddr));
	
	const char *udp_destination_port_name = "udp_destination_port";
    	cJSON *udp_destination_val = cJSON_GetObjectItem(config_json, udp_destination_port_name);
    	int udp_destination_port = udp_destination_val->valueint;

	
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(udp_destination_port);
	serverAddr.sin_addr.s_addr = inet_addr(server_ipaddr->valuestring);
	
	//Get the number of packets
	//Get the payload of packets
	//b-zero buffer
	
	const char *train_size_name= "udp_packet_train_size";
    	cJSON *train_size_val = cJSON_GetObjectItem(config_json, train_size_name);
    	int train_size = train_size_val->valueint;
	
	const char *payload_name= "udp_payload_size";
    	cJSON *payload_val = cJSON_GetObjectItem(config_json, payload_name);
    	int payload = payload_val->valueint;

	char low_entropy[payload];
	
	bzero(low_entropy, sizeof(low_entropy));
	
	
	int sendtoCheck = -1;
	for (int i = 0; i < train_size; i++) {
		int pack_id = i + 1;
		low_entropy[0] = (pack_id >> 8) & 0xFF;
        	low_entropy[1] = pack_id & 0xFF;
	
		int sendtoCheck = sendto(udpsocket, low_entropy, sizeof(low_entropy), 0, (struct sockaddr *) &serverAddr, sizeof(serverAddr));
		
		if (sendtoCheck < 0) {
		perror("send failed");
		//return 1;
		}
		
		usleep(200);
	}
	
	char entropy[payload];
	FILE *fp = fopen("/dev/urandom", "r");
    	if (fp == NULL) {
        	perror("Error opening file");
        	return 1;
    	}
    	//Allocate memory for buffer
    	char *buffer = (char *)malloc(payload);
    	
    	if (buffer == NULL) {
        	perror("Buffer malloc failed.");
        	fclose(fp);
        	return 1;
    	}
    	
    	//Reading file using byte size check
    	size_t bytes_read = fread(buffer, 1, payload, fp);
    	
    	//Byte size check
   	if (bytes_read != payload) {
        	perror("Error reading file");
        	fclose(fp);
        	free(buffer);
        	return 1;
   	}
   	
   	memcpy(entropy, buffer, payload);
   	free(buffer);
   	fclose(fp);
   	
   	const char *time_name = "inter_measurement_time";
    	cJSON *time_val = cJSON_GetObjectItem(config_json, time_name);
    	int time = time_val->valueint;
    	
    	sleep(time);
    	
    	for (int i = 0; i < train_size; i++) {
		int pack_id = i + 1;
		entropy[0] = (pack_id >> 8) & 0xFF;
        	entropy[1] = pack_id & 0xFF;
	
		int sendtoCheck = sendto(udpsocket, entropy, sizeof(entropy), 0, (struct sockaddr *) &serverAddr, sizeof(serverAddr));
		usleep(200);
	}
	
	return udpsocket;
}

int post_tcp_socket(int port, cJSON *config_json) {
	
	const char *ipaddrname = "server_ip_address";
    	cJSON *server_ipaddr = cJSON_GetObjectItem(config_json, ipaddrname);
    	
	int fdsocket = socket(PF_INET, SOCK_STREAM, PF_UNSPEC);
	if (fdsocket == -1) {
		perror("Socket creation failed");
		return 0;
	}
	
	int optval = 1;
	
	struct sockaddr_in serverAddr;
	
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.s_addr = inet_addr(server_ipaddr->valuestring);
	
	int conCheck = connect(fdsocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));
	
	if (conCheck < 0) {
		perror("Connect failed");
		return 1;
	}
	
	char buffer[1020];
	
	int check = read(fdsocket, buffer, sizeof(buffer));
	
	buffer[check] = '\0';
	printf("%s\n", buffer);
	//recive here
	
	return fdsocket;
	
}




int main(int argc, char* argv[]) {

	//Getting config file name
        char* config_filename = argv[1];
        
        char* json_buffer = get_config_file(config_filename);
        //opening file with open check
        if (json_buffer == NULL) {
        	return 1;
        }
        
    	//get the json object using parse
    	cJSON *config_json = cJSON_Parse(json_buffer);
    	
    	if (config_json == NULL) {
        	perror("Error parsing JSON: %s\n");
        	free(json_buffer);
        	return 1;
    	}
    	
    	int tcpfdsocket = make_tcp_socket(config_json);
	
	client_deliver_json(tcpfdsocket, json_buffer);
	
	free(json_buffer);
	
	close(tcpfdsocket);
	
	int udpfdsocket_low = make_udp_sock(config_json);
	close(udpfdsocket_low);
	
	const char *post_prob_port_name = "tcp_post_probing_port"; 
	cJSON *post_prob_port_val =  cJSON_GetObjectItem(config_json, post_prob_port_name); 
	
	int tcpfd = post_tcp_socket(post_prob_port_val->valueint, config_json);
	close(tcpfd);
	
	cJSON_Delete(config_json);

}

