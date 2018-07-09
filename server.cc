#include "tcp.h"
#include <iostream>
using namespace std;

int main(int argc, char const *argv[])
{
    TCP_Server *tcpserver = new TCP_Server(2000, "filename.txt");
    tcpserver->test();

    delete tcpserver;
    return 0;
}