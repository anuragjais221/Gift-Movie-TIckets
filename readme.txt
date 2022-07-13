CS assignment
Anurag Jaiswal
2021CSM1002


############ Gift Movie Tickets ##########

The file  named as "2021CSM1002.c" contains the server code.
## "gcc 2021CSM1002.c -o server -pthread -lcurl" in a terminal and
then to run this file type "./server" in that terminal.
Server will start on port 7000


## For sending the email curl library implemented in c.

## Before compile this main code file kindly install libcurl using this command below -
"sudo apt-get install libcurl4-gnutls-dev"

## Request type GET with params
## Route accepted http://localhost:7000/?Guest_Email=anuragjais111@gmail.com&Guest_Name=Anurag&Ticket_Type=NORMAL&Quantity=4
## Get request with query params and can also tested via postman or curl
In another terminal use this below curl command to send a ticket
booking request to the sever-
"curl 'http://localhost:7000/?Guest_Email=anuragjais111@gmail.com&Guest_Name=Anurag&Ticket_Type=NORMAL&Quantity=4'".
Here you can change the parameter values such as Guest_Name, Guest_Email,
Quantity and Ticket_Type.

############# THANK YOU #############