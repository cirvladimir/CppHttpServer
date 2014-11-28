#include <bits/stdc++.h>
#include "server.h"

using namespace std;

class MyHandler : public HTTPRequestHandler
{
public:
    HTTPResponse handleRequest(HTTPRequest request)
    {
	cout << request.method << " request to " << request.path << endl;
	if (request.method == "POST")
	{
	    cout << "post body" << endl << request.body << endl;
	}
	HTTPResponse response;
	response.code = 200;
	response.contentType = "text/html";
	response.body = "<html><body><form method='post'><input name='sdfsd' value='dsfsdfsdfasd'/><button>go</button></form></body></html>";
	return response;
    }
};

int main()
{
    MyHandler hnd;
    HTTPServer server(&hnd, 1122);
    server.start();
}