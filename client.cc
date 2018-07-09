#include "tcp.h"
#include <iostream>
using namespace std;

int main(int argc, char const *argv[])
{
	TCP_Client *tcpclient = new TCP_Client("localhost", 2000);
	// sendto()
	
	tcpclient->test();

	delete tcpclient;
	return 0;
}