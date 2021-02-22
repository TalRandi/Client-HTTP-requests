/*=====================================================================================================================================*/
/*=====================================================================================================================================*/
/*=======================================--------------------------------------------------------======================================*/
/*==================================---------------------------Tal Randi------------------------------=================================*/
/*==================================------------------------------Ex2--------------------------------==================================*/
/*=======================================--------------------------------------------------------======================================*/
/*=====================================================================================================================================*/
/*=====================================================================================================================================*/


#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h>
#include <netdb.h>
#include <stdlib.h>
#include <stddef.h>
#include <sys/types.h>     	          /* standard system types        */
#include <netinet/in.h>    	          /* Internet address structures */

#define BUFFSIZE 100
#define NO_ERR 0
#define USAGE_ERR 1
#define SOCKET_ERR 3
#define HOST_ERR 4
#define CONNECT_ERR 5
#define READ_ERR 6
#define MALLOC_ERR 10

//This function gets all buffers and it's length or flags. free the allocate memory, print Usage error and exit the program
void UsageError(int hostLen, int portLen, int reqLen, int postFlag, int parFlag, char* host, char* page, char* port, char* request, char* post, char* temp, char* valAnArgs,int errFlag)
{
    if(hostLen != 0)
    {
        free(page);
        free(host);
    }
    if(portLen != 0)
        free(port);
    if(reqLen != 0)
        free(request);
    if(postFlag == 1)
    {
        free(post);
        free(temp);
    }
    if(parFlag == 1)
        free(valAnArgs);  

    if(errFlag == NO_ERR)
        return;

    if(errFlag == USAGE_ERR)
        fprintf(stderr,"Usage: client [-p <text>] [-r n <pr1=value1 pr2=value2 …>] <URL>\n");

    else if(errFlag == SOCKET_ERR)
        perror("socket failed\n");

    else if(errFlag == HOST_ERR)
        herror("No such host\n");

    else if(errFlag == CONNECT_ERR)
        perror("Connect failed\n");

    else if(errFlag == READ_ERR)
        perror("read() failed\n") ;

    else if(errFlag == MALLOC_ERR)
        perror("Malloc failed\n");

    exit(1);
}
//This function gets a number and return how many digit are exists
int digitsNumber(int number)
{
    int cntr = 0;
    while(number > 0)
    {
        cntr++;
        number = number/10;
    }
    return cntr;
}
//This function get a string and return 0 if it include only digits, -1 in any other case
int isPositiveNumber(char* number)
{
    for(int i = 0 ; i < strlen(number) ; i++)
        if(number[i] < '0' || number[i] > '9')
            return -1;
    return 0;
}
/****************************************************************************/

int main(int argc, char* argv[]){

    //Invalid execution
    if(argc < 2)
    {
        fprintf(stderr, "Usage: client [-p <text>] [-r n <pr1=value1 pr2=value2 …>] <URL>\n");
        exit(1);
    }


    char buff[BUFFSIZE];
    int rc = 0;
    int socket_fd = 0;
    struct hostent* server;
    struct sockaddr_in server_address;
    char* request = 0;
    int request_length = 0;
    int j = 0;
    char* url_pointer = 0;
    char* port_pointer = 0;
    char* port = 0;
    int port_length = 0;
    char* host = 0; 
    int host_length = 0;
    char* page = 0;
    int page_length;
    int counter = 0;
    int post_flag = 0;
    int parameter_flag = 0;
    int parameters_number = 0;
    char* post_param = 0;
    int content_length = 0;
    char* values_and_args = 0;;
    int val_and_args_length = 0;
    char* temp = 0;
    int response_size = 0;

    
    //Pass over argv array
    for(int i = 1 ; i < argc ; i++)
    {
        //POST request
        if(strcmp(argv[i],"-p") == 0 && post_flag == 0)
        {
            post_flag = 1;
            if(argv[i+1])
            {
                content_length = strlen(argv[i+1]);
                post_param = (char*)malloc(sizeof(char)*(content_length + 1));
                if(!post_param)
                    UsageError(host_length,port_length,request_length,post_flag,parameter_flag,host,page,port,request,post_param,temp,values_and_args,MALLOC_ERR);

                strncpy(post_param,argv[i+1],content_length);
                post_param[content_length] = '\0';
                i++;
            }
            else
                UsageError(host_length,port_length,request_length,post_flag,parameter_flag,host,page,port,request,post_param,temp,values_and_args,USAGE_ERR);
            
        }
        //Parameters exists in request
        else if(strcmp(argv[i],"-r") == 0 && parameter_flag == 0)
        {
            parameter_flag = 1;
            if(argv[i+1])
            {
                j = i+2; //First value
                if(isPositiveNumber(argv[i+1]) == -1)
                    UsageError(host_length,port_length,request_length,post_flag,parameter_flag,host,page,port,request,post_param,temp,values_and_args,USAGE_ERR);

                parameters_number = atoi(argv[i+1]);
                if(parameters_number < 0)
                    UsageError(host_length,port_length,request_length,post_flag,parameter_flag,host,page,port,request,post_param,temp,values_and_args,USAGE_ERR);


                //Count the length of arguments buffer
                while(j < (parameters_number + i + 2))
                {
                    if(!argv[j])
                        UsageError(host_length,port_length,request_length,post_flag,parameter_flag,host,page,port,request,post_param,temp,values_and_args,USAGE_ERR);
                    
                    if(!strstr(argv[j],"=") || (strstr(argv[j],"=") && (argv[j][0] == '=' || argv[j][strlen(argv[j]) - 1] == '=')))
                        UsageError(host_length,port_length,request_length,post_flag,parameter_flag,host,page,port,request,post_param,temp,values_and_args,USAGE_ERR);     
            
                    val_and_args_length += strlen(argv[j]) + 1;
                    j++;
                }
                //Allocate the arguments and values buffer
                if(val_and_args_length > 0)
                {
                    values_and_args = (char*)malloc(sizeof(char)*val_and_args_length + 1);
                    if(!values_and_args)
                        UsageError(host_length,port_length,request_length,post_flag,parameter_flag,host,page,port,request,post_param,temp,values_and_args,MALLOC_ERR);

                    strcpy(values_and_args,"?");
                    j = i+2;
                    //Check validation of each parameter and add it to values and args array
                    while(j < (parameters_number + i + 2))
                    {
                        if(!strstr(argv[j],"="))
                            UsageError(host_length,port_length,request_length,post_flag,parameter_flag,host,page,port,request,post_param,temp,values_and_args,USAGE_ERR);
                    
                        strcat(values_and_args,argv[j]);    
                        if(j < parameters_number + i + 1)
                            strcat(values_and_args,"&");
                        j++;
                    }
                    //Set 0 in array's end
                    values_and_args[val_and_args_length] = '\0';
                    if(argv[j])
                    {
                        if(strstr(argv[j],"="))
                            UsageError(host_length,port_length,request_length,post_flag,parameter_flag,host,page,port,request,post_param,temp,values_and_args,USAGE_ERR);                     
                    }
                    i = j - 1;
                }
                else
                {
                    i = j - 1;
                }
            }
            else
                UsageError(host_length,port_length,request_length,post_flag,parameter_flag,host,page,port,request,post_param,temp,values_and_args,USAGE_ERR);
        }
        //argv[i] is optional contains the url
        else if(strstr(argv[i],"http://") || strstr(argv[i],"HTTP://"))
        {
            if(strstr(argv[i],"http://"))
                url_pointer = strstr(argv[i],"http://") + 7; //Move to the char next to the http:// statemant
            else
                url_pointer = strstr(argv[i],"HTTP://") + 7;
                
            if(!url_pointer)
                UsageError(host_length,port_length,request_length,post_flag,parameter_flag,host,page,port,request,post_param,temp,values_and_args,USAGE_ERR);
            
            port_pointer = strchr(url_pointer, ':');
            //Default port 80
            if(!port_pointer)
            {
                port_length = 3;
                port = (char*)malloc(sizeof(char)*port_length);
                if(!port)
                    UsageError(host_length,port_length,request_length,post_flag,parameter_flag,host,page,port,request,post_param,temp,values_and_args,MALLOC_ERR);

                strncpy(port,"80\0",3);
            }
            //Other port
            else 
            {
                counter = 0;
                j = (int)(port_pointer - url_pointer);
                port_pointer++;
                while(port_pointer[counter] != '/' && j < strlen(argv[i]))
                {
                    counter++;
                    j++;
                }
                //Case of : and no port number 
                if(counter == 0)
                    UsageError(host_length,port_length,request_length,post_flag,parameter_flag,host,page,port,request,post_param,temp,values_and_args,USAGE_ERR);

                port_length = counter + 1;
                port = (char*)malloc(sizeof(char)*port_length);
                if(!port)
                    UsageError(host_length,port_length,request_length,post_flag,parameter_flag,host,page,port,request,post_param,temp,values_and_args,MALLOC_ERR);

                strncpy(port,port_pointer,counter);
                port[counter] = '\0';
                if(isPositiveNumber(port) == -1 || atoi(port) > 65535)
                    UsageError(host_length,port_length,request_length,post_flag,parameter_flag,host,page,port,request,post_param,temp,values_and_args,USAGE_ERR);
            }
            counter = 0;
            j = 0;
            //Build the host address
            while(url_pointer[j] != '/' && url_pointer[j] != ':')
            {
                counter++;
                j++;
            }
            host_length = counter + 1;
            host = (char*)malloc(sizeof(char)*host_length);
            if(!host)
                UsageError(host_length,port_length,request_length,post_flag,parameter_flag,host,page,port,request,post_param,temp,values_and_args,MALLOC_ERR);

            strncpy(host,url_pointer,counter);
            host[counter] = '\0';

            //Build the page address
            url_pointer = strstr(url_pointer, "/");
            if(!url_pointer)
            {
                page_length = 2; 
                page = (char*)malloc(sizeof(char) + 1); 
                if(!page)
                    UsageError(host_length,port_length,request_length,post_flag,parameter_flag,host,page,port,request,post_param,temp,values_and_args,MALLOC_ERR);

                page[0] = '/';
                page[1] = '\0';                
            }
            //Page is exist
            else
            {
                counter = 0;
                j = 0;
                while(url_pointer[j] != 0)
                {
                    counter++;
                    j++;
                }
                page_length = counter + 1;
                page = (char*)malloc(sizeof(char)*page_length);
                if(!page)
                    UsageError(host_length,port_length,request_length,post_flag,parameter_flag,host,page,port,request,post_param,temp,values_and_args,MALLOC_ERR);

                strncpy(page,url_pointer,counter);    
                page[counter] = '\0';                 
            }
        }
        else
            UsageError(host_length,port_length,request_length,post_flag,parameter_flag,host,page,port,request,post_param,temp,values_and_args,USAGE_ERR);
    }
    if(!host)
        UsageError(host_length,port_length,request_length,post_flag,parameter_flag,host,page,port,request,post_param,temp,values_and_args,USAGE_ERR);
    

    /********************** Build connection with the server **********************/
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_fd < 0)
        UsageError(host_length,port_length,request_length,post_flag,parameter_flag,host,page,port,request,post_param,temp,values_and_args,SOCKET_ERR);
    
    //Cast the host name to ip address
    server = gethostbyname(host);
    if(!server)
        UsageError(host_length,port_length,request_length,post_flag,parameter_flag,host,page,port,request,post_param,temp,values_and_args,HOST_ERR);
    
    server_address.sin_family = AF_INET;
    bcopy((char*)server->h_addr, (char*)&server_address.sin_addr.s_addr, server->h_length);
    server_address.sin_port = htons(atoi(port));
    
    rc = connect(socket_fd, (const struct sockaddr*)&server_address, sizeof(server_address));
    //Connection to the server has been failed
    if(rc < 0)
        UsageError(host_length,port_length,request_length,post_flag,parameter_flag,host,page,port,request,post_param,temp,values_and_args,CONNECT_ERR);

    /********************** Connection has been established **********************/
    request_length = page_length + port_length + host_length + val_and_args_length + 42;
    if(post_param)
        request_length += content_length;

    //Build request
    if(post_flag == 0)//GET request
    {
        request_length += 4; // Add the size of GET request
        request = (char*)malloc(sizeof(char)*request_length);
        if(!request)
            UsageError(host_length,port_length,request_length,post_flag,parameter_flag,host,page,port,request,post_param,temp,values_and_args,MALLOC_ERR);

        strcpy(request,"GET ");
    }
    else // POST request
    {
        request_length += 5;  // Add the size of POST request
        request = (char*)malloc(sizeof(char)*request_length);
        if(!request)
            UsageError(host_length,port_length,request_length,post_flag,parameter_flag,host,page,port,request,post_param,temp,values_and_args,MALLOC_ERR);

        strcpy(request,"POST ");
    }
    strcat(request,page);  
    if(val_and_args_length > 0)
        strcat(request,values_and_args);  

    strcat(request," HTTP/1.0\r\nHost: ");
    strcat(request,host);
    //POST request
    if(post_flag == 1)
    {
        temp = (char*)malloc(sizeof(char)*(digitsNumber(content_length)) + 1);
        if(!temp)
            UsageError(host_length,port_length,request_length,post_flag,parameter_flag,host,page,port,request,post_param,temp,values_and_args,MALLOC_ERR);

        sprintf(temp,"%d",content_length);
        temp[digitsNumber(content_length)] = '\0';
        strcat(request,"\r\n");
        strcat(request,"Content-length:");
        strcat(request,temp);
        strcat(request,"\r\n\r\n");
        strcat(request,post_param);
    }
    else
        strcat(request,"\r\n\r\n");

    request[request_length - 1] = '\0';
    printf("HTTP request =\n%s\nLEN = %ld\n", request, strlen(request));
    
    //Send a request to the server
    write(socket_fd, request, strlen(request) + 1);


    //Prints the server's response 
    do{
        rc = read(socket_fd, buff, BUFFSIZE);
        response_size += rc;
        buff[rc] = '\0';
        if(rc > 0)
	        printf("%s", buff);
        
        else if(rc < 0)
            UsageError(host_length,port_length,request_length,post_flag,parameter_flag,host,page,port,request,post_param,temp,values_and_args,READ_ERR);

    }while(rc > 0);
    printf("\n Total received response bytes: %d\n",response_size); 

    //Close connection with the server
    close(socket_fd);
        UsageError(host_length,port_length,request_length,post_flag,parameter_flag,host,page,port,request,post_param,temp,values_and_args,NO_ERR);

    return 0;
}
