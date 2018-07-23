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
    m_serverFD = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);    
    
    memset((char *)&m_serverInfo, 0, m_serverLen);
    m_serverInfo.sin_family = AF_INET;
    m_serverInfo.sin_port = htons(serverPort);
    string hostname = serverHost;
    if(serverHost == "localhost") { hostname = "127.0.0.1"; }    
    inet_aton(hostname.c_str(), &m_serverInfo.sin_addr);

    // client did not bind port => OS assign port to socket
}

// UDP to TCP
// TODO: handshake i.e. udp version of connect()
// if(connect(remote_socket, servinfo->ai_addr, servinfo->ai_addrlen) <0)
void TCP_Client::UDP_connect(){
    
    // setsocktopt() to return flag if itself fails
    // also make send() return status code is it has been blocking over a period of time
    // what if sliding window ??

    // engineer + send SYN 

    // receive SYN/ACK + timerout retransmit 
    // => while loop keep listening to select
    // timeout only implemented in recv w packets to resend ??

    // send ACK
    
    // return 1,0,-1
}


void TCP_Client::UDP_recv(){

}


void TCP_Client::UDP_send(){

}

void TCP_Client::test(){
    // char data[] = "testing";
    // typedef struct {short x2;int x;} msgStruct;
    // typedef struct {int x;short x2;int y;short y2;} msgStruct;
    // typedef struct {int x;short x2;} msgStruct;
    // typedef struct {int x;int y;short x2;short y2;} msgStruct;
    typedef struct {int x;short x2;char pad[2];int y;short y2;} msgStruct;
    msgStruct msg;
    cout << sizeof(msg) << endl;
    // msg.x = 5;

    Packet *pkt = new Packet(10, 10, 10, 1,1,1);    
    pkt->m_packet = pkt->encode();

    // strcpy(m_sendBuffer, data);
    

    for (int i = 0; i < 3; ++i)
    {
        memset(m_sendBuffer, '\0', sizeof(m_sendBuffer));
        // strcpy(m_sendBuffer, data);
        memcpy(m_sendBuffer, pkt->m_packet, sizeof(Packet));
        sendto(m_serverFD, m_sendBuffer, MSS, 0, (struct sockaddr *)&m_serverInfo, m_serverLen);
        
        memset(m_recvBuffer, '\0', sizeof(m_recvBuffer));
        recvfrom(m_serverFD, m_recvBuffer, MSS, 0, (struct sockaddr *)&m_serverInfo, &m_serverLen);
        cout << m_recvBuffer << endl;
    }

}