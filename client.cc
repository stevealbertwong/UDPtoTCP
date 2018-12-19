#include "tcp.h"
#include <iostream>
using namespace std;

int main(int argc, char const *argv[])
{
	TCP_Client *tcpclient = new TCP_Client("localhost", 2000); // server port: 2000	
	// tcpclient->test();
	
	int fd = tcpclient->TCP_connect();
	// tcpclient->TCP_recv(fd, "file.txt"); // receive from server, save as file.txt

	delete tcpclient;
	return 0;
}