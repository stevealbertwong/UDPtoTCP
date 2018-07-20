#include "tcp.h"
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <cmath>
#include <iterator>
#include <algorithm>
#include <unistd.h>
#include <sys/fcntl.h> // for non-blocking
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#include <sys/select.h>
#include "packet.h"


using namespace std;

TCP_Server::TCP_Server(uint16_t serverPort, string filename)
: TCP(serverPort), m_filename(filename)
{
    m_serverFD = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);    

    memset((char *) &m_serverInfo, 0, m_serverLen);
    m_serverInfo.sin_family = AF_INET;
    m_serverInfo.sin_port = htons(serverPort);
    // Allow to bind to localhost only - Change to INADDR_ANY for any address - ASK TA
    //m_serverInfo.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    //m_serverInfo.sin_addr.s_addr = htonl(INADDR_ANY);

    // Accept to 0.0.0.0 which is generic for all NIC devices on machine    
    inet_pton(AF_INET, "0.0.0.0", &m_serverInfo.sin_addr.s_addr);    
    
    // 1 socket 1 port
    ::bind(m_serverFD, (struct sockaddr*)&m_serverInfo, 
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
    
    // int flags = fcntl(m_serverFD, F_GETFL);    
    // fcntl(m_serverFD, F_SETFL, flags |= O_NONBLOCK);

    struct timeval tv;
    tv.tv_sec = 6;
    tv.tv_usec = 500000;
    fd_set masterfds;
    fd_set readfds; // working sets
    FD_ZERO(&masterfds);
    FD_ZERO(&readfds);
    FD_SET(m_serverFD, &masterfds);

    while(1){
        memcpy(&readfds, &masterfds, sizeof(masterfds));
        select(m_serverFD + 1, &readfds, NULL, NULL, &tv); // block for 6.5s
        cout << "select blocks for a period of time, unblock either on time out or receives packet" << endl;

        if (FD_ISSET(m_serverFD, &readfds)){ // if server receives packet
            FD_CLR(m_serverFD, &readfds);
            recvfrom(m_serverFD, m_recvBuffer, MSS, 0, (struct sockaddr *)&m_clientInfo, &m_cliLen);
            // recvfrom(m_serverFD, m_recvBuffer, MSS, MSG_DONTWAIT, (struct sockaddr *)&m_clientInfo, &m_cliLen);
            // cout << m_recvBuffer << endl;

            Packet *pkt = new Packet(10,2,10, 1,1,1);
            pkt->debug((uint8_t*)m_recvBuffer);
            // Packet pkt = Packet();
            // pkt.debug((uint8_t*)m_recvBuffer);
            
            memset(m_recvBuffer, '\0', sizeof(m_sendBuffer));
            sendto(m_serverFD, "ACK", MSS, 0, (struct sockaddr *)&m_clientInfo, m_cliLen);

        } else {
            cout << "timeout " << endl;
        }

    }
}

