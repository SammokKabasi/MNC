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
#include <math.h>

using namespace std;

#define STDIN 0
#define MAXIMUM_TIMEOUTS 3

int numberOfNodes;

int numberOfNeighbours;

int hostServerId = -1;

char weights[10][3];

char* ip_address;

void print_nodes_table();

//void print_routing_table3();

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

	cout << endl << "The IP address is " << ip_address << endl;

	freeifaddrs(ifap);
	return 0;
}

int readTopologyFile(char* filePath);

char* portNumber; //port number of current process

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
int isNumber(char *input) {
	while (*input) {
		if (!isdigit(*input))
			return 0;
		else
			input += 1;
	}

	return 1;
}

int timeout = -1;

void printRoutingTable();

//Data structure
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

struct nodes_table_info nodes[10];

struct ServerInfo {
	uint32_t serverIp;
	uint16_t port;
	uint16_t blank;
	uint16_t serverId;
	uint16_t cost;
};

//This is the format of the message packet.

struct RoutingPacket {
	uint16_t numberOfUpdateFields;
	uint16_t serverPort;
	uint32_t serverIp;
	struct ServerInfo updateFields[10];
};

struct RoutingTableEntry {
	uint16_t srcServerId;
	uint16_t destServerId;
	int nextHopId;
	uint16_t linkCost;

	RoutingTableEntry() {
		srcServerId = -1;
		destServerId = -1;
		nextHopId = -1;
		linkCost = 0xffff;
	}
};
struct RoutingTableEntry routingTable[10][10];

/** TODO : change
 * converts string array ip address to a uint32_t object
 */
uint32_t parse_ipaddress_string_to_uint32_t(char* ipAddress) {
	struct in_addr ip_addr;
	inet_pton(AF_INET, ipAddress, &ip_addr);
	return (uint32_t) ntohl(ip_addr.s_addr);

}

/*TODO : change
 *
 * Convert uint32_t to String IP address
 */
char* print_uint32_ip(uint32_t ip) {
	char *ipAddress = (char*) malloc(INET_ADDRSTRLEN);
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

struct RoutingPacket* decodeUpdate(char* rcvPacketString);

/**TODO: change!
 *
 */
int createSocket(int listeningPort) {
	cout << endl << "in createSocket";
	int listen_sock_fd = -1;

	struct addrinfo listen_sock_details, *ai, *p;

	memset(&listen_sock_details, 0, sizeof(listen_sock_details));
	listen_sock_details.ai_family = AF_INET;
	listen_sock_details.ai_socktype = SOCK_DGRAM;
	listen_sock_details.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
	listen_sock_details.ai_protocol = 0;
	if (listeningPort != -1) {
		char portString[6];
		memset(&portString, '\0', 6);
		sprintf(portString, "%d", listeningPort);
		int retval = getaddrinfo(NULL, portString, &listen_sock_details, &ai);
		if (retval != 0) {
			fprintf(stderr, "UDP_socket creation selectserver: %s\n",
					gai_strerror(retval));
			return (-1);
		}
	} else {
		int retval = getaddrinfo(NULL, "50000", &listen_sock_details, &ai);
		if (retval != 0) {
			fprintf(stderr, "sending_socket creation selectserver: %s\n",
					gai_strerror(retval));
			return (-1);
		}
	}
	for (p = ai; p != NULL; p = p->ai_next) {
		listen_sock_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (listen_sock_fd < 0)
			continue;

		int y = 1;
		setsockopt(listen_sock_fd, SOL_SOCKET, SO_REUSEADDR, &y, sizeof(int));

		if (bind(listen_sock_fd, p->ai_addr, p->ai_addrlen) < 0) {
			close(listen_sock_fd);
			continue;
		}
		break;
	}

	if (p == NULL) {
		fprintf(stderr, "listen_socket selectserver: failed to bind\n");
		return (-2);
	}

	freeaddrinfo(ai);
	return listen_sock_fd;
}

/**
 * Reads the topology file, parses the values and populates the nodes_table_info struct object "nodes"
 */
int readTopologyFile(char* filePath) {

	cout << "inside readTopologyFile";

	FILE *fp;
	char buf[255];
	fp = fopen(filePath, "r");

	if (NULL == fp) {
		printf("Error opening file");
		return -1;
	}

	//Line 1 is number of nodes
	fscanf(fp, "%s", buf);

	numberOfNodes = atoi(buf);
	printf("number of nodes -->%d\n", numberOfNodes);

	//Line 2 is numberOfNeighbours
	fscanf(fp, "%s", buf);
	numberOfNeighbours = atoi(buf);
	printf("number of connections -->%d\n", numberOfNeighbours);

	//Next numberOfNodes lines is weight information
	for (int i = 0; i < numberOfNodes; i++) {
		fscanf(fp, "%s", buf);
		cout << buf << endl;
		nodes[i].serverId = atoi(buf);
//		cout<<"nodes[i].serverIp : "<<nodes[i].serverIp;
		fscanf(fp, "%s", buf);
		cout << buf << endl;
		nodes[i].serverIp = parse_ipaddress_string_to_uint32_t(buf);
		char *ip_str;
		uint32_t ip_int = parse_ipaddress_string_to_uint32_t(buf);
		ip_str = print_uint32_ip(ip_int);
		fscanf(fp, "%s", buf);
		cout << buf << endl;
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
	cout << "Read Topology File" << endl;
//	print_nodes_table();

	//Next numberOfNeighbours lines is
	for (int i = 0; i < numberOfNeighbours; i++) {
		fscanf(fp, "%s", buf);
		if (hostServerId == -1) {
			hostServerId = atoi(buf);
			cout << "IP address is " << hostServerId;
		} else {
			if (hostServerId != atoi(buf)) {
				cout << "Invalid topology file. More than one host ?" << endl;
			}
		}
		fscanf(fp, "%s", buf);
		int neighbour = atoi(buf);
		int j;
		for (j = 0; j < numberOfNodes; j++) {
			if (neighbour == nodes[j].serverId) {
				break;
			}
		}
		nodes[j].isNeighbour = 1;
		fscanf(fp, "%s", buf);
		nodes[j].linkCost = atoi(buf);
		nodes[j].nextHopId = neighbour; //nodes[i].server_id;
	}
	for (int i = 0; i < numberOfNodes; i++) {
		if (hostServerId == nodes[i].serverId) {
			nodes[i].linkCost = 0;
			nodes[i].nextHopId = hostServerId;
			break;
		}
	}
	fclose(fp);
	return 0;
}

int input(char buf[100]);

void updateRoutingTable(struct RoutingPacket* rcvPacket) {
	int sourceId = -1;

	//find the node object in the node array with the right server id..
	for (int i = 0; i < numberOfNodes; i++) {
		if (nodes[i].serverIp == rcvPacket->serverIp
				&& nodes[i].serverPort == rcvPacket->serverPort) {
			sourceId = nodes[i].serverId;
			break;
		}
	}

	//Update link cost in nodes table, using the link cost in the received packet
	for (int i = 0; i < numberOfNodes; i++) {
		for (int j = 0; j < numberOfNodes; j++) {
			for (int k = 0; k < numberOfNodes; k++) {
				if (routingTable[j][k].srcServerId == sourceId
						&& routingTable[j][k].destServerId
								== rcvPacket->updateFields[i].serverId) {
					routingTable[j][k].linkCost =
							rcvPacket->updateFields[i].cost;
					break;
				}
			}
		}
	}
}

int bellmanFord();

int bellmanFord() {

}


void sendUpdate();

int sendingPort = 38990; //start searching at this port and keep increasing till it finds an empty port
int listeningSocketFd = 0;
/**
 *
 * Used for debugging timing...
 */
void print_current_time_with_ms(void) {
	long ms; // Milliseconds
	time_t s;  // Seconds
	struct timespec spec;

	clock_gettime(CLOCK_REALTIME, &spec);

	s = spec.tv_sec;
	ms = round(spec.tv_nsec / 1.0e6); // Convert nanoseconds to milliseconds

	printf("Current time: %"PRIdMAX".%03ld seconds since the Epoch\n",
			(intmax_t) s, ms);
}

int sendingsocketfd = -1;
int main(int nNumberofArgs, char* args[]) {
	setvbuf(stdout, NULL, _IONBF, 0);
	int c;

	if (nNumberofArgs < 2) {
		fprintf(stderr, "Missing arguments\n");
		cout
				<< "Usage:   -t <topology-file-name> -i <routing-update-interval-in-seconds>";
		return -1;
	}

	char* filePath = (char*) malloc(200);

	//Get the ip address and port number and store it in the local variables
	getIP();

	/*TODO change!
	 * Parse the arguments
	 * http://www.gnu.org/software/libc/manual/html_node/Example-of-Getopt.html
	 */
	while ((c = getopt(nNumberofArgs, args, "t:i:")) != -1) {
		switch (c) {
		case 't':
			cout << "Topology file name:" << optarg << endl;
			strcpy(filePath, optarg);
			cout << "break";
			break;

		case 'i':
			cout << "case i : " << optarg;

			//argument should be an integer number (seconds)
			if (!isNumber(optarg)) {
				fprintf(stderr, "Invalid value for -i\n");
				cout << "Invalid value for -i\n";
				return -1;
			}
			timeout = atoi(optarg);
			break;

		case '?':
			break;

		default:
			cout << "Usage: " << args[0]
					<< "  -t <topology-file-name> -i <routing-update-interval-in-seconds>";

			return -1;
		}
	}

	cout << "Reading the topology file...";
	//read the local topology file, exit if invalid.
	if (readTopologyFile(filePath) == -1) {
		cout << "Topology file is invalid. Exiting...";
		exit(1);
	}

	cout << "Topology file read successfully";

	//Initializing the local routing table...

	//copy local nodes info table (populated from tpoology file) to routing table, so as to initialize it.

	for (int i = 0; i < numberOfNodes; i++) {
		for (int j = 0; j < numberOfNodes; j++) {
			routingTable[i][j].srcServerId = nodes[i].serverId;
			routingTable[i][j].destServerId = nodes[j].serverId;

			//If the node's server id is the current hostId, update the values for that table
			if (nodes[i].serverId == hostServerId) {
				routingTable[i][j].linkCost = nodes[j].linkCost;
				routingTable[j][i].linkCost = nodes[j].linkCost;
				routingTable[i][j].nextHopId = nodes[j].nextHopId;
			}
		}
	}

	cout << endl << "sock number : " << createSocket(59999) << endl;

	fd_set master;	//mastersocket
	fd_set read_fds; //read socket
	int fdmax; 	//number of fds in set
	FD_ZERO(&master);
	FD_ZERO(&read_fds);

	//get the listening port number of the host server id, and create a socket and listen at that port.
	//we know our server id. Listening port number is obtained by getting port number of that server id by looking up the nodes object.
	int i = 0;
	uint16_t port;
	while (i < numberOfNodes) {
		if (hostServerId == nodes[i].serverId) {
			port = nodes[i].serverPort;
			break;
		}
		i++;
	}

	i = 0;

	listeningSocketFd = createSocket(port);

	cout << endl << "Creating a socket for sending packets...";

	//Now create a socket for sending packets
	//lets select a port 38990 for sending, and keep incrementingg till it finds a valid empty port number

	cout << endl << sendingsocketfd << endl;
	while (sendingsocketfd == -1) {
		sendingsocketfd = createSocket(sendingPort++);
//		cout << endl<< "Sending messages from port " + sendingPort;
	}
	cout << endl << "sending socket fd : " << sendingsocketfd;
	cout << endl << "Sending messages from port " << sendingPort;

	FD_SET(STDIN, &master);
	FD_SET(sendingsocketfd, &master);
	FD_SET(listeningSocketFd, &master);

	fdmax = max(listeningSocketFd, sendingsocketfd);

	cout << endl << "Server started. Listening at port --->" << port;
	int packetsReceived;
	int byteCount;

	struct timeval tv;

	tv.tv_sec = timeout;
	tv.tv_usec = 0;

	cout << "Before while is starting..." << endl;
	for (;;) {
		memcpy(&read_fds, &master, sizeof(master));
//		cout<<"inside while... "<<endl;
		print_current_time_with_ms();
		int selectReturn = select(fdmax + 1, &read_fds, NULL, NULL, &tv);
//		cout << "inside while again..." << endl;
		if (selectReturn == -1) {
			perror("Error returned by select");
			exit(4);
		}

		//run through existing connections looking for data to read.
		for (int i = 0; i <= fdmax; i++) {
			fflush(stdin);
//			cout<<"Value of i, fdmax-->" << i<<","<<fdmax;
			if (FD_ISSET(i, &read_fds)) {
//				cout<<"Some input received.";
				if (i == STDIN) {
					char buf[100];
					memset(&buf, '\0', sizeof buf);
					read(0, buf, sizeof(buf));
					input(buf);
				}
				if (i == listeningSocketFd) {
					packetsReceived++;

					//Received packet
					char* buf = (char*) malloc(8 + 12 * numberOfNodes);

					struct sockaddr_storage addr;
					socklen_t fromlen;
					fromlen = sizeof(addr);
					byteCount = recvfrom(listeningSocketFd, buf,
							(8 + 12 * numberOfNodes), 0,
							(struct sockaddr*) &addr, &fromlen);

					//receiev the decode packet

					struct RoutingPacket* recv_pkt = decodeUpdate(buf);
					//reset num_timeout for the sender
					for (int i = 0; i < numberOfNodes; i++) {
						if (nodes[i].serverIp == recv_pkt->serverIp
								&& nodes[i].serverPort
										== recv_pkt->serverPort) {
							nodes[i].numberOfTimeouts = 0;
							cout << "Received a message from serverid "
									<< nodes[i].serverId;
							for (int p = 0; p < numberOfNodes; p++) {
								for (int q = 0; q < numberOfNodes; q++) {
									if (recv_pkt->updateFields[q].serverId
											== (p + 1)) {
										cout
												<< recv_pkt->updateFields[q].serverId
												<< " "
												<< recv_pkt->updateFields[q].cost
												<< endl;
									}
								}
							}
							break;
						}
					}
									updateRoutingTable(recv_pkt);
//									do_bellman_ford();
				}

			}
		}
		if (selectReturn == 0) {
			//INCREASE NUM_TIMEOUTS
			for (int i = 0; i < numberOfNodes; i++) {
				if (nodes[i].isNeighbour == 1) {
					nodes[i].numberOfTimeouts++;
				}
			}
			//CHECK FOR EXPIRED NEIGHBOURS, SET LINK COST TO INF
			for (int i = 0; i < numberOfNodes; i++) {
				if (nodes[i].isNeighbour
						== 1&& nodes[i].numberOfTimeouts > MAXIMUM_TIMEOUTS) {
					nodes[i].linkCost = 0xffff;
					routingTable[hostServerId - 1][i].linkCost = 0xffff;
					//TEST BI-D
					routingTable[i][hostServerId - 1].linkCost = 0xffff;
					//SET ALL ROUTING COSTS TO INF FOR THAT NODE
					for (int j = 0; j < numberOfNodes; j++) {
						routingTable[i][j].linkCost = 0xffff;
					}
				}
			}
//					do_bellman_ford();
					sendUpdate();
			tv.tv_sec = timeout;
			tv.tv_usec = 0;
		}
		cout << "End of while loop" << endl;
	}
	return 0;

}

int packetsReceived;

int disableServer(int serverId) {
	for (int i = 0; i < numberOfNodes; i++) {
		if (nodes[i].isNeighbour == 1 && nodes[i].serverId == serverId) {
			nodes[i].isDisabled = 1;
			cout << "Server ID " << serverId << " successfully disabled. ";
			return 0;
		}
	}
	//if server id is not found...
	cout << "Invalid server id.";
	return -1;
}

int input(char inputBuf[25]) {

	cout << "host server id is --> " << hostServerId;

	cout << "some input received : " << inputBuf << endl;

	int i = 0;
	char* token;
	while ((token = strsep(&inputBuf, " "))) {
		i++;
//		cout << "i value : " << i <<", token -->" <<token<<endl;
		if (strcmp(token, "update") == 0) {
			int sourceServerUpdateId = atoi(strsep(&inputBuf, " "));
			int destinationServerUpdateId = atoi(strsep(&inputBuf, " "));
			char* cost = strsep(&inputBuf, " ");
//			cout<<"The cost string is " <<cost;
			uint16_t cost1;
			if (strcmp(cost, "inf") == 0) {
				cost1 = 0xffff;
			} else {
//				cout << "inside else";
				cost1 = atoi(cost);
			}
			cout << "cost (to be updated ) is " << cost1;
// update routing table here
			routingTable[sourceServerUpdateId - 1][destinationServerUpdateId - 1].linkCost =
					cost1;
			routingTable[destinationServerUpdateId - 1][sourceServerUpdateId - 1].linkCost =
					cost1;
			if (sourceServerUpdateId == hostServerId) {
				nodes[sourceServerUpdateId - 1].linkCost = cost1;
			}
			if (destinationServerUpdateId == hostServerId) {
				nodes[sourceServerUpdateId - 1].linkCost = cost1;
			}
			cout << "Successfully updated routing table.";
		} else if (strcmp(token, "crash") == 0 || strcmp(token, "CRASH") == 0) {
			cout << "Simulating crash.. \nclosing sockets";
			close(sendingsocketfd);
			close(listeningSocketFd);
			cout << "Sockets closed.";
		} else if (strcmp(token, "step") == 0 || strcmp(token, "STEP") == 0) {
			cout << "Sending an update.. ";
			sendUpdate();
		} else if (strcmp(token, "packets") == 0
				|| strcmp(token, "PACKETS") == 0) {
			cout << "Number of packets received : " << packetsReceived;
		} else if (strcmp(token, "display") == 0
				|| strcmp(token, "DISPLAY") == 0) {
			printRoutingTable();
		} else if (strcmp(token, "disable") == 0
				|| strcmp(token, "DISABLE") == 0) {
			char* serverToDisable = strsep(&inputBuf, " ");
			int serverToDisable_int = atoi(serverToDisable);

			disableServer(serverToDisable_int);
		} else {
			cout << "Invalid input.";
			return -1;

		}
	}

	return -2;

}

void sendUpdate() {

}

//void printRoutingTable() {
//	cout << endl << "Routing table : " << endl;
//
//	cout<<"\t";
//	for (int i = 0; i < numberOfNodes; i++) {
//		cout << "|Node " <<i << "\t";
//	}
//	cout<<endl;
//	for (int i = 0; i < numberOfNodes; i++) {
//		cout<<"Node "<<i;
//		for (int j = 0; j < numberOfNodes; j++) {
//			cout <<"\t|" << routingTable[i][j].linkCost;
//		}
//		cout << endl;
//	}
//}

/*
 * This method converts the byte array into a routing packet structure
 * params (char* recv_packet)
 * return struct routing_packet*
 *
 * TODO change
 */
struct RoutingPacket* decodeUpdate(char* rcvPacketString) {
	struct RoutingPacket *receivedRoutingPacket = (RoutingPacket*) malloc(
			sizeof(RoutingPacket));

	uint16_t temp16 = -1;
	uint32_t temp32 = -1;

//numberOfUpdateFields
	memcpy(&temp16, rcvPacketString, sizeof(uint16_t));
	temp16 = ntohs(temp16);
	memcpy(&receivedRoutingPacket->numberOfUpdateFields, &temp16, sizeof(uint16_t));
	rcvPacketString += sizeof(uint16_t);
//serverPort
	memcpy(&temp16, rcvPacketString, sizeof(uint16_t));
	temp16 = ntohs(temp16);
	memcpy(&receivedRoutingPacket->serverPort, &temp16, sizeof(uint16_t));
	rcvPacketString += sizeof(uint16_t);
//serverIp
	memcpy(&temp32, rcvPacketString, sizeof(uint32_t));
	temp32 = ntohl(temp32);
	memcpy(&receivedRoutingPacket->serverIp, &temp32, sizeof(uint32_t));
	rcvPacketString += sizeof(uint32_t);

	for (int i = 0; i < receivedRoutingPacket->numberOfUpdateFields; i++) {

		struct ServerInfo *server = (ServerInfo*) malloc(sizeof(server));
		//READ next 4 bytes server_ip
		memcpy(&temp32, rcvPacketString, sizeof(uint32_t));
		temp32 = ntohl(temp32);
		memcpy(&server->serverIp, &temp32, sizeof(uint32_t));
		rcvPacketString += sizeof(uint32_t);

		//READ next 2 bytes server_port
		memcpy(&temp16, rcvPacketString, sizeof(uint16_t));
		temp16 = ntohs(temp16);
		memcpy(&server->port, &temp16, sizeof(uint16_t));
		rcvPacketString += sizeof(uint16_t);

		//READ next 2 bytes dummy
		memcpy(&temp16, rcvPacketString, sizeof(uint16_t));
		temp16 = ntohs(temp16);
		memcpy(&server->blank, &temp16, sizeof(uint16_t));
		rcvPacketString += sizeof(uint16_t);

		//READ next 2 bytes server_id
		memcpy(&temp16, rcvPacketString, sizeof(uint16_t));
		temp16 = ntohs(temp16);
		memcpy(&server->serverId, &temp16, sizeof(uint16_t));
		rcvPacketString += sizeof(uint16_t);

		//READ next 2 bytes link_cost
		memcpy(&temp16, rcvPacketString, sizeof(uint16_t));
		temp16 = ntohs(temp16);
		memcpy(&server->cost, &temp16, sizeof(uint16_t));
		rcvPacketString += sizeof(uint16_t);

		memcpy(&receivedRoutingPacket->updateFields[i], server, sizeof(ServerInfo));
	}
	return receivedRoutingPacket;
}

void printRoutingTableWithProperFormat() {
	cout << endl << "Routing table : " << endl;

	cout << "\t";
	for (int i = 0; i < numberOfNodes; i++) {
		cout << "|Node " << i << "\t";
	}
	cout << endl;
	for (int i = 0; i < numberOfNodes; i++) {
		cout << "Node " << i;
		for (int j = 0; j < numberOfNodes; j++) {
			cout << "\t|" << routingTable[i][j].linkCost;
		}
		cout << endl;
	}
}
/*
 * Print routing table
 */
void printRoutingTable() {
cout << "\nserverId\t|nextHopId\t|linkCost\n";
for (int i = 0; i < numberOfNodes; i++) {
	cout << nodes[i].serverId << "\t";
	for (int j = 0; j < numberOfNodes; j++) {
		cout << "c(" << routingTable[i][j].srcServerId << ","
				<< routingTable[i][j].destServerId << ") = "
				<< routingTable[i][j].linkCost << ", ";
		cout << "NH(" << routingTable[i][j].srcServerId << ","
				<< routingTable[i][j].destServerId << ") = "
				<< routingTable[i][j].nextHopId << "\t|";
	}
	printf("\n");
}
}

