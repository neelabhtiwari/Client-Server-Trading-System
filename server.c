/*
Compile Using the following command ------>
								gcc server.c
Run the created executable file using the gollowing command ------>
				./a.out <PORT NUMBER ON WHICH SERVER SHOULD LISTEN>
*/
//Assignment Group 11 (Application 2 Trading System) - Code Written for CS 349 Networks Lab - 2019 by Neelabh Tiwari (160123024), Himanshu Raj (160123049) and Uddeshya Mathur (160123048)

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include<time.h>
#include<ctype.h>
#include<unistd.h>
#include"server.h"

char **extractaction(char *buffer){                               // Any message received from server is of the form -
                                                                  // <user name/number> <password> <action (L/B/VO/VT)> <?aux. data?> #$@
  int wordcount=0;                                                // This function extracts the information from the message
  int buffsize=strlen(buffer);                                    // by using strtok(char *,char *) function to split the message
  int i=0;

  for(i=0;i<buffsize;i++){
    char check=buffer[i];                                         // find number of atomic words in the message
    if(isspace(check)) wordcount=wordcount+1;
  }

  char **ret=(char **)malloc(wordcount*sizeof(char *));           // allocate space to store the retreived message

  int wnumber=0;

  char *token=strtok(buffer," ");                                // splits the string by token " "

  while(token!=NULL){
    char *word=token;
    ret[wnumber]=(char *)malloc(strlen(word)*sizeof(char));
    strcpy(ret[wnumber],token);                                 // retreives the atomic words from message and store in an array
    token=strtok(NULL," ");
    wnumber=wnumber+1;
  }

  return ret;
}

void sendmessage(int sock,char *texttosend){                         // This message sends the message from server to client using write function

  if(write(sock,texttosend,strlen(texttosend))<0){
    FILE *fptr=fopen("error.txt","a");                              // In case of any failure in transmission, error is logged in error.txt
    if(fptr==NULL){
      printf("Error in opening the error log\n");
    }
    time_t t=time(NULL);
    struct tm *tm=localtime(&t);
    char disp[100];
    strftime(disp,sizeof(disp),"%c",tm);                            // Time stamp of the error retreived from the clock
    fprintf(fptr,"%s --- Action taken but message transmission to client failed\n",disp);
    printf("Action taken but message transmission to client failed\n");
    fclose(fptr);
  }

}

struct authorizationresult credcheck1(char *uname,char *ps){          // Authorizaton Checker - accepts input of username and password
  FILE *fptr=fopen("credentials.txt","r");                            // return a data structure which contains the authorization result and related details (refer server.h)
  if(fptr==NULL){
    printf("Error in opening the Credentials File\n");
  }
  char details[5][50];
  char buff[50];
  int i=0;
  while(fgets(buff,sizeof(buff),fptr)!=NULL){                         // usernames and passwords fetched from the database (credentials.txt)
    strcpy(details[i],buff);
    details[i][strlen(details[i])-1]='\0';
    i=i+1;
  }
  fclose(fptr);

  struct authorizationresult ret;

  for(i=0;i<5;i++){
    char *fnumber=strtok(details[i],":");                           // username and password matching from database (credentials.txt)
    char *fname=strtok(NULL,":");
    char *fpassword=strtok(NULL,"\0");
    if(strcmp(uname,fname)==0){
      if(strcmp(ps,fpassword)==0){
        strcpy(ret.name,fname);
        ret.number=atoi(fnumber); ret.result=1;                 // username and password both matched
        return ret;
      }
      else{
        strcpy(ret.name,fname);
        ret.number=atoi(fnumber); ret.result=2;                  // wrong password for the given username
        return ret;
      }
    }
  }

  strcpy(ret.name,"NO MATCH");                                        // No match for the username in the database
  ret.number=-1; ret.result=3;
  return ret;
}

struct authorizationresult credcheck2(char *unum,char *ps){         //Authorizaton Checker - accepts input of usernumber (OTP) and password
  FILE *fptr=fopen("credentials.txt","r");                          //return a data structure which contains the authorization result and related details (refer server.h)
  if(fptr==NULL){
    printf("Error in opening the Credentials File\n");
  }
  char details[5][50];
  char buff[50];
  int i=0;
  while(fgets(buff,sizeof(buff),fptr)!=NULL){               // usernames and passwords fetched from the database (credentials.txt)
    strcpy(details[i],buff);
    details[i][strlen(details[i])-1]='\0';
    i=i+1;
  }
  fclose(fptr);

  struct authorizationresult ret;

  for(i=0;i<5;i++){
    char *fnumber=strtok(details[i],":");              // username and password matching from database (credentials.txt)
    char *fname=strtok(NULL,":");
    char *fpassword=strtok(NULL,"\0");
    if(strcmp(unum,fnumber)==0){
      if(strcmp(ps,fpassword)==0){
        strcpy(ret.name,fname);
        ret.number=atoi(fnumber); ret.result=1;       // username and password both matched
        return ret;
      }
      else{
        strcpy(ret.name,fname);
        ret.number=atoi(fnumber); ret.result=2;         // wrong password for the given username
        return ret;
      }
    }
  }
  strcpy(ret.name,"NO MATCH");                      // No match for the username in the database
  ret.number=-1; ret.result=3;
  return ret;
}

void queueinsert(struct bs_request req)         // For inserting pending buy and sell request in buyqueue and sellqueue respectively in sorted manner.
{
    if(req.type=='B')           // request type is buy
    {
        int itemno=req.itemnumber;
        int *head=&(buyht[itemno][0]);          // retreives head index of buy queue
        int *tail=&(buyht[itemno][1]);          // retreives tail index of buy queue

        int i=*head;
        if(*head==*tail)            // if buy queue is empty simply insert the buy request in the buy queue
        {
            buyqueue[*head][itemno]=req;
            *tail=(*tail+1)%1000;               // increment the tail index of the buy queue
        }
        else
        {
                    // insert buy request in increasing order of money offered by buyers
                    // If two or more buyers are willing to offer same price then insert in FCFS manner

            for (i=*head ;i<*tail;i=(i+1)%1000)
            {
                if(buyqueue[i][itemno].price > req.price)      // location to insert found
                    break;
            }
            int j;
            struct bs_request forswap=req;
            for (j=i;j!=*tail;j=(j+1)%1000)
            {
                struct bs_request temp;                           // request inserted in the buy queue and other requests shifted to their new locations
                temp=buyqueue[j][itemno];
                buyqueue[j][itemno]=forswap;
                forswap=temp;
            }
            buyqueue[*tail][itemno]=forswap;
            *tail=(*tail+1)%1000;
        }
      }

    else if(req.type=='S')        // request type is sell
    {
        int itemno=req.itemnumber;
        int *head=&(sellht[itemno][0]);      // retreives head index of sell queue
        int *tail=&(sellht[itemno][1]);      // retreives tail index of sell queue

        int i=*head;
        if(*head==*tail)              //if sell queue is empty simply insert the sell request
        {
            sellqueue[*head][itemno]=req;
            *tail=(*tail+1)%1000;         // increment the tail index of the Sell queue
        }
        else
        {
                  // insert sell request in increasing order of money at which seller wants to sell
                  // If two or more seller want to sell at same price  then insert in FCFS manner

            for (i=*head ;i<*tail;i=(i+1)%1000)
            {
                if(sellqueue[i][itemno].price>req.price)        // location to insert found
                    break;
            }
            int j;
            struct bs_request forswap=req;
            for (j=i;j!=*tail;j=(j+1)%1000)
            {
                struct bs_request temp;
                temp=sellqueue[j][itemno];               // request inserted in the sell queue and other requests shifted to their new locations
                sellqueue[j][itemno]=forswap;
                forswap=temp;
            }
            sellqueue[*tail][itemno]=forswap;
            *tail=(*tail+1)%1000;
        }
    }
}

int main(int argc,char const *argv[]){

  if(argc<2 || argc>2 || (atoi(argv[1])<1024)){     // Check validity of the server start-up - Error is logged in error.txt
    printf("Please give a valid PORT NUMBER\nWhen Executing, Follow the format : \n\n./a.out <PORT NUMBER>\n\nPort Number should be a number between 1024 and 65535\n");
    FILE *fptr=fopen("error.txt","a");
    if(fptr==NULL){
      printf("Error in opening the error log\n");
    }
    time_t t=time(NULL);
    struct tm *tm=localtime(&t);
    char disp[100];
    strftime(disp,sizeof(disp),"%c",tm);
    fprintf(fptr,"%s --- Server not started properly\n",disp);
    fclose(fptr);
    return 0;
  }

  int servfd;
  if((servfd=socket(AF_INET,SOCK_STREAM,0))<0){                   // Create socket for the server - Error is logged in error.txt
    FILE *fptr=fopen("error.txt","a");
    if(fptr==NULL){
      printf("Error in opening the error log\n");
    }
    time_t t=time(NULL);
    struct tm *tm=localtime(&t);
    char disp[100];
    strftime(disp,sizeof(disp),"%c",tm);
    fprintf(fptr,"%s --- Socket Creation Failed\n",disp);
    printf("Socket Creation Failed\n");
    fclose(fptr);
    return 0;
  }

  struct sockaddr_in servaddr;
  bzero((char *)&servaddr,sizeof(servaddr));              // Clear garbage value in servaddr (Server address structure)
  int servportnumber=atoi(argv[1]);
  servaddr.sin_family=AF_INET;                        // AF_INET corresponds to IPv4
  servaddr.sin_addr.s_addr=INADDR_ANY;                // all available interfaces to the operating system
  servaddr.sin_port=htons(servportnumber);            // host-to-network short - swaps the endianness


  if(bind(servfd,(struct sockaddr *)&servaddr,sizeof(servaddr))<0){           // binds the socket to the preset address - error is logged in error.txt
      FILE *fptr=fopen("error.txt","a");
      if(fptr==NULL){
        printf("Error in opening the error log\n");
      }
      time_t t=time(NULL);
      struct tm *tm=localtime(&t);
      char disp[100];
      strftime(disp,sizeof(disp),"%c",tm);
      fprintf(fptr,"%s --- Socket Binding Failed\n",disp);
      printf("Socket Binding Failed\n");
      fclose(fptr);
      return 0;
    }

    if(listen(servfd,5)<0){                                           // listens with queue length upto 5 - error is logged in error.txt
      FILE *fptr=fopen("error.txt","a");
      if(fptr==NULL){
        printf("Error in opening the error log\n");
      }
      time_t t=time(NULL);
      struct tm *tm=localtime(&t);
      char disp[100];
      strftime(disp,sizeof(disp),"%c",tm);
      fprintf(fptr,"%s --- Socket Listening Failed\n",disp);
      printf("Socket Listening Failed\n");
      fclose(fptr);
      return 0;
    }

    int newsockfd;
    struct sockaddr_in cliaddr;                  // socket parameters for the client
    int clientaddrsize=sizeof(cliaddr);

    char primarybuffer[65536];  // main buffer used by the server


    while(1){

      newsockfd=accept(servfd,(struct sockaddr *)&cliaddr,&clientaddrsize);         // server accepts an incoming client - error is logged in error.txt

      if(newsockfd<0){
        FILE *fptr=fopen("error.txt","a");
        if(fptr==NULL){
          printf("Error in opening the error log\n");
        }
        time_t t=time(NULL);
        struct tm *tm=localtime(&t);
        char disp[100];
        strftime(disp,sizeof(disp),"%c",tm);
        fprintf(fptr,"%s --- Client Accept Failed\n",disp);
        printf("Client Accept Failed\n");
        fclose(fptr);
        continue;
      }

      char secondarybuffer[512];            // secondary buffer used by the client for each individual message
      int pbofpos=0;                        // offset for the main buffer

      while(1){                                       // message from client is stored in the 512 byte size secondary buffer

        memset(secondarybuffer,'\0',sizeof(secondarybuffer));

        int sizeread=read(newsockfd,secondarybuffer,sizeof(secondarybuffer));                 // message is read - error is logged in error.txt
        if(sizeread<0){
          FILE *fptr=fopen("error.txt","a");
          if(fptr==NULL){
            printf("Error in opening the error log\n");
          }
          time_t t=time(NULL);
          struct tm *tm=localtime(&t);
          char disp[100];
          strftime(disp,sizeof(disp),"%c",tm);
          fprintf(fptr,"%s --- Read from Client Failed\n",disp);
          printf("Read from Client Failed\n");
          fclose(fptr);
          exit(-1);
        }

        memcpy(primarybuffer+pbofpos,secondarybuffer,sizeread);                   // message is copied from secondary to primary buffer
        pbofpos=pbofpos+sizeread;

        char *delimiter="#$@";
        int isended=0;

        int pos=0;                                                       // checks if the message transmission from the client has ended
        int lendelim=strlen(delimiter);
        int length=sizeof(secondarybuffer);
        for(pos=0;pos<length-lendelim;pos++){
            if(memcmp(secondarybuffer+pos,delimiter,lendelim)==0)  {isended=1; break;}            // message ends with the delimiter "#$@"
        }
        if(isended==1) break;

      }


      primarybuffer[pbofpos]='\0';

      printf("The message received at the Server is - \n\n%s\n\n",primarybuffer);                 // displayes the message received at the server
      char **requestmessage=extractaction(primarybuffer);

      char *action=requestmessage[2];         // extracts action initiated by the client

        if(strcmp(action,"L")==0){          // client wants to login

        char *uname=requestmessage[0];    // extracts username from the message
        char *ps=requestmessage[1];       // extracts password from the message
        struct authorizationresult ret=credcheck1(uname,ps);                // authorization check
        int valid=ret.result;

        if(valid==1){                   // both username and password correct
          int num2send=ret.number;
          char num2sendstring[10];
          sprintf(num2sendstring,"%d\n\n",num2send);
          char message[]="ACCEPTED - \n\nSIGNED IN Successfully\nYour USER NUMBER IS ";
          strcat(message,num2sendstring);
          sendmessage(newsockfd,message);
        }
        else if(valid==2){        // username correct but password wrong - Unauthorized access is logged in error.txt
          char* message="REJECTED - \n\nPASSWORD WRONG\nPlease enter the correct password\n\n";
          sendmessage(newsockfd,message);
          FILE *fptr=fopen("error.txt","a");
          if(fptr==NULL){
            printf("Error in opening the error log\n");
          }
          time_t t=time(NULL);
          struct tm *tm=localtime(&t);
          char disp[100];
          strftime(disp,sizeof(disp),"%c",tm);
          fprintf(fptr,"%s --- Unauthorized access for the user - %s \n",disp,ret.name);
          fclose(fptr);
        }
        else if(valid==3){        // No match for username in the database
          char* message="REJECTED - \n\nUSERNAME WRONG\nPlease enter the correct username\n\n";
          sendmessage(newsockfd,message);
        }

      }

      else if (strcmp(action,"S")==0){ // if client wants to sell

          char *uid=requestmessage[0];        // extracts user number from the message
          char *pass=requestmessage[1];       // extracts password from the message
          char username[50];
          struct authorizationresult authcheck=credcheck2(uid,pass);    // authorization check
          int validation=authcheck.result;
          strcpy(username,authcheck.name);      // name of the user who initiated the request
          int userid=atoi(uid);

          if(validation!=1){                          //checks the validation of  username and password
              sendmessage(newsockfd,"REJECTED \n\nLOG IN Unsuccessful\n\n");
              close(newsockfd);
              continue;
          }
          int item = atoi( requestmessage[3] );
          int qty =  atoi( requestmessage[4] );                       // parameters of the request
          int unitprice = atoi( requestmessage[5] );

          struct bs_request sell;
          strcpy(sell.user,username);
          sell.itemnumber = item;
          sell.qty = qty;
          sell.price = unitprice;
          sell.id = userid;
          sell.type = 'S';

          int i;
          while(sell.qty>0)
          {
                  // searches ,if there is any pending buy request  .
                  // searches maximum price at which buyer  wants to buy.
                  // checks ,if that price is more than the price at which seller wants to sell

              if(buyht[item][0]==buyht[item][1])        // if buy queue is empty, then insert the sell request in the sell queue
              {
                  queueinsert(sell);
                  break;
              }
              int bestsell=buyht[item][0];
              for(i=buyht[item][0]+1;i!=buyht[item][1]; i = (i+1)%1000)                     // buy queue not empty
              {
                  if( (buyqueue[i][item].price>buyqueue[bestsell][item].price) )            // finds maximum price at which any trader wants to buy the requested item
                      bestsell=i;
              }
              if(buyqueue[bestsell][item].price>=sell.price)         // if any trader is capable of matching with the request
              {
                  if( buyqueue[bestsell][item].qty > sell.qty )         // if whole requested quantitiy can be accommodated
                  {
                          buyqueue[bestsell][item].qty -= sell.qty;
                          struct logdetail temp;
                          strcpy(temp.seller, username);
                          strcpy(temp.buyer, buyqueue[bestsell][item].user);
                          temp.itemnumber = item;
                          temp.price = buyqueue[bestsell][item].price;              // Trade log updated by inserting the details of this matched trades
                          temp.qty = sell.qty;
                          temp.buyerid = buyqueue[bestsell][item].id;
                          temp.sellerid = sell.id;
                          tradelog[tradeno++] = temp;                                   // Trade Number incremented by 1

                          sell.qty = 0;
                          break;
                  }
                  else
                  {
                      sell.qty -=buyqueue[bestsell][item].qty;                            // whole requested quantitiy not available

                      struct logdetail temp;
                      strcpy(temp.seller, username);
                      strcpy(temp.buyer, buyqueue[bestsell][item].user);
                      temp.itemnumber = item;
                      temp.price =buyqueue[bestsell][item].price;                   // Trade log updated by inserting the details of this matched trades
                      temp.qty =  buyqueue[bestsell][item].qty;
                      temp.buyerid = buyqueue[bestsell][item].id;
                      temp.sellerid = sell.id;
                      tradelog[tradeno++] = temp;         // Trade Number incremented by 1

                      for(i=bestsell;i!=buyht[item][1];i=(i+1)%1000)
                          buyqueue[i][item]=buyqueue[(i+1)%1000][item];
                      buyht[item][1]=(1000+buyht[item][1]-1)%1000;
                  }
              }
              else
              {
                                                  //inserts left sell request in sell queue
                  queueinsert(sell);
                  break;
              }
          }
          sendmessage(newsockfd,"ACCEPTED\n\n");
      }

      else if(strcmp(action,"B")==0){   //if client wants to buy

        char *uid=requestmessage[0];    // extracts user number from the message
        char *pass=requestmessage[1];   // extracts password from the message
        char username[50];
        struct authorizationresult authcheck=credcheck2(uid,pass);    // authorization check
        int validation=authcheck.result;
        strcpy(username,authcheck.name);      // name of the user who initiated the request
        int userid=atoi(uid);

        if(validation!=1){            //checks the validation of  username and password
            sendmessage(newsockfd,"REJECTED \n\nLOG IN Unsuccessful\n\n");
            close(newsockfd);
            continue;
        }
        int item = atoi( requestmessage[3] );
        int qty =  atoi( requestmessage[4] );         // parameters of the request
        int unitprice = atoi( requestmessage[5] );

        struct bs_request buy;
        strcpy(buy.user,username);
        buy.itemnumber = item;
        buy.qty = qty;
        buy.price = unitprice;
        buy.id = userid;
        buy.type = 'B';

        int check=0,i;
        for (i=sellht[item][0];i!=sellht[item][1]; i=(i+1)%1000)
        {
                  // searches ,if there is any pending sell request  .
                  // searches minimum price at which seller wants to sell .
                  // checks ,if that price is in budget of buyer

            if(sellqueue[i][item].price<=unitprice) // if any trader is capable of matching with the request
            {
                if(sellqueue[i][item].qty>=buy.qty) // if whole requested quantitiy can be accommodated
                {
                    check=1;
                    sellqueue[i][item].qty-=buy.qty;
                    if(sellqueue[i][item].qty==0)
                        sellht[item][0]=(sellht[item][0]+1)%1000;

                    struct logdetail temp;
                    strcpy(temp.buyer,username );
                    strcpy(temp.seller, sellqueue[i][item].user);
                    temp.itemnumber = item;
                    temp.price = sellqueue[i][item].price;
                    temp.qty = buy.qty;
                    temp.buyerid = buy.id;
                    temp.sellerid = sellqueue[i][item].id;   // Trade log updated by inserting the details of this matched trades
                    tradelog[tradeno++] = temp;   // Trade Number incremented by 1


                    break;
                }
                else
                {
                    sellht[item][0]=(sellht[item][0]+1)%1000;
                    buy.qty-=sellqueue[i][item].qty;
                    struct logdetail temp;
                    strcpy(temp.buyer, username);
                    strcpy(temp.seller, sellqueue[i][item].user);
                    temp.itemnumber = item;                           // Trade log updated by inserting the details of this matched trades
                    temp.price = sellqueue[i][item].price;
                    temp.qty = sellqueue[i][item].qty;
                    temp.buyerid = buy.id;
                    temp.sellerid = sellqueue[i][item].id;
                    tradelog[tradeno++] = temp;     // Trade Number incremented by 1
                }

            }
        }
        if(check==0)        //inserts left buy request in the buy queue
            queueinsert(buy);
        sendmessage(newsockfd,"ACCEPTED\n\n");

    }

    else if(strcmp(action,"VO")==0){ //if client wants to view orders

        int i,j;
        char ret[1000] = "\0";
        for(i=0; i<10; i++)
        {
                sprintf(ret+strlen(ret), "\nItem no. : %d\n", i);
                strcpy(ret+strlen(ret), "   best sell(Least price): ");
                if( sellht[i][0] != sellht[i][1] )  // checks if sell queue is empty ?
                {
                    //sends best sell of item 'i'
                    sprintf(ret+strlen(ret), "quantity- %d",sellqueue[sellht[i][0]][i].qty );
                    strcpy(ret+strlen(ret), ", ");
                    sprintf(ret+strlen(ret), "price- %d", sellqueue[sellht[i][0]][i].price);
                }
                else
                    strcpy(ret+strlen(ret), "No sell available for this item "); // prints this if sell queue is empty
                strcpy(ret+strlen(ret), "\n");

                strcpy(ret+strlen(ret), "   best Buy(Max price): ");
                if( buyht[i][0] != buyht[i][1] )    //checks if buy queue is empty
                {
                    //sends best buy of item 'i'
                  int maxprice= buyqueue[(1000+buyht[i][1]-1)%1000][i].price;
                  int maxprice_quantity=buyqueue[(1000+buyht[i][1]-1)%1000][i].qty;
                  for (j=buyht[i][0] ;j!=buyht[i][1];j=(j+1)%1000)
                  {
                      if(maxprice==buyqueue[j][i].price)
                      {
                        maxprice_quantity=buyqueue[j][i].qty;
                        break;
                      }
                  }
                    sprintf(ret+strlen(ret), "quantity- %d",maxprice_quantity);
                    strcpy(ret+strlen(ret), ", ");
                    sprintf(ret+strlen(ret), "price- %d", buyqueue[(1000+buyht[i][1]-1)%1000][i].price);
                }
                else
                    strcpy(ret+strlen(ret), "No buy available for this item"); // prints this if buy queue is empty
                strcpy(ret+strlen(ret), "\n");
            }
            sendmessage(newsockfd,"ACCEPTED\n\n");
            sendmessage(newsockfd, ret);

    }

    else if(strcmp(action,"VT")==0){  // if client wants to see his/her trade history
      char *uid=requestmessage[0];
      char *pass=requestmessage[1];
      char username[50];
      struct authorizationresult authcheck=credcheck2(uid,pass);
      int validation=authcheck.result;
      strcpy(username,authcheck.name);

      if(validation!=1){  //checks the validation of  username and password
          sendmessage(newsockfd,"REJECTED \n\nLOG IN Unsuccessful\n\n");
          close(newsockfd);
          continue;
      }
        int userid=atoi(uid);
        char ret[1000];
        sendmessage(newsockfd,"ACCEPTED\n\n");
        int i,j;

        sprintf(ret, "BUYER      SELLER      ITEM    QUANTITY  PRICE  BUYER_ID  SELLER_ID\n\n");
        sendmessage(newsockfd, ret);

        for (i=0;i<tradeno;i++) // prints details of matched orders like buyer name ,seller name ,item number,qunatity ,price .
        {
            if(tradelog[i].buyerid==userid)
            {
                sprintf(ret, "%s      %s   %d       %d         %d      %d         %d\n", tradelog[i].buyer, tradelog[i].seller, tradelog[i].itemnumber, tradelog[i].qty, tradelog[i].price,tradelog[i].buyerid,tradelog[i].sellerid);
                sendmessage(newsockfd, ret);
            }
            else if(tradelog[i].sellerid==userid)
            {
                sprintf(ret, "%s      %s   %d       %d         %d      %d         %d\n", tradelog[i].buyer, tradelog[i].seller, tradelog[i].itemnumber, tradelog[i].qty, tradelog[i].price,tradelog[i].buyerid,tradelog[i].sellerid);
                sendmessage(newsockfd, ret);
            }
        }
    }
    close(newsockfd);

   }
   return 0;
}
