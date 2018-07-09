#include "tcp.h"
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include "globals.h"
#include <cmath>
#include <iterator>
#include <algorithm>
#include <unistd.h>

using namespace std;

TCP_Server::TCP_Server(uint16_t serverPort, string filename)
: TCP(serverPort), m_filename(filename)
{
    m_sockFD = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);    

    memset((char *) &m_serverInfo, 0, m_serverLen);
    m_serverInfo.sin_family = AF_INET;
    m_serverInfo.sin_port = htons(serverPort);
    // Allow to bind to localhost only - Change to INADDR_ANY for any address - ASK TA
    //m_serverInfo.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    //m_serverInfo.sin_addr.s_addr = htonl(INADDR_ANY);

    // Accept to 0.0.0.0 which is generic for all NIC devices on machine    
    inet_pton(AF_INET, "0.0.0.0", &m_serverInfo.sin_addr.s_addr);    
    
    ::bind(m_sockFD, (struct sockaddr*)&m_serverInfo, 
        sizeof(m_serverInfo));
}

// UDP to TCP
void TCP_Server::UDP_listen(){

}

// TODO: handshake i.e. UDP version of accept()
void TCP_Server::UDP_accept(){

}

void TCP_Server::UDP_recv(){
    
}

void TCP_Server::UDP_send(){

}

void TCP_Server::test(){
    while(1){
        recvfrom(m_sockFD, m_recvBuffer, MSS, 0, (struct sockaddr *)&m_clientInfo, &m_cliLen);

        cout << m_recvBuffer << endl;
        sendto(m_sockFD, "ACK", MSS, 0, (struct sockaddr *)&m_clientInfo, m_cliLen);
    }
}

