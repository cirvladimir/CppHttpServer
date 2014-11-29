// The server class

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
#include <pthread.h>


#define HTTP_SERVER_BUFFER_SIZE 1024
#define HTTP_MAX_CONNECTIONS 10

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
    pthread_t acceptThreadId;
    int sockfd;
    static void* invokeAsyncAccept(void* server)
    {
	((HTTPServer*)server)->asyncAccept();
    }
public:
    HTTPServer(HTTPRequestHandler * hndl, int port) 
    {
	handler = hndl;
	portno = port;
	started = false;
    }
    void start()
    {
	if (started)
	    return;
	started = true;
	struct sockaddr_in serv_addr;
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
	listen(sockfd, HTTP_MAX_CONNECTIONS);
	
	int rc = pthread_create(&acceptThreadId, NULL, invokeAsyncAccept, (void*)this);
	if (rc)
	    error("ERROR on starting new thread");
    }
    void stop()
    {
	if (!started)
	    return;
	started = false;
	close(sockfd);
	pthread_detach(acceptThreadId);
    }
    void asyncAccept()
    {
	struct sockaddr_in cli_addr;
	socklen_t clilen = sizeof(cli_addr);
	int newsockfd = accept(sockfd, 
		    (struct sockaddr *) &cli_addr, 
		    &clilen);
	int rc = pthread_create(&acceptThreadId, NULL, invokeAsyncAccept, (void*)this);
	if (rc)
	    error("ERROR on starting new thread");
	
	char buffer[HTTP_SERVER_BUFFER_SIZE];
	int n;
	
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
	n = write(newsockfd, respStr.str().c_str(), respStr.str().size());
	if (n < 0) error("ERROR writing to socket");
	close(newsockfd);
	pthread_exit(NULL);
    }
};
