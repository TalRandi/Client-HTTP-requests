
Login name: talrn

Name & ID: Tal Randi - 315633503

Exercise Name: EX2 – Client HTTP requests

Files:

Client.c 	   :	 This c file implements the client-server connection, it translate an HTTP request 
			 from the user to actual HTTP request. sent the request to the server, and show server's response.
README 		   :	 This file

Remarks: 

This program implements an HTTP request by getting a request from user, in some format - and converts it to actual HTTP 
request so the server can read it and do action after reading.
The format the program gets it's input - 

1)http://----some site address----:--port number(optional)--/---some page inside the site---- 
2)-p ---some text----
3)-r n value=args value=args ... n times

The order does not have to be as described. It does not matter.
In case of port number not delivered to the program, the program will sent the request to port 80 by defult. 
In case of page not delivered to thte program, the program will sent the request to the main page of site.
By -p option - user can send a POST request and sent one body message.
By -r option - user can send to the server n values and arguments.
In any case of user insert wrong request format - the program show to user the correct format.

there are two private function:

UsageError = gets the all buffers, free their memory. this function also print an error message by the relevant case.
digitsNumber = returns the number of digits of an integer.




