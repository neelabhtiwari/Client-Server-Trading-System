//Assignment Group 11 (Application 2 Trading System) - Code Written for CS 349 Networks Lab - 2019 by Neelabh Tiwari (160123024), Himanshu Raj (160123049) and Uddeshya Mathur (160123048)

struct authorizationresult{  // This Data Structure returns to the server the result of
  char name[50];             // login attempts made by any user
  int number;                // result = 1 - login id and password matched
  int result;                // result = 2 - login id correct but password incorrect
};                           // result = 3 - login id did not match to any username

struct logdetail{         // contains trade details
    char buyer[50];       // contains buyer name
    char seller[50];      // contains seller name
    int buyerid;          // buyer id
    int sellerid;         // seller id
    int itemnumber;       // item number
    int price;            // trade price of item
    int qty;              // quantity of item traded
};

struct bs_request{     // structure for buy queue and sell queue
    char user[50];     // buyer or seller name
    int itemnumber;    // item number
    int id;            // id of buyer or seller who initiated the request
    int qty;           // quantity to buy or sell
    int price;         // trade price of the item
    char type;         // 'B' for Buy ans 'S' for Sell
};

struct logdetail tradelog[1000];  // contains all the matched trades

int tradeno = 0;    // contains total  number of matched trades

struct bs_request buyqueue[1000][10];   // buy queue at the trade exchange
struct bs_request sellqueue[1000][10];  //sell queue at the trade exchange

int buyht[10][2]={0};    //Head and Tail index of Buy Queue for each item
int sellht[10][2]={0};   //Head and Tail index of Sell Queue for each item
