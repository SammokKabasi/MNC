//============================================================================
// Name        : main.cpp
// Author      : Sammok Kabasi
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================


/**
 * Check for TODO before submitting
 *
 * TODO remove unwanted couts and printfs
 *
 */
#include <iostream>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <unistd.h>

using namespace std;

#define STDIN 0
#define MAXIMUM_TIMEOUTS 3

int readTopologyFile();

int numberOfNodes;

int numberOfNeighbours;

int hostServerId = -1;

char weights[10][3];

char* ip_address;

void print_nodes_table() ;

/*
 http://stackoverflow.com/questions/4139405/how-to-know-ip-address-for-interfaces-in-c

 Retrieve the IP address of the process.
 */
int getIP() {
	struct ifaddrs *ifap, *ifa;
	struct sockaddr_in *sa;
	char * ip_address_temp;

	getifaddrs(&ifap);
	for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr->sa_family == AF_INET) {
			sa = (struct sockaddr_in *) ifa->ifa_addr;
			ip_address_temp = inet_ntoa(sa->sin_addr);
			//Make sure it's not the default localhost address that you are using.
			if ((strcmp(ip_address_temp, "127.0.0.1") != 0)
					&& (strcmp(ip_address_temp, "0.0.0.0") != 0)) {
				//  				cout<<"Setting the IP address as " <<ip_address_temp<<"   ";
				ip_address = ip_address_temp;
//				cout << ip_address;
				break;
			}
		}
	}

	cout<<endl<<"The IP address is "<<ip_address<<endl;

	freeifaddrs(ifap);
	return 0;
}

char* port_number; //port number of current process

/**
 * Startup function.
 *TODO: do something here
 */
int startServer() {
return 0;
}

//HELPER METHODS
/**
 * TODO: kuch badal isme
 * Checks if the array pointed to by input holds a valid number.
 * Source: PA2 template
 * @param  input char* to the array holding the value.
 * @return TRUE or FALSE
 */
int isNumber(char *input)
{
    while (*input){
        if (!isdigit(*input))
            return 0;
        else
            input += 1;
    }

    return 1;
}

int timeout = -1;

int main(int nNumberofArgs, char* args[]) {


	int c;

	char* file_path;

	//Get the ip address and port number and store it in the local variables
		getIP();

	//read the local topology file
	int c = readTopologyFile();
	printf("%d", c);

	printf("Printing nodes table..\n");


	/*
 	 * Parse the arguments
	 * http://www.gnu.org/software/libc/manual/html_node/Example-of-Getopt.html
	*/
	while((c = getopt(nNumberofArgs, args,"t:i:")) != -1){
		switch (c){
		case 't':
			cout<<"Filename:"<<optarg<<endl;
                       strcpy(file_path, optarg);
                       break;

	        case 'i':   if(!isNumber(optarg)){
                           fprintf(stderr, "Invalid value for -i\n");
			   cout<<"Invalid value for -i\n";
                           return -1;
                       }
                       timeout = atoi(optarg);
                       break;

           case '?':   break;

           default:
			cout<<"Usage: %s -t <path_to_topology_file> -i <routing_update_interval>\n", args[0];

                       return -1;
       }
   }





	print_nodes_table();
	return 0;
}

struct nodes_table_info {
	uint32_t serverIp;
	uint16_t serverPort;
	uint16_t serverId;
	int numberOfTimeouts;
	int isNeighbour;
	uint16_t linkCost;
	int isDisabled;
	int nextHopId;
	char server_name[20];
	nodes_table_info() {
		serverId = 0;
		serverPort = 0;
		serverIp = 0;
		numberOfTimeouts = 0;
		isNeighbour = -1;
		isDisabled = -1;
		linkCost = 0xffff;
		nextHopId = -1;
	}
};

struct nodes_table_info nodes[5];


struct server_details {
	uint32_t serverIp;
	uint16_t port;
	uint16_t serverId;
	uint16_t linkCost;
};

struct routing_packet{
	uint16_t serverPort;
	uint32_t serverIp;
	uint16_t updateFields;
};

/** TODO : change
 * converts string array ip address to a uint32_t object
 */
uint32_t parse_ipaddress_string_to_uint32_t(char* ipAddress) {
	struct in_addr ip_addr;
	inet_pton(AF_INET, ipAddress, &ip_addr);
	return (uint32_t)ntohl(ip_addr.s_addr);

}


/*TODO : change
 *
 * Convert uint32_t to String IP address
*/
char* print_uint32_ip(uint32_t ip)
{
	 char *ipAddress = (char*)malloc(INET_ADDRSTRLEN);
	 struct in_addr ip_addr;
	 ip_addr.s_addr = htonl(ip);
	 inet_ntop(AF_INET, &ip_addr, ipAddress, INET_ADDRSTRLEN);
	 return ipAddress;
}

/*
 * Function to print the nodes table
*/
void print_nodes_table() {
	printf("%-15s|%-15s|%-15s|%-15s|%-15s|%-15s|%-15s|%-15s|\n", "serverId",
			"noOfTimeouts", "isNeighbour", "linkCost", "nextHopId",
			"isDisabled", "serverIp", "server_port");
	for (int i = 0; i < numberOfNodes; i++) {
		printf("%-15d|%-15d|%-15d|%-15d|%-15d|%-15d|%-15s|%-15d|\n",
				nodes[i].serverId, nodes[i].numberOfTimeouts,
				nodes[i].isNeighbour, nodes[i].linkCost, nodes[i].nextHopId,
				nodes[i].isDisabled, print_uint32_ip(nodes[i].serverIp),
				nodes[i].serverPort);
	}
}


int readTopologyFile() {

	FILE *fp;
	char buf[255];
	char inputFileName[] =
			"C:\\Users\\Sammok\\workspace_for_c\\MNCProject2C++\\foliage.txt";
	fp = fopen(inputFileName, "r");

	if (NULL == fp) {
		printf("Error opening file");
		return -1;
	}

	//Line 1 is number of nodes
	fscanf(fp, "%s", buf);

	numberOfNodes = atoi(buf);
	printf("number of nodes -->%d\n" , numberOfNodes);


	//Line 2 is numberOfNeighbours
	fscanf(fp, "%s", buf);
	numberOfNeighbours = atoi(buf);
	printf("number of connections -->%d\n" , numberOfNeighbours);

	//Next numberOfNodes lines is weight information
	for (int i = 0; i < numberOfNodes; i++) {
		fscanf(fp, "%s", buf);
		cout<<buf<<endl;
		nodes[i].serverId = atoi(buf);
//		cout<<"nodes[i].serverIp : "<<nodes[i].serverIp;
		fscanf(fp, "%s", buf);
		cout<<buf<<endl;
		nodes[i].serverIp = parse_ipaddress_string_to_uint32_t(buf);
		char *ip_str;
		uint32_t ip_int = parse_ipaddress_string_to_uint32_t(buf);
		ip_str = print_uint32_ip(ip_int);
		fscanf(fp, "%s", buf);
		cout<<buf<<endl;
		nodes[i].serverPort = atoi(buf);
	}
	//Parsed nodes should be in order in table
	struct nodes_table_info temp_node;
	for (int i = 0; i < numberOfNodes; i++) {
		for (int j = 0; j < numberOfNodes; j++) {
			if (nodes[j].serverId == (i + 1)) {
				temp_node = nodes[i];
				nodes[i] = nodes[j];
				nodes[j] = temp_node;
				break;
			}
		}
	}
	cout<<"Read Topology File"<<endl;
//	print_nodes_table();

	//Next numberOfNeighbours lines is
	for(int i=0;i<numberOfNeighbours;i++){
		fscanf(fp,"%s",buf);
		if(hostServerId == -1){
			hostServerId = atoi(buf);
		}else{
			if(hostServerId != atoi(buf)){
				cout<<"Invalid topology file. More than one host ?"<<endl;
			}
		}
		fscanf(fp,"%s",buf);
		int neighbour = atoi(buf);
		int j;
		for(j=0;j<numberOfNodes;j++){
			if(neighbour == nodes[j].serverId){
				break;
			}
		}
		nodes[j].isNeighbour = 1;
		fscanf(fp,"%s",buf);
		nodes[j].linkCost = atoi(buf);
		nodes[j].nextHopId = neighbour;//nodes[i].server_id;
	}
	for(int i=0;i<numberOfNodes;i++){
		if(hostServerId == nodes[i].serverId){
			nodes[i].linkCost = 0;
			nodes[i].nextHopId = hostServerId;
			break;
		}
	}
	fclose(fp);
	return 0;
}


