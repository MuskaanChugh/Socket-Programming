//2015B4A70620P Yajat Dawar
//2015B2A70881P Muskaan Chugh

#include<stdio.h> //printf
#include<string.h> //memset
#include<stdlib.h> //exit(0);
#include<arpa/inet.h>
#include<sys/socket.h>
 
#define BUFLEN 512  //Max length of buffer   
 
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

int main(int argc, char* argv[]){
	int PORT = atoi(argv[1]);
	struct sockaddr_in si_other;
    int rv,snd,s,i,slen=sizeof(si_other);

    int curr_size=0;
    char message[BUFLEN];
    ACK ack_pkt;
    RRQ rrq_pkt;
    DATA data_pkt;
    if ( (s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }


    memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(PORT);
    si_other.sin_addr.s_addr = inet_addr("127.0.0.1");
    
	int state = 0;

	strcpy(rrq_pkt.Filename, "input.txt");
	strcpy(rrq_pkt.Mode, "octet");

	printf("Sending RRQ\n");
	if (sendto(s, &rrq_pkt, sizeof(rrq_pkt), 0 , (struct sockaddr *) &si_other, slen)==-1)
  		{ 
      		die("sendto()");
  		}

  	printf("Waiting for RRQ acknowledgement\n");
  	if (recvfrom(s, &ack_pkt, sizeof(ack_pkt), 0, (struct sockaddr *) &si_other, &slen) == -1)
		{ 
			die("recvfrom()");
		}
//	sleep(0.005);

  	printf("Waiting for data\n");


  	while(1){
  		switch(state)
  		{
  		case 0:
  		{
  			if (recvfrom(s, &data_pkt, sizeof(data_pkt), 0, (struct sockaddr *) &si_other, &slen) == -1)
			{ 
				die("recvfrom()");
			}
			else{
				printf("Data packet seq %d recieved successfully\n", curr_size);
			//	printf("%s\n", data_pkt.data);
				curr_size++;
				ack_pkt.sq_no = 0;
				if (sendto(s, &ack_pkt, sizeof(ack_pkt), 0 , (struct sockaddr *) &si_other, slen)==-1)
		  		{ 
		      		die("sendto()");
		  		}
		  		if(strlen(data_pkt.data)<511){
		  			printf("recieved last packet now exiting\n");
		  			return 0;
		  		}
				state = 1;
			}	
  		}

  		case 1:
  		{
  			if (recvfrom(s, &data_pkt, sizeof(data_pkt), 0, (struct sockaddr *) &si_other, &slen) == -1)
			{ 
				die("recvfrom()");
			}
			else{
				printf("Data packet seq %d recieved successfully\n", curr_size);
			//	printf("%s\n", data_pkt.data);
				curr_size++;
				ack_pkt.sq_no = 1;
				if (sendto(s, &ack_pkt, sizeof(ack_pkt), 0 , (struct sockaddr *) &si_other, slen)==-1)
		  		{ 
		      		die("sendto()");
		  		}
		  		if(strlen(data_pkt.data)<511){
		  			printf("recieved last packet now exiting\n");
		  			return 0;
		  		}
				state = 0;
			}
  		}
  	}
  }
  	

close(s);

}