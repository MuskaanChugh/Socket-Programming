//2015B4A70620P Yajat Dawar
//2015B2A70881P Muskaan Chugh

#include<stdio.h> //printf
#include<string.h> //memset
#include<stdlib.h> //exit(0);
#include<arpa/inet.h>
#include<sys/socket.h>
#include <time.h>
 
#define BUFLEN 512  //Max length of buffer
#define PORT2 8882   //The port on which to listen for incoming data
 
double alpha = 0.5;
// Initializing the RTT Values to 1
// Initializing value of alpha to 0.5
double initialRTT1 = 1.5;
double initialRTT2 = 1.5;
clock_t start0, end1, start2, end3;
double cpu_time_used1;
double cpu_time_used2;

double calculateRTT(double oldRTT, double ActualRTT)
{
	return (alpha*oldRTT + (1-alpha)*ActualRTT);
}


double calculateTimeout(double RTT)
{
	return 2*RTT;
}

void die(char *s)
{
    perror(s);
    exit(1);
}
 
typedef struct packet1{
    int sq_no;
}ACK;

typedef struct packet2{
    int sq_no;
    char data[BUFLEN];
}DATA;

typedef struct packet3{
    char Filename[20];
    char Mode[20];
}RRQ;

typedef struct packet4{
    int ErrorCode;
    char ErrMsg[50];
}ERROR;

int flag=0;


DATA getDataPkt(DATA* data_pkt,FILE* fptr,int *flag)
{
	char* temp = (char*)malloc(BUFLEN);
	char ch;
	int z=0;
	while(z<BUFLEN-1 && (ch = fgetc(fptr)) != EOF){
		temp[z++]=ch;
	}
	if(ch==EOF){
		if(z==0) //file size is a multiple of 512.
		{
			data_pkt->data[0]='\0';
			return *data_pkt;
		}
		*flag=1;
	}
	temp[z]='\0';
//	printf("%s\n", temp);
	int k=0;
	while(k<=z){
		 data_pkt->data[k]=temp[k];
		 k++;
//		 printf("%c", temp[k]);
	}
//	strcpy(data_pkt->data, temp);
//	printf("%s\n", data_pkt->data);
	return *data_pkt;
}

int main(int argc, char* argv[])
{
	int PORT = atoi(argv[1]);
    struct sockaddr_in si_me, si_other;
     
    int snd, s, snew, i, slen = sizeof(si_other) , recv_len;
    int FLAG=1;   
    DATA data_pkt;
    ACK  ack_pkt;
    RRQ rrq_pkt;
    ERROR err_pkt;

    int curr_size=0;
    int curr_sent=0;
    //create a UDP socket
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }
     
    // zero out the structure
    memset((char *) &si_me, 0, sizeof(si_me));
     
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(PORT);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
     
    //bind socket to port
    if( bind(s , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1)
    {
        die("bind");
    }
    
    printf("Waiting for RRQ from sender...\n");
    fflush(stdout);

    //try to receive some data, this is a blocking call
    if ((recv_len = recvfrom(s, &rrq_pkt, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == -1)
	{
		die("recvfrom()");
	}

	printf("RRQ received\n");

	//send RRQ ack to client for sending the new port
	if ((snew=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }
     
    // zero out the structure
 //   memset((char *) &si_other, 0, sizeof(si_other));

    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(PORT2);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);

    //bind socket to port
    if( bind(snew , (struct sockaddr*)&si_me, sizeof(si_other) ) == -1)
    {
        die("bind");
    }
     


    if (sendto(snew, &ack_pkt, sizeof(ack_pkt), 0 , (struct sockaddr *) &si_other, slen)==-1)
		{ 
      		die("sendto()");
  		}


	FILE* fptr = fopen(rrq_pkt.Filename, "r");
	data_pkt = getDataPkt(&data_pkt, fptr, &flag);

	int time_check;
		struct timeval timeout;
		//Setting timeout of 5 seconds
		timeout.tv_sec  = 5;
   	timeout.tv_usec = 0;
   	fd_set writefds,readfds; 

   	int retransmission=0;

    int state =0;
    while(1)
    {	
	      
		switch(state)
		{  
			case 0:
			{
				if(retransmission>=3){
					printf("Cannot retransmit more than 3 times\n");
					return 0;
					if (sendto(s, &err_pkt, sizeof(err_pkt), 0 , (struct sockaddr *) &si_other, slen)==-1)
			  		{ 
			      		  die("sendto()");
			  		}
				}
				printf("sending DATA pkt with seq no %d \n", curr_size);//wait for sending packet with seq. no. 0
		 		data_pkt.sq_no = 0;	
		 		printf("Total bytes sent= %d\n",curr_sent);
		 	//	printf("%d\n",strlen(data_pkt.data) );	
		 	//	printf("%s\n", data_pkt.data);
					if (sendto(s, &data_pkt, sizeof(data_pkt), 0 , (struct sockaddr *) &si_other, slen)==-1)
			  		{ 
			      		  die("sendto()");
			  		}
			  		usleep(5000); 
					start0 = clock();
			  		printf("Timeout interval for this packet is %f ms\n",calculateTimeout(initialRTT1));
					state = 1; 
				break;
			}
			
			case 1:
			{
				FD_ZERO(&readfds);
				FD_SET(s, &readfds);
				FD_ZERO(&writefds);
				FD_SET(s, &writefds);
					
				timeout.tv_sec  = 5;
	   			timeout.tv_usec = 0;
				time_check=select(s+1, &readfds, NULL, NULL, &timeout); 
		   	
	      		if(time_check==0){
					printf("Timeout\n");
					retransmission++;
					state = 0;
					break;	
				}
				else{
					if (recvfrom(s, &ack_pkt, sizeof(ack_pkt), 0, (struct sockaddr *) &si_other, &slen) == -1)
					{ 
			    		  die("recvfrom()");
					}

					end1 = clock();
					cpu_time_used1 =  ((double) (end1 - start0)); // / CLOCKS_PER_SEC;

	   				initialRTT1 = calculateRTT(initialRTT1,cpu_time_used1);					
	
					if (ack_pkt.sq_no==0)
					{
						printf("Received ack seq. no. %d\n",curr_size);
						printf("RTT Value for sending the next packet = %f ms\n",initialRTT1);
						retransmission=0;
						if(flag==1){
							printf("Sent all the data now exiting\n");
							return 0;
						}
						curr_size++;
						curr_sent+=strlen(data_pkt.data)+1;
						data_pkt = getDataPkt(&data_pkt,fptr, &flag);
						
						state = 2; 
		 			}
					
				}
			break;
			}

			case 2:
			{
				if(retransmission>=3){
					printf("Cannot retransmit more than 3\n");
					return 0;
					if (sendto(s, &err_pkt, sizeof(err_pkt), 0 , (struct sockaddr *) &si_other, slen)==-1)
			  		{ 
			      		  die("sendto()");
			  		}
				}
				printf("sending DATA pkt with seq no %d\n", curr_size);//wait for sending packet with seq. no. 0
				printf("Total bytes sent= %d\n", curr_sent);
			//	printf("%d\n",strlen(data_pkt.data) );	
			//	printf("%s\n", data_pkt.data);
		 		data_pkt.sq_no = 1;		
					if (sendto(s, &data_pkt, sizeof(data_pkt), 0 , (struct sockaddr *) &si_other, slen)==-1)
			  		{ 
			      		  die("sendto()");
			  		}
			  		usleep(5000);
			  	//	sleep(0.5);
			  		start2 = clock();
			  		printf("Timeout interval for this packet is %f ms\n",calculateTimeout(initialRTT2));
					state = 3; 
				break;

			} 

			case 3:
			{
				FD_ZERO(&readfds);
				FD_SET(s, &readfds);
				FD_ZERO(&writefds);
				FD_SET(s, &writefds);
					
				timeout.tv_sec  = 5;
	   			timeout.tv_usec = 0;
				time_check=select(s+1, &readfds, NULL, NULL, &timeout); 
		   	
	      		if(time_check==0){
					printf("Timeout\n");
					state = 0;
					break;	
				}
				else{
					if (recvfrom(s, &ack_pkt, sizeof(ack_pkt), 0, (struct sockaddr *) &si_other, &slen) == -1)
					{ 
			    		  die("recvfrom()");
					}

					end3 = clock();

					cpu_time_used2 = ((double) (end3 - start2)); // / CLOCKS_PER_SEC;

					initialRTT2 = calculateRTT(initialRTT2,cpu_time_used2);					
	
					if (ack_pkt.sq_no==1)
					{
						printf("Received ack seq. no. %d\n",curr_size);
						printf("RTT Value for sending the next Packet is %f ms\n",initialRTT2);
						if(flag==1){
							printf("Sent all the data now exiting\n");
							return 0;
						}
						data_pkt = getDataPkt(&data_pkt,fptr, &flag);
						curr_size++;
						curr_sent+=strlen(data_pkt.data)+1;
						retransmission=0;
						state = 0; 
		 			}	

				}
			break;
			}
		}
	}
	fclose(fptr);
    close(s);
    return 0;
}


