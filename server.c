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
#include <stdbool.h>

#define BUF_SIZE 61000		//Max buffer size of the data in a frame

//A frame packet with sequence id, length of frame and the content of file
struct frame_st {
	long int SeqID;
	long int length;
	char data[BUF_SIZE];
};
	struct sockaddr_in server_addr, cl_addr;
	struct stat st;
	struct frame_st frame;
	struct timeval t_out = {0, 0};

	char msg_recv[BUF_SIZE];
	char flname_recv[20];         
	char cmd_recv[10];

	ssize_t numRead;
	ssize_t length;
	off_t f_size;

	long int ack_num = 0;
	int ack_send = 0;
	int serverd;
	int frame_count = 0;
	long int i = 0;
	FILE *fptr;

int main(int argc, char **argv)
{
	//Check for appropriate commandline arguments
		if ((argc < 2) || (argc > 2)) {			
			printf("Usage --> ./[%s] [Port Number]\n", argv[0]);
			exit(EXIT_FAILURE);
		}	

	//Populate Structure with port and IP address
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(argv[1]));
	server_addr.sin_addr.s_addr = INADDR_ANY;

	if ((serverd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
		perror("Server: socket");

	if (bind(serverd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1)
		perror("Server: bind");

	for(;;) {
		printf("Waiting for client to connect\n");

		memset(msg_recv, 0, sizeof(msg_recv));
		memset(cmd_recv, 0, sizeof(cmd_recv));
		memset(flname_recv, 0, sizeof(flname_recv));

		length = sizeof(cl_addr);

		if((numRead = recvfrom(serverd, msg_recv, BUF_SIZE, 0, (struct sockaddr *) &cl_addr, (socklen_t *) &length)) == -1)
			perror("Server: receive");

		sscanf(msg_recv, "%s", flname_recv);

		if ( (flname_recv[0] != '\0')) {
		printf("Requested File Name = %s\n", flname_recv);

			if (access(flname_recv, F_OK) == 0){			//Check if file exist
					
				stat(flname_recv, &st);
				f_size = st.st_size;			//Size of the file

				t_out.tv_sec = 0;			
				t_out.tv_usec = 250000;
				setsockopt(serverd, SOL_SOCKET, SO_RCVTIMEO, (char *)&t_out, sizeof(struct timeval));   //Setting timeout option for receiving ACK

				fptr = fopen(flname_recv, "rb");                       
				if ((f_size % BUF_SIZE) != 0)
					frame_count = (f_size / BUF_SIZE) + 1;				//Total number of frames to be sent
				else
					frame_count = (f_size / BUF_SIZE);
					
				length = sizeof(cl_addr);
				fprintf(stdout, "Time Stamp of first bit sent = %lu \n", (unsigned long)time(NULL)); 
				sendto(serverd, &(frame_count), sizeof(frame_count), 0, (struct sockaddr *) &cl_addr, sizeof(cl_addr));	//Send file size to client
				recvfrom(serverd, &(ack_num), sizeof(ack_num), 0, (struct sockaddr *) &cl_addr, (socklen_t *) &length); 

				while (ack_num != frame_count){		//Check for the acknowledgement
				
					//Keep retrying until the ack matches
					sendto(serverd, &(frame_count), sizeof(frame_count), 0, (struct sockaddr *) &cl_addr, sizeof(cl_addr)); 
					while (recvfrom(serverd, &(ack_num), sizeof(ack_num), 0, (struct sockaddr *) &cl_addr, (socklen_t *) &length) < 0){
				            sendto(serverd, &(frame_count), sizeof(frame_count), 0, (struct sockaddr *) &cl_addr, sizeof(cl_addr));		//If timeout on receiving ACK, retransmit the frame
					}
				}
				int i = 1;
            	bool flag=true;
            	ack_num=0;
				    while(ack_num !=frame_count){
				            memset(&frame, 0, sizeof(frame));
				            frame.SeqID = i;
				            if(flag)
				                frame.length = fread(frame.data, 1, BUF_SIZE, fptr);
				            sendto(serverd, &(frame), sizeof(frame), 0, (struct sockaddr *) &cl_addr, sizeof(cl_addr));		//send the frame
							while (recvfrom(serverd, &(ack_num), sizeof(ack_num), 0, (struct sockaddr *) &cl_addr, (socklen_t *) &length) < 0){
				            sendto(serverd, &(frame), sizeof(frame), 0, (struct sockaddr *) &cl_addr, sizeof(cl_addr));
							}

				            if(ack_num == frame.SeqID){
				                i++;
				                flag=true;
				            }
				            else{
				                flag=false;
				            }

				            if (frame_count == ack_num){
				                printf("File sent\n");
								exit(1);
				            }
				        }
				fclose(fptr);
			}
		}
	}
	close(serverd);
	return 0;
}