Assignment Group 11 (Application 2 Trading System) - Code Written for CS 349 Networks Lab - 2019 by Neelabh Tiwari (160123024), Himanshu Raj (160123049) and Uddeshya Mathur (160123048)

Protocols for message transfer in trading system  ------

Client Messages (Requests) -------------------------------------------------------------------------------------------------------------------

  - Credentials are sent with every request to verify the identity of trder initiating a trade
  - This is analogous to OTP used in real life transactions
  - Information in request messages are seperated by space character
  - Messages are terminated by special character sequence "#$@"

Request Types -------------------------------------

 1. Login request
      <Log-IN ID> <PASSWORD> L #$@

 2. Buy request
      <Log-IN Number> <PASSWORD> B <item-code> <qty> <unit-price> #$@

 3. Sell request
      <Log-IN Number> <PASSWORD> S <item-code> <qty> <unit-price> #$@

 4. View orders
      <Log-IN Number> <PASSWORD> VO #$@

 5. View Trades
      <Log-IN Number> <PASSWORD> VT #$@


Server Messages (Responses) -------------------------------------------------------------------------------------------------------------------

- First line of the message is status line (ACCEPTED or REJECTED)
    - ACCEPTED : action initiated by client is implemented Successfully by server
    - REJECTED : action refused by the server (Eg. Sign IN denied due to wrong password)

- Second line onwards contain the message intended for the client - It is displayed as is to the client

- Common response format
    <status line>\n
    <content>

- Example response for sign in request:
    ACCEPTED -
    SIGNED IN Successfully
    Your USER NUMBER IS <1/2/3/4/5>

- Once successful log in is done by a client, he is allocated a user number which acts as an one time password (OTP) for the duration of the current session


Run Server 
-------------------------------------------------------------------------------------------------------------------

gcc server.c
./a.out 1234

Run Clients
-------------------------------------------------------------------------------------------------------------------

gcc -Wall -o client client.c
./client 127.0.0.1 1234

