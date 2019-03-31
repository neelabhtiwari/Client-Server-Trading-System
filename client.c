/*
Compile Using the following command ------>
								gcc -Wall -o client client.c
Run the created executable file using the gollowing command ------>
				./client <SERVER IP ADDRESS> <PORT NUMBER ON WHICH SERVER IS LISTENING>
*/
//Assignment Group 11 (Application 2 Trading System) - Code Written for CS 349 Networks Lab - 2019 by Neelabh Tiwari (160123024), Himanshu Raj (160123049) and Uddeshya Mathur (160123048)

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<arpa/inet.h>
#include<sys/socket.h>

static char id_trader[100];										// holds the trader id
static char pass[100];												// holds the password of the user for entire session
static struct sockaddr_in S;									// use of the predefined structure named sockaddr_in tol hold details of the socket
static int trader_number;											// holds the trader number used for the session (OTP - one time password)


void send_request(const char * request, char * response)			// function to send request over the socket to the server
{
	int s=socket(AF_INET,SOCK_STREAM,0);												//  creation of the socket.

	if(s==-1)
	{
		printf("Socket not created\n");
		exit(-1);
	}

	int t;
	t= connect(s,(struct sockaddr*) &S, sizeof(S));					// connection request to the server
	if(t<0)
	{
		printf("Connection to server could not be established\n");
		exit(-1);
	}

	t= send(s,request,strlen(request),0);							// sending of the request to the server
	if(t<0)
	{
		printf("Request could not be sent\n");
		printf("Please send again\n");
		exit(-1);
	}

	char * buffer[1024];
	int offset=0;
	while(1)														// storing of response from server to client into a buffer
	{
		int tmp=recv(s,buffer,1024,0);
		if(tmp>0)
		{
			memcpy(response+offset,buffer,tmp);
			offset+=tmp;
		}
		else
		{
			break;
		}
	}

	response[offset]='\0';

}

int successful(char * response)										// checks if the sent request was succesful or not
{
	char tmp[8];
	strncpy(tmp,response,8);

	if(strcmp(tmp,"ACCEPTED")==0)									// each response if successful contains the word ACCEPTED in the first line
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int check_login (char * response)									// authenticates the user
{
	char request[65536];
	memset(request,'\0',sizeof(request));

	strcat(request,id_trader); strcat(request," ");
	strcat(request,pass); strcat(request," ");
	strcat(request,"L");
	strcat(request," #$@");											// these are the ending character markers for each request sent to mark the end of request
	send_request(request,response);

	if(successful(response))
	{
		return 1;
	}
	else
	{
		return 0;
	}

}


int buy_sell(int item_number, int quantity, int unit_price, char type) 		// creates the request to send over to the server based on what the client wants to do
{
	char request[65536];
	char response[65536];
	memset(response,'\0',sizeof(response));
	memset(request,'\0',sizeof(request));

	char tmp[32];
	sprintf(tmp,"%d",trader_number);
	strcat(request,tmp); strcat(request," ");
	strcat(request,pass); strcat(request," ");

	if(type=='b')															// indicates that the request is a buy request
	{
		strcat(request,"B"); strcat(request," ");
	}
	else 																	// // indicates that the request is a sell request
	{
		strcat(request,"S"); strcat(request," ");
	}

	sprintf(tmp,"%d",item_number);
	strcat(request,tmp); strcat(request," ");								// creation  of reauest by appending to the string named request using strcat function
	sprintf(tmp,"%d",quantity);
	strcat(request,tmp); strcat(request," ");
	sprintf(tmp,"%d",unit_price);
	strcat(request,tmp); strcat(request," #$@");

	send_request(request,response);

	if(successful(response))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void view ( char type )											// creates a request to send over to the server to view the trades by the user or the orders available
{
	char request[65536];
	char response[65536];
	memset(request,'\0',sizeof(request));
	char tmp[32];
	sprintf(tmp,"%d",trader_number);
	strcat(request,tmp); strcat(request," ");
	strcat(request,pass); strcat(request," ");

	if(type=='o')												// indicates that the request is a view order request
	{
		strcat(request,"VO #$@");
	}
	else 														// indicates that the request is a view trade request
	{
		strcat(request,"VT #$@");
	}
	send_request(request,response);

	if(!successful(response))
	{
		printf("Request Failed\nTry again\n");
		return;
	}

	char * check;
	check=response;
	while(*check != '\n')
	{
		check=check+1;
	}
	check++;
	printf("%s",check);
}

void view_menu()
{
	printf("\n\nWhat would you like to do:\n");
	printf("1: Enter 1 to view orders\n");
	printf("2: Enter 2 to view your trades\n");
	printf("3: Enter 3 to send a buy request\n");
	printf("4: Enter 4 to send a sell request\n");
	printf("5: Enter 5 to exit the system\n\n");
}

int main(int argc , char const *argv[])
{
	S.sin_addr.s_addr = inet_addr(argv[1]);											// takes the first command line argument as the server address
	S.sin_family = AF_INET;                                                         // To specify that IPv4 is used for the network
	S.sin_port = htons(atoi(argv[2]));												// takes the second command line argument as the port number and converts it to internet readable form

	printf("\n\n****************************************************************\n\n");
	printf("  ****************Welcome to the Trading System****************\n\n");

	while(1)																		// log in process
	{
		char response[65536];
		memset(response,'\0',sizeof(response));
		printf("Enter your id:\n");
		scanf("%s",id_trader);
		printf("Please enter your password %s\n",id_trader);
		scanf("%s",pass);


		char * check;
		if(check_login(response))
		{
			check=response;
			while(*check != '\n')
			{
				check=check+1;
			}
			check++;
			printf("%s",check);
			printf("Enter your trader number as shown by the trading system\n");
			scanf("%d",&trader_number);
			break;
		}
		else
		{
			check=response;
			while(*check != '\n')
			{
				check=check+1;
			}
			check++;
			printf("%s",check);
		}
	}

	int opt;
	int item_number; int quantity; int unit_price;

	while(1)																// services offered by the trading system
	{
		view_menu();

		scanf("%d",&opt);

		switch(opt)
		{
			case 1:
			{
				view('o');
				break;
			}
			case 2:
			{
				view('t');
				break;
			}
			case 3:
			{
				printf("Enter the item number of the item you want to buy:\n");
				scanf("%d",&item_number);
				printf("Enter quantity you want to buy:\n");
				scanf("%d",&quantity);
				printf("Enter the unit price you are willing to offer:\n");
				scanf("%d",&unit_price);
				buy_sell(item_number,quantity,unit_price,'b');
				break;
			}
			case 4:
			{
				printf("Enter the item number of the item you want to sell:\n");
				scanf("%d",&item_number);
				printf("Enter quantity you want to sell:\n");
				scanf("%d",&quantity);
				printf("Enter the unit price you are willing to sell at:\n");
				scanf("%d",&unit_price);
				buy_sell(item_number,quantity,unit_price,'s');
				break;
			}
			case 5:
			{
				exit(-1);
			}
			default:
			{
				printf("Invalid command. Choose again\n");
				view_menu();
				break;
			}
		}
	}

	return 0;
}
