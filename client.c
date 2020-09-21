#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <dirent.h>
#include <time.h>

#define BUFFER_SIZE 61000	//Max size of data in a single frame

//A frame packet with sequence id, length of frame and the content of file
 struct frame_st {
 	long int SeqID;
 	long int length;
 	char data[BUFFER_SIZE];
 };

	struct sockaddr_in to_addr, from_addr;
	struct frame_st frame;
	struct timeval t_out = {0, 0};

	char cmd_send[50];
	char flname[20];
	char cmd[10];
	char ack_send[4] = "ACK";
	
	ssize_t numRead = 0;
	ssize_t length = 0;
	off_t f_size = 0;
	long int ack_num = 0;
	int clientd, ack_recv = 0;

	long int frame_count = 0;
	long int bytes_received = 0, i = 0;
	FILE *fptr;

static void print_error(char *msg)
{
	perror(msg);
	exit(EXIT_FAILURE);
}


int main(int argc, char **argv)
{
	if ((argc < 3) || (argc >3)) {
		printf("Client: Usage --> ./[%s] [IP Address] [Port Number]\n", argv[0]); 
		exit(EXIT_FAILURE);
	}


	/*Clear all the data buffer and structure*/
	memset(ack_send, 0, sizeof(ack_send));
	memset(&to_addr, 0, sizeof(to_addr));
	memset(&from_addr, 0, sizeof(from_addr));

	/*Populate to_addr structure with IP address and Port*/
	to_addr.sin_family = AF_INET;
	to_addr.sin_port = htons(atoi(argv[2]));
	to_addr.sin_addr.s_addr = inet_addr(argv[1]);

	if ((clientd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) //socket descriptor
		print_error("Client: socket");

	for (;;) {

		memset(cmd_send, 0, sizeof(cmd_send));
		memset(cmd, 0, sizeof(cmd));
		memset(flname, 0, sizeof(flname));

		printf("\n Enter the file name to fetch\n");		
		scanf(" %[^\n]%*c", cmd_send);
		
		sscanf(cmd_send, "%s", flname);

		if (sendto(clientd, cmd_send, sizeof(cmd_send), 0, (struct sockaddr *) &to_addr, sizeof(to_addr)) == -1) {
			print_error("Client: send");
		}

	if ((flname[0] != '\0' )) {
			
			t_out.tv_sec = 0;
			t_out.tv_usec = 0;
            setsockopt(clientd, SOL_SOCKET, SO_RCVTIMEO, (char *)&t_out, sizeof(struct timeval)); 	//Disabling the timeout option on client side
			recvfrom(clientd, &(frame_count), sizeof(frame_count), 0, (struct sockaddr *) &from_addr, (socklen_t *) &length); //Informing user the size of file by total frame

			if (frame_count > 0) {
				sendto(clientd, &(frame_count), sizeof(frame_count), 0, (struct sockaddr *) &to_addr, sizeof(to_addr)); //Acknowledge the receipt of file size
				printf("Total Frames to be received from server %ld with each frame containing 61K Bytes data\n", frame_count);
			}
				fptr = fopen(flname, "wb");
				//Recieve the frames sequentially and send an acknowledgement for each frame received
				int i=1;
				while (frame.SeqID !=frame_count){
					memset(&frame, 0, sizeof(frame));
					recvfrom(clientd, &(frame), sizeof(frame), 0, (struct sockaddr *) &from_addr, (socklen_t *) &length);  //Recieve the frame
					if (frame.SeqID == frame_count)
					{
						fprintf(stdout, " Time Stamp of last bit received = %lu\n", (unsigned long)time(NULL));
						printf("File received\n");
					}
					sendto(clientd, &(frame.SeqID), sizeof(frame.SeqID), 0, (struct sockaddr *) &to_addr, sizeof(to_addr));	//Send Acknowledgement for each frame received successfully
					//Write content into file only if correct frame received
					 if (frame.SeqID == i){
					 	i++;
						fwrite(frame.data, 1, frame.length, fptr);   //Write the recieved data to the file
						bytes_received += frame.length;
					}
				}
				printf("Total bytes recieved = %ld\n", bytes_received);
				fclose(fptr);
			}
		}
	close(clientd);
	exit(EXIT_SUCCESS);
}