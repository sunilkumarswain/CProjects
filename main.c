/*

Main process for creating several threads and communicating & syncronizing between threads
Command to exeute this program: gcc main.c -lpthread
Threads: 
Task: Receive command from client and send it to firmware board
EEPROM commands handling
DAC commands handling
led comands handling
ssd commands handling
IoT Handling to publish and subscribe data through MQTT Server

*/
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>    //write
#include <pthread.h> //for threading , link with lpthread
#include <netdb.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <errno.h>
#include <wiringPi.h>
#include <wiringSerial.h>

char tbuffer[100] = "led -s A 0";
char dbuffer[100] = "ddd -s A 0";
char rbuffer[100] = "rrr -s A 0";
int maxlen = 100;
char tbuffer[100];
pthread_t tid[3];
int counter;
static volatile int balance = 0;
pthread_mutex_t mutex; 
char host[NI_MAXHOST];
char *res;
char *FindIpAddress()
{
    struct ifaddrs *ifaddr, *ifa;
    int family, s;
    

    if (getifaddrs(&ifaddr) == -1) 
    {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }


    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) 
    {
        if (ifa->ifa_addr == NULL)
            continue;  

        s=getnameinfo(ifa->ifa_addr,sizeof(struct sockaddr_in),host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);

        if((strcmp(ifa->ifa_name,"eth0")==0)&&(ifa->ifa_addr->sa_family==AF_INET))
        {
            if (s != 0)
            {
                printf("getnameinfo() failed: %s\n", gai_strerror(s));
                exit(EXIT_FAILURE);
            }
            printf("\tInterface : <%s>\n",ifa->ifa_name );
            printf("\t  Address : <%s>\n", host); 
        }
    }
    freeifaddrs(ifaddr);
    return host;
    //exit(EXIT_SUCCESS);
}


int PicAppServer()
{
	res = FindIpAddress();
    printf("Ip Address:%s\n", res);

	/* Required for acting as server */
	// port to start the server on
	int SERVER_PORT = 2011;

	// socket address used for the server
	struct sockaddr_in server_address;
	memset(&server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;

	// htons: host to network short: transforms a value in host byte
	// ordering format to a short value in network byte ordering format
	server_address.sin_port = htons(SERVER_PORT);

	// htonl: host to network long: same as htons but to long
	server_address.sin_addr.s_addr = inet_addr(res);

	// create a TCP socket, creation returns -1 on failure
	int listen_sock;
	if ((listen_sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		printf("could not create listen socket\n");
		return 1;
	}

	// bind it to listen to the incoming connections on the created server
	// address, will return -1 on error
	if ((bind(listen_sock, (struct sockaddr *)&server_address,
	          sizeof(server_address))) < 0) {
		printf("could not bind socket\n");
		return 1;
	}

	int wait_size = 16;  // maximum number of waiting clients, after which
	                     // dropping begins
	if (listen(listen_sock, wait_size) < 0) {
		printf("could not open socket for listening\n");
		return 1;
	}

	// socket address used to store client address
	struct sockaddr_in client_address;
	int client_address_len = 0;

	// run indefinitely
	while (1) {
		// open a new socket to transmit data per connection
		int sock;
		if ((sock =
		         accept(listen_sock, (struct sockaddr *)&client_address,
		                &client_address_len)) < 0) {
			printf("could not open a socket to accept data");
			return 1;
		}
        
		int n = 0;
		//int len = 0, 

		char *rbuffer = tbuffer; 
		char response[25];
		char LedCmd[20];
		char SsdCmd[20];

		// keep running as long as the client keeps the connection open
		while ((n = recv(sock, rbuffer, maxlen, 0)) > 0)
		{
			//printf("Size of data from client: %d\n", n);
			rbuffer += n;
			maxlen -= n;
			//len += n;
			tbuffer[n] = '\0';
			//printf("received from client: %s", rbuffer);
			strcpy(LedCmd,"blink");
			strcpy(SsdCmd,"display");
			printf("received from client: %s", tbuffer);
			//f_word = getfword(tbuffer);
			
			//f_word = "display"; //TO DO get the first word from tbuffer
			/*
			printf("first word from client: %s\n", f_word);
			if(strcmp(LedCmd,f_word) == 0)
        	{
        		words = NumOfWords(tbuffer);
        		printf("No of words in given led string is: %d\n",words);
        		if(words < 3)
        		{
        			strcpy(response,"Invalid Command,blink command usage example: blink port_no pin_no\n");
        		}else
        		{
        			AppClient(tbuffer,response);
        		} 
	        }else if(strcmp(SsdCmd,f_word) == 0)
	        {
	        	ssd_words = NumOfWords(tbuffer);
        		printf("No of words in given ssd string is: %d\n",ssd_words);
        		if(ssd_words < 3)
        		{
        			strcpy(response,"Invalid Command,display command usage example: display ssd_no number\n");
        		}else
        		{
        			AppClient(tbuffer,response);
        		}
	        }else
	        {
	        	strcpy(response,"Invalid Command,Try Again with a valid command\n");
	        }
			*/
			//AppClient(tbuffer,response);
			// echo received content back
			//printf("Data to be sent to GUI: %s", response);
			//n = 0;
			//maxlen = 100;
			//rbuffer = '\0';
			//send(sock, response, strlen(response), 0);
			send(sock, tbuffer, strlen(tbuffer), 0);
		}
		close(sock);
	}
	close(listen_sock);
	return 0;
}


void *Led_Thread(void *arg)
{
    printf("Led Thread Started\n");
    //char *who = arg;
    unsigned long i;
    //printf("%s: begin\n", who);
    
    //printf("%s: done\n", who);
    while(1)
    {
    	if(strcmp(tbuffer,"led -s A 0") == 0)
    	{
    		for (i = 0; i < 1000000; i++) 
		    {
		    	pthread_mutex_lock(&mutex);
		        balance = balance + 1;
		        pthread_mutex_unlock(&mutex);
		    }
		    printf("Balance is from Led Thread %d\n",balance);
    		printf("Execute the led cmd to fw board\n");
    		tbuffer[0] = '\0';
    	}
    }
}

void *Deposit(void *arg)
{
    printf("Diposit Thread Started\n");
    //char *who = arg;
    unsigned long i;
    while(1)
    {
    	if(strcmp(dbuffer,"ddd -s A 0") == 0)
    	{
    		for (i = 0; i < 1000000; i++) 
		    {
		    	pthread_mutex_lock(&mutex);
		        balance = balance + 1;
		        pthread_mutex_unlock(&mutex);
		    }
		    printf("Balance is from Deposit Thread %d\n",balance);
    		printf("Execute the ddd cmd to fw board\n");
    		dbuffer[0] = '\0';
    	}
    }
}

void *Display(void *arg)
{
	printf("Display Thread Started\n");
	unsigned long i;
    while(1)
    {
    	if(strcmp(rbuffer,"rrr -s A 0") == 0)
    	{
    		for (i = 0; i < 1000000; i++) 
		    {
		    	pthread_mutex_lock(&mutex);
		        balance = balance + 1;
		        pthread_mutex_unlock(&mutex);
		    }
		    printf("Balance is from Display Thread %d\n",balance);
    		printf("Execute the rrr cmd to fw board\n");
    		rbuffer[0] = '\0';
    	}
    }
}

int main(void)
{
    int i = 0;
    int err;
    
    printf("main() starts depositing, balance = %d\n", balance);
    err = pthread_create(&(tid[0]), NULL, &Led_Thread, NULL);
        if (err != 0)
            printf("\ncan't create thread :[%s]", strerror(err));
    err = pthread_setname_np(tid[0], "LED_THREAD");
           if (err != 0)
               printf("\ncan't set name to led thread :[%s]", strerror(err));
    err = pthread_create(&(tid[1]), NULL, &Deposit, NULL);
        if (err != 0)
            printf("\ncan't create thread :[%s]", strerror(err));
    err = pthread_setname_np(tid[1], "EEPROM_THREAD");
           if (err != 0)
               printf("\ncan't set name to eeprom thread :[%s]", strerror(err));
    err = pthread_create(&(tid[2]), NULL, &Display, NULL);
        if (err != 0)
            printf("\ncan't create thread :[%s]", strerror(err));
    err = pthread_setname_np(tid[2], "IOT_THREAD");
           if (err != 0)
               printf("\ncan't set name to iot thread :[%s]", strerror(err));
    //pthread_create(&tid[0], NULL, &Deposit, "A");
    pthread_join(tid[0], NULL);
    pthread_join(tid[1], NULL);
    pthread_join(tid[2], NULL);
    printf("main() A and B finished, balance = %d\n", balance); 
    
    //PicAppServer();   
    return 0;
}

/*
The first argument is a pointer to thread_id which is set by this function.
The second argument specifies attributes. If the value is NULL, then default attributes shall be used.
The third argument is name of function to be executed for the thread to be created.
The fourth argument is used to pass arguments to the function, myThreadFun.
*/