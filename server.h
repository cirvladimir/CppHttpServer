/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <string>


#define HTTP_SERVER_BUFFER_SIZE 1024

class HTTPRequest 
{
public:
    std::string path;
    std::string method;
    std::string body;
};

class HTTPResponse 
{
public:
    int code;
    std::string body;
    std::string contentType;
};

class HTTPRequestHandler 
{
public:
    virtual HTTPResponse handleRequest(HTTPRequest request) = 0;
};

class HTTPServer 
{
private:
    void error(const char *msg)
    {
	perror(msg);
	exit(1);
    }
    HTTPRequestHandler * handler;
    int portno;
    bool started;
    int sockfd;
public:
    HTTPServer(HTTPRequestHandler * hndl, int port) 
    {
	handler = hndl;
	portno = port;
	started = false;
    }
    void start()
    {
	started = true;
	int newsockfd;
	socklen_t clilen;
	char buffer[HTTP_SERVER_BUFFER_SIZE];
	struct sockaddr_in serv_addr, cli_addr;
	int n;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
	    error("ERROR opening socket");
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if (bind(sockfd, (struct sockaddr *) &serv_addr,
		sizeof(serv_addr)) < 0) 
	    error("ERROR on binding");
	listen(sockfd,5);
	clilen = sizeof(cli_addr);
	while (started)
	{
	    newsockfd = accept(sockfd, 
			(struct sockaddr *) &cli_addr, 
			&clilen);
	    if (newsockfd < 0) 
		error("ERROR on accept");
	    n = read(newsockfd,buffer,HTTP_SERVER_BUFFER_SIZE);
	    if (n < 0) error("ERROR reading from socket");
	    HTTPRequest request;
	    request.method = "";
	    int ind = 0;
	    while (buffer[ind] != ' ')
		request.method += buffer[ind++];
	    ind++;
	    request.path = "";
	    while (buffer[ind] != ' ')
	    {
		if (buffer[ind] == '%')
		{
		    char* endPtr = buffer + ind + 3;
		    request.path += (char)strtol(buffer + ind + 1, 
						 &endPtr, 16);
		    ind += 3;
		}
		else
		{
		    request.path += buffer[ind++];
		}
	    }
	    if (request.method == "POST")
	    {
		bool foundStart = false;
		int numSlNs = 0;
		while (!foundStart)
		{
		    if (buffer[ind] == '\n') {
			numSlNs++;
			if (numSlNs >= 2)
			    foundStart = true;
		    } else if ((buffer[ind] != '\r') && (buffer[ind] != ' '))
			numSlNs = 0;
		    ind++;
		}
		request.body = "";
		for (; ind < n; ind++) {
		    request.body += buffer[ind];
		}
		while (n == HTTP_SERVER_BUFFER_SIZE)
		{
		    ind = 0;
		    n = read(newsockfd,buffer,HTTP_SERVER_BUFFER_SIZE);
		    for (; ind < n; ind++) {
			request.body += buffer[ind];
		    }
		}
	    }
	    HTTPResponse response = handler->handleRequest(request);
	    std::stringstream respStr;
	    respStr << "HTTP/1.1 " << response.code << " OK\n" <<
		"Content-Type: " << response.contentType << "; charset=utf-8\n" << 
		"Content-Length: " << response.body.size() << "\n\n" << 
		response.body;
	    /*respStr.flush();
	    const char * rc = respStr.str().c_str();
	    std::cout << respStr.gcount() << std::endl;
	    for (int i = 0; i < respStr.gcount(); i++)
	    {
		std::cout << rc[i];
	    }*/
	    n = write(newsockfd, respStr.str().c_str(), respStr.str().size());
	    //printf("Here is the message: %s\n",buffer);
	    //n = write(newsockfd,"<html><body><form method='post'><button>go</button></form></body></html>                                                ",75);
	    if (n < 0) error("ERROR writing to socket");
	    close(newsockfd);
	}
	close(sockfd);
    }
    void test()
    {
	handler->handleRequest(HTTPRequest());
    }
};