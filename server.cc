#include "tcp.h"
#include <iostream>
using namespace std;

int main(int argc, char const *argv[])
{
    TCP_Server *tcpserver = new TCP_Server(2000); // server port
    // tcpserver->test();
    
    tcpserver->TCP_listen();
    int fd = tcpserver->TCP_accept();

    // tcpserver->TCP_send(fd, "file.txt");
    tcpserver->test_send("test.data");

    delete tcpserver;
    return 0;
}