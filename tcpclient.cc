#include "tcp.h"
#include "globals.h"
#include <iostream>
#include <stdlib.h>
#include <utility>
#include <time.h>
#include <algorithm>
#include <unistd.h>

using namespace std;

/* socket() */
TCP_Client::TCP_Client(string hostname, uint16_t server_port) : m_server_port(server_port){        

    // heap -> won't stack overflow
    handshakes = new std::unordered_map<std::string, tcp_handshake>();
    connections = new std::vector<tcp_connection>();

    m_server_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); // fd that reaches server    
    memset((char *)&m_server_info, 0, m_server_len);
    m_server_info.sin_family = AF_INET;
    m_server_info.sin_port = htons(m_server_port);    
    if(hostname == "localhost") { m_server_ip = "127.0.0.1"; }    
    inet_aton(m_server_ip.c_str(), &m_server_info.sin_addr);    
    
    m_server_ip = inet_ntoa(m_server_info.sin_addr); // IP in string
    // client socket did not bind port -> OS randomly assign port
}

// send SYN to server, record in handshakes
void TCP_Client::send_SYN(sockaddr_in m_server_info){    
    Packet syn_pkt = Packet(0,0,0,1,0,0);
    // syn_pkt.debug();
    // Packet fin_pkt = Packet(0,0,0,0,0,1);
    // fin_pkt.debug();
    // Packet syn_ack_pkt = Packet(0,0,0,1,1,0);
    // syn_ack_pkt.debug();
    // char test[] = "i love you";
    // Packet test_pkt = Packet(test, sizeof(test));

    // sendto(m_server_fd, syn_pkt.getPacket(), MSS, 0, (struct sockaddr *)&m_server_info, m_server_len);
    sendto(m_server_fd, syn_pkt.getPacket(), syn_pkt.getEncodedSize(), 0, (struct sockaddr *)&m_server_info, m_server_len);
    
    tcp_handshake hs = tcp_handshake();    
    hs.m_sockaddr = m_server_info;
    hs.syned = true;    
    
    // handshakes->insert(std::pair<std::string, tcp_handshake>(inet_ntoa(m_server_info.sin_addr), hs));
    handshakes->insert(make_pair(m_server_ip, hs));    
    syn_pkt.free_m_packet();
}

// loop through handshakes to get serverinfo, resend SYN
void TCP_Client::handshake_retransmit(){   
    // cout << "handshake_retransmit " << endl; 
    std::unordered_map<std::string, tcp_handshake>::iterator it;
    for ( it = handshakes->begin(); it != handshakes->end(); it++ ){
        send_SYN(it->second.m_sockaddr);
    }
}

// send SYN to server, record in handshakes
void TCP_Client::send_ACK(sockaddr_in m_server_info){    
    Packet syn_pkt = Packet(0,0,0,0,0,1);
    sendto(m_server_fd, syn_pkt.getPacket(), syn_pkt.getEncodedSize(), 0, (struct sockaddr *)&m_server_info, m_server_len);    
    syn_pkt.free_m_packet();
}

// if SYN ACK, delete handshakes, return true to create new fd
bool TCP_Client::ifSYNACKPacket(Packet pkt, sockaddr_in m_server_info){    
    cout << "server received SYN ACK packets : " << pkt.getFlag() << endl;
    bool keyExist = (handshakes->find(inet_ntoa(m_server_info.sin_addr)) != handshakes->end());
    
    if((pkt.getFlag() == 6) && keyExist){
        std::unordered_map<std::string, tcp_handshake>::iterator it;
        for ( it = handshakes->begin(); it != handshakes->end(); it++ ){
            // client has sent SYN before, remove server from handshakes, so no more retransmit
            if(!strcmp((it->first.c_str()), inet_ntoa(m_server_info.sin_addr)) && it->second.syned) {                
                handshakes->erase(inet_ntoa(m_server_info.sin_addr));     
                send_ACK(m_server_info);
                cout << "client successfully created connection and sent ACK. " << endl;                
                return true;
            }            
        }
        return true;
    }    
    // if not SYN ACK from SYNed server, return false to retransmit
    return false; 
    pkt.free_m_packet();    
}

/* handshake !!
if(connect(remote_socket, servinfo->ai_addr, servinfo->ai_addrlen) <0)*/
int TCP_Client::TCP_connect(){
    
    send_SYN(m_server_info);

    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 500000;
    fd_set masterfds;
    fd_set readfds; // working sets
    FD_ZERO(&masterfds);
    FD_ZERO(&readfds);
    FD_SET(m_server_fd, &masterfds); // add server fd to master set

    // make recv() return status code is it has been blocking over a period of time    
    // => while loop keep listening to select
    // timeout only implemented in recv w packets to resend ??
    // receive SYN/ACK + timeout retransmit 

    while(1){            
        
        memcpy(&readfds, &masterfds, sizeof(masterfds));
        // block for fixed time, check if receive any packets
        int numPacketsReceived = select(m_server_fd + 1, &readfds, NULL, NULL, &tv);                

        // if client receives SYN-ACK, check handshakes, send ACK packet
        if (FD_ISSET(m_server_fd, &readfds)){ 
            FD_CLR(m_server_fd, &readfds);

            uint32_t recv_size = recvfrom(m_server_fd, m_recv_buffer, MSS, 0, (struct sockaddr *)&m_server_info, &m_server_len);
            
            // local variable since need copy by value, gone in next iteration
            Packet recvd_pkt = Packet(m_recv_buffer, recv_size); // bytearray to packet so could call class methods
            // recvd_pkt->debug((uint8_t*)m_recv_buffer);
            memset(m_recv_buffer, '\0', sizeof(m_recv_buffer));
            
            if(ifSYNACKPacket(recvd_pkt, m_server_info)){
                tcp_connection connection = tcp_connection();
                connections->push_back(connection);                
                
                
                // delete entry in map handshakes
                return connections->size();  // fd as index of vector

            } else {                
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
        reconstruct file
            rearrange in order according to seq#
                priority_queue vs map -> always sorted
                    map instead of priority queue when keep a list of struct sorted
                    take a hit in performance in insert(), but quicker when iterate()
                vector -> later call sort()                
            parse data from byte array            
            << back to a file -> write() will overwrite 
        ACK ack#, resend ACK/data packet
            buffer not in sequence packets
            when receive new packet in sequence, update new ack#

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

/* pseudo code

while loop check if receive server packet
    if data packet
        if in sequence packet
            write to file
            write in sequence packets in buffer to file
        if NOT
            add to vector<Packet> buffer
        send ACK

    if FIN packet
        free() tcp_connection, fd
        break while loop
                
*/
void TCP_Client::TCP_recv(){
    // receive + rearrange packets in order + reconstruct file
    
    while(1){ // break if timeout or FIN packet
        // each loop 1 packet
        memset(m_recv_buffer, '\0', sizeof(m_recv_buffer));
        uint32_t recv_size = recvfrom(m_server_fd, m_recv_buffer, MSS, 0, (struct sockaddr *)&m_server_info, &m_server_len);
        cout << "recv_size : " << recv_size << endl;        
        // parse seq# n data from m_recv_buffer -> sorted map
        Packet recvd_pkt = Packet(m_recv_buffer, recv_size);        
        
        if(recvd_pkt.getFlag() == 1){ break; } // FIN

        uint16_t seq = recvd_pkt.getSeq();
        cout << "seq no : " << seq << endl;
        // packets_in_sequence.insert(pair<uint16_t, Packet>(seq, recvd_pkt));        
        packets_in_sequence[seq] = recvd_pkt;
        cout << "loop" << endl;
        // recvd_pkt.free_m_packet(); // don't free otherwise map will segfault
    }
    
    // iterate through sorted map -> ofstream write()
    // ofstream output_file("received_filed.data", std::ofstream::out | std::ofstream::app);
    ofstream output_file("received_filed.data");
    for(auto i = packets_in_sequence.begin(); i != packets_in_sequence.end(); i++){
        vector<uint8_t> v = i->second.m_payload;
        for(vector<uint8_t>::iterator it = v.begin(); it!=v.end(); it++){
            // cout << *it << endl;
            output_file << *it;
        }

        // for(ssize_t i = 0; i < v.size(); i++){
        //     output_file << v.at(i);
        // }
        // output_file.write((char *)&v[0], i->second.m_payload.size()); // EACH WRITE() CURSOR IS ADVANCED
    }

    cout << "packets_in_sequence.size() : " << packets_in_sequence.size() << endl;

    // for(ssize_t i = 0; i < data_size; i++){
    //     outputFile << m_payload->at(i);
    // }   
}

void TCP_Client::test_recv(){
    // receive + rearrange packets in order + reconstruct file
    
    while(1){ // break if timeout or FIN packet
        // each loop 1 packet
        memset(m_recv_buffer, '\0', sizeof(m_recv_buffer));
        uint32_t recv_size = recvfrom(m_server_fd, m_recv_buffer, MSS, 0, (struct sockaddr *)&m_server_info, &m_server_len);
        cout << "recv_size : " << recv_size << endl;        
        // parse seq# n data from m_recv_buffer -> sorted map
        Packet recvd_pkt = Packet(m_recv_buffer, recv_size);        
        
        if(recvd_pkt.getFlag() == 1){ break; } // FIN

        uint16_t seq = recvd_pkt.getSeq();
        cout << "seq no : " << seq << endl;
        // packets_in_sequence.insert(pair<uint16_t, Packet>(seq, recvd_pkt));        
        packets_in_sequence[seq] = recvd_pkt;
        cout << "loop" << endl;
        // recvd_pkt.free_m_packet(); // don't free otherwise map will segfault
    }
    
    // iterate through sorted map -> ofstream write()
    // ofstream output_file("received_filed.data", std::ofstream::out | std::ofstream::app);
    ofstream output_file("received_filed.data");
    for(auto i = packets_in_sequence.begin(); i != packets_in_sequence.end(); i++){
        vector<uint8_t> v = i->second.m_payload;
        for(vector<uint8_t>::iterator it = v.begin(); it!=v.end(); it++){
            // cout << *it << endl;
            output_file << *it;
        }

        // for(ssize_t i = 0; i < v.size(); i++){
        //     output_file << v.at(i);
        // }
        // output_file.write((char *)&v[0], i->second.m_payload.size()); // EACH WRITE() CURSOR IS ADVANCED
    }

    cout << "packets_in_sequence.size() : " << packets_in_sequence.size() << endl;

    // for(ssize_t i = 0; i < data_size; i++){
    //     outputFile << m_payload->at(i);
    // }
   
}


void TCP_Client::test(){
    char data[] = "testing";
    // typedef struct {short x2;int x;} msgStruct;
    // typedef struct {int x;short x2;int y;short y2;} msgStruct;
    // typedef struct {int x;short x2;} msgStruct;
    // typedef struct {int x;int y;short x2;short y2;} msgStruct;
    typedef struct {int x;short x2;char pad[2];int y;short y2;} msgStruct;
    msgStruct msg;
    cout << sizeof(msg) << endl;
    // msg.x = 5;

    Packet *pkt = new Packet(10, 10, 10, 1,1,1);    
    // pkt->m_packet = pkt->encode();
    
    pkt->debug();

    for (int i = 0; i < 3; ++i){
        memset(m_send_buffer, '\0', sizeof(m_send_buffer));
        
        strcpy(m_send_buffer, data);    
        cout << m_send_buffer << endl;
        
        // memcpy(m_send_buffer, pkt->getPacket(), sizeof(Packet));
        sendto(m_server_fd, m_send_buffer, MSS, 0, (struct sockaddr *)&m_server_info, m_server_len);
        cout << "sent packets" << endl;
        memset(m_recv_buffer, '\0', sizeof(m_recv_buffer));
        recvfrom(m_server_fd, m_recv_buffer, MSS, 0, (struct sockaddr *)&m_server_info, &m_server_len);
        cout << m_recv_buffer << endl;
    }

}