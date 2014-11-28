#include <bits/stdc++.h>
#include "server.h"

using namespace std;

class MyHandler : public HTTPRequestHandler
{
public:
    HTTPResponse handleRequest(HTTPRequest request)
    {
	/*cout << request.method << " request to " << request.path << endl;
	if (request.method == "POST")
	{
	    cout << "post body" << endl << request.body << endl;
	}*/
	//HTTPResponse response;
	//response.code = 200;
	//response.contentType = "text/html";
	//response.body = "<html><body>Hello world!</body></html>";
	return HTTPResponse();
    }
};

class A 
{
public:
    HTTPResponse test(HTTPRequest r) {
	return HTTPResponse();
	
    }
};

class B
{
public:
    A * h;
    B(A * hnd) 
    {
	h = hnd;
    }
    void test() 
    {
	h->test(HTTPRequest());
    }
};

int main()
{
    MyHandler hnd;
    HTTPServer server(&hnd, 1122);
    A a;
    B b(&a);
    //server.start();
    string an;
    while (true)
    {
	cin >> an;
	//b.test();
	server.test();
    }
}