#include "tcp.h"
#include "globals.h"
#include <iostream>
#include <stdlib.h>
#include <utility>
#include <time.h>
#include <algorithm>
#include <unistd.h>

using namespace std;

TCP_Client::TCP_Client(string serverHost, uint16_t serverPort)
: TCP(serverPort), m_serverHost(serverHost)
{    
    m_sockFD = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);    
    
    memset((char *)&m_serverInfo, 0, m_serverLen);
    m_serverInfo.sin_family = AF_INET;
    m_serverInfo.sin_port = htons(serverPort);
    string hostname = serverHost;
    if(serverHost == "localhost") { hostname = "127.0.0.1"; }    
    inet_aton(hostname.c_str(), &m_serverInfo.sin_addr);

}

// UDP to TCP
// TODO: handshake i.e. udp version of connect()
void TCP_Client::UDP_connect(){

}


void TCP_Client::UDP_recv(){

}


void TCP_Client::UDP_send(){

}

void TCP_Client::test(){
    char data[] = "testing";
    strcpy(m_sendBuffer, data);

    for (int i = 0; i < 3; ++i)
    {
        memset(m_sendBuffer, '\0', sizeof(m_sendBuffer));
        strcpy(m_sendBuffer, data);
        sendto(m_sockFD, m_sendBuffer, MSS, 0, (struct sockaddr *)&m_serverInfo, m_serverLen);
        
        memset(m_recvBuffer, '\0', sizeof(m_recvBuffer));
        recvfrom(m_sockFD, m_recvBuffer, MSS, 0, (struct sockaddr *)&m_serverInfo, &m_serverLen);
        cout << m_recvBuffer << endl;
    }

}