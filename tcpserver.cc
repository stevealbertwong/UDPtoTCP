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

/* socket() */
TCP_Server::TCP_Server(uint16_t server_port) : m_server_port(server_port){
    
    // heap -> won't stack overflow
    handshakes = new std::unordered_map<std::string, tcp_handshake>();
    connections = new std::vector<tcp_connection>();

    m_server_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);    
    memset((char *) &m_server_info, 0, m_server_len);
    m_server_info.sin_family = AF_INET;
    m_server_info.sin_port = htons(m_server_port);

    /* Allow to bind to localhost only - Change to INADDR_ANY for any address */
    //m_server_info.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    //m_server_info.sin_addr.s_addr = htonl(INADDR_ANY);

    // 0.0.0.0 -> generic for all NIC devices on machine    
    inet_pton(AF_INET, "0.0.0.0", &m_server_info.sin_addr.s_addr);   
    ::bind(m_server_fd, (struct sockaddr*)&m_server_info, 
        sizeof(m_server_info)); // register process w OS
    
}

/* bind() */
void TCP_Server::TCP_listen(){
  
}

void TCP_Server::send_SYN_ACK(sockaddr_in m_client_info){
    Packet syn_ack_pkt = Packet(0,0,0,1,1,0);    
    sendto(m_server_fd, syn_ack_pkt.getPacket(), syn_ack_pkt.getEncodedSize(), 0, (struct sockaddr *)&m_client_info, m_client_len);               
    cout << "client sent SYN ACK packets" << endl;
    syn_ack_pkt.free_m_packet();
}

/* if timeout or checksum error, server only needs to retransmit outstanding ACK/SYN */
void TCP_Server::handshake_retransmit(){     
    // cout << "handshake_retransmit " << endl;       
    std::unordered_map<std::string, tcp_handshake>::iterator it;
    for ( it = handshakes->begin(); it != handshakes->end(); it++ ){
        send_SYN_ACK(it->second.m_sockaddr); 
    }
}

/* if SYN, add to connection, send SYN-ACK */
bool TCP_Server::ifSYNPacket(Packet pkt, sockaddr_in m_client_info){    
    // if SYN, create new handshakes, return false to not to create new fd
    if(pkt.getFlag() == 4){
        cout << "client received SYN packets" << endl;
        tcp_handshake hs = tcp_handshake();
        hs.m_sockaddr = m_client_info;
        hs.syned = true;
        
        // inet_ntoa() -> turn in_addr to char*
        handshakes->insert(std::pair<std::string, tcp_handshake>(inet_ntoa(m_client_info.sin_addr), hs));
        // handshakes->insert(make_pair(m_client_ip, hs));    
        
        send_SYN_ACK(m_client_info); 
                
        return true;
    } else {
        return false;
    }    
    // DONT FREE PACKET HERE, OTHERWISE FREE TWICE ??
    pkt.free_m_packet();
}

/* if ACK, ++fd, delete outstanding packets */
bool TCP_Server::ifACKPacket(Packet pkt, sockaddr_in m_client_info){    
    if(pkt.getFlag() == 1){ // if ACK
        
        std::unordered_map<std::string, tcp_handshake>::iterator it;
        
        for ( it = handshakes->begin(); it != handshakes->end(); it++ ){
            
            // if client has sent SYN before -> remove client from handshakes, so no more retransmit
            if(!strcmp((it->first.c_str()), inet_ntoa(m_client_info.sin_addr)) && it->second.syned) {                
                
                handshakes->erase(inet_ntoa(m_client_info.sin_addr));                        
                tcp_connection connection = tcp_connection();
                connection.m_sockaddr = m_client_info;
                connection.m_socklen = sizeof(m_client_info);
                connection.m_FD = connections->size() + 1;
                connections->push_back(connection);
                cout << "server received ACK packets and successfully created connection" << endl;
                return true;
            }                         
        }        
        return false; // no SYN recorded in handshakes, ignore packet
    } else { // not ACK
        return false;
    }    
    // DONT FREE PACKET HERE, OTHERWISE FREE TWICE ??
    pkt.free_m_packet();
}

/* Handshake !!

1. every 6.5 sec check if expected packet received, if not retransmit, if yes stored handshake as fd
TCP_accept() is blocking since using select() 

2. select() receive SYN packet -> check client IP, port
if receive further ACK packet -> store "connection", return fd

NOTE:
if this process receives packet it must be that port !!!
udp version of select() does not return client sockets -> only server socket

TODO: new thread for each new client
there WILL be situation TCP_accept() receive packet other than SYN or ACK from another TCP_client
-> store received data packets to deal w in next iteration

*/
int TCP_Server::TCP_accept(){
    
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 500000;
    fd_set masterfds;
    fd_set readfds; // working sets
    FD_ZERO(&masterfds);
    FD_ZERO(&readfds);
    FD_SET(m_server_fd, &masterfds); // add server fd to master set
        
    while(1){
        memcpy(&readfds, &masterfds, sizeof(masterfds)); // copy master set fd into working sets fd
        // block for fixed time, update fd in working sets if receive packets
        int numPacketsReceived = select(m_server_fd + 1, &readfds, NULL, NULL, &tv);         
        cout << "server numPacketsReceived : " << numPacketsReceived << endl; 

        // if server receives packet, check handshakes, send SYN or ACK packet
        if (FD_ISSET(m_server_fd, &readfds)){ // if server fd in working set is updated i.e. received packets
            FD_CLR(m_server_fd, &readfds);

            uint32_t recv_size = recvfrom(m_server_fd, m_recv_buffer, MSS, 0, (struct sockaddr *)&m_client_info, &m_client_len);
            m_client_ip = inet_ntoa(m_server_info.sin_addr);

            cout << "recv size : " << recv_size << endl;
            // local variable since need copy by value, gone in next iteration
            Packet recvd_pkt = Packet(m_recv_buffer, recv_size); // bytearray to packet so could call class methods
            
            // recvd_pkt->debug((uint8_t*)m_recv_buffer);            
            memset(m_recv_buffer, '\0', sizeof(m_recv_buffer));
            
            if(ifSYNPacket(recvd_pkt, m_client_info)){ // if SYN, ++handshakes, send SYN-ACK                 
                
                continue; // restart while loop

            } else if (ifACKPacket(recvd_pkt, m_client_info)) { // if ACK, ++fd, --handshakes                
                return connections->size();  // fd as index of vector

            } else { // if data packet
                stored_data_packets.push_back(recvd_pkt); // TODO: deal w data packets from another client                                
                cout << "othr pcket handshake_retransmit" << endl;
                handshake_retransmit();
            }
            recvd_pkt.free_m_packet();

        } else { // if timeout n receives no packet(numPacketsReceived == 0) or error(-1)
            // FD_CLR(m_server_fd, &readfds);
            cout << "timeout handshake_retransmit" << endl;
            handshake_retransmit();             
        }
    }
}

/*
1. helper functions
    divide files into data packets + reconstruct back to file
        divide file into data packets
            seekg() + tellg() -> seq#
            malloc() data packet only when send, free() when ACKed
        packet constructor
            memcpy() data packet to byte array, X c-style typecast
        reconstruct back
            parse data from byte array
            append back to a file

    fixed sliding window + selective retranmit/repeat
    timer 
        self implemented timer
            gettimeofday()            
            vector of packets, each has different timeout
            for each packet that has timeout, resend packet
        timer block
            setsockopt() or select()
            1 packet
        non block
            recvfrom(MSG_DONTWAIT)
            1 packet

2. Tahoe -> elastic sliding window + ssthresh 
    2.a start as SS
            random cwnd, ssthresh
            send cwnd number of data packets, wait for ACKs
    2.b SS 
            if received ACKs, cwnd += MSS (MSS: max segment size, += MSS adds previous sent total number of packet size -> exponential increaseMSS: max segment size, += MSS adds previous sent total number of packet size -> exponential increase)
            if cwnd > ssthresh 
                SS -> CA
            if 3 dup ACKs or timeout
                remain SS
                ssthread = cwnd/2
                cwnd = 1
                retransmit that lost packet
        CA
            if received ACKs, cwnd += 1
            if 3 dup ACKs or timeout
                CA -> SS
                ssthread = cwnd/2
                cwnd = 1
                retransmit that lost packet
            
3. Reno -> elastic sliding window + ssthresh + 3 dup ACKs

*/

void TCP_Server::TCP_send(const char* filename){

    std::ifstream file(filename);
    file.seekg(0, file.end); 
    uint32_t total_file_bytes = file.tellg(); // current stream cursor position
    file.seekg(0, file.beg); 

    int total_data_packets = ceil((float)total_file_bytes/PACKET_SIZE);    
    int ack = 0; // last ACK received
    int seq = 0; // last data segment sent
    ssize_t total_sent_bytes = 0;
    
    for(int i = 0; i < total_data_packets; i++){
                
        ssize_t remain_bytes = total_file_bytes - file.tellg(); // total bytes - cursor 
        
        // might not enough data to fill last data packet 
        ssize_t data_packet_size;
        
        if(remain_bytes < PACKET_SIZE){ 
            data_packet_size = remain_bytes;
        } else {      
            data_packet_size = PACKET_SIZE;
        }
        
        char buffer[data_packet_size];
        file.read(buffer, data_packet_size); // EACH READ(), CURSOR IS ADVANCED
        cout << "file buffer : " << buffer << endl;
        Packet data_packet = Packet(0,0,0,0,0,0); // init header to 0                                    
        // data_packet.m_payload = std::vector<uint8_t>(buffer, buffer+PACKET_SIZE);        
        data_packet.setPayload(buffer, data_packet_size); // array to vector         
        data_packet.setSeq(total_sent_bytes); // seq#
        total_sent_bytes += data_packet_size;
        cout << "total_sent_bytes : " << total_sent_bytes << endl;
    
        data_packet.encode(); // header n data packet -> byte array
        sendto(m_server_fd, data_packet.getPacket(), data_packet.getEncodedSize(), 0, (struct sockaddr *)&m_client_info, m_client_len);                
        data_packet.free_m_packet();
    }

    // all data packets succesfully sent -> FIN
    Packet fin_packet = Packet(0,0,0,0,0,1);
    sendto(m_server_fd, fin_packet.getPacket(), fin_packet.getEncodedSize(), 0, (struct sockaddr *)&m_client_info, m_client_len);                
    fin_packet.free_m_packet();
}

// void TCP_Server::TCP_send(int client_socket_fd, Packet pkt){
//     // given int fd, find corresponding clientinfo + tcp status
//     sockaddr_in client_info = client_fd_map[client_socket_fd];    
//     memset(m_send_buffer, '\0', sizeof(m_send_buffer));
//     memcpy(m_send_buffer, byte_array, sizeof(Packet));
//     sendto(m_server_fd, pkt.m_packet, MSS, 0, (struct sockaddr *)&m_client_info, m_client_len);
// }


// testing any of above code snippets
// rmb to delete before production !!!
void TCP_Server::test(){
    
    // int flags = fcntl(m_server_fd, F_GETFL);    
    // fcntl(m_server_fd, F_SETFL, flags |= O_NONBLOCK);

    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 500000;
    fd_set masterfds;
    fd_set readfds; // working sets
    FD_ZERO(&masterfds);
    FD_ZERO(&readfds);
    FD_SET(m_server_fd, &masterfds);

    while(1){
        memcpy(&readfds, &masterfds, sizeof(masterfds));
        select(m_server_fd + 1, &readfds, NULL, NULL, &tv); // block for 6.5s
        cout << "loop" << endl;

        if (FD_ISSET(m_server_fd, &readfds)){ // if server receives packet
            cout << "received packets" << endl;
            FD_CLR(m_server_fd, &readfds);
            recvfrom(m_server_fd, m_recv_buffer, MSS, 0, (struct sockaddr *)&m_client_info, &m_client_len);
            // recvfrom(m_server_fd, m_recv_buffer, MSS, MSG_DONTWAIT, (struct sockaddr *)&m_client_info, &m_client_len);
            // cout << m_recv_buffer << endl;

            Packet *pkt = new Packet(10,2,10,1,1,1);
            
            // Packet pkt = Packet();
            pkt->debug();            
            memset(m_recv_buffer, '\0', sizeof(m_send_buffer));
            sendto(m_server_fd, "ACK", MSS, 0, (struct sockaddr *)&m_client_info, m_client_len);

        } else {
            cout << "timeout " << endl;
        }
    }
}

