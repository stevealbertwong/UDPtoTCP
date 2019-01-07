/*
data structure to implement TCP n Tahoe
w network syscall that updates such data structure

1. abstract class, inheritance

*/
#ifndef TCP_H
#define TCP_H

#include "globals.h"
#include <fstream>
#include <string.h>
#include <string>
#include <cstring>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <vector>
#include <map>
#include <unordered_map>
#include "packet.h"

/*************************************************************/
// TCP stores tcp packets fields + recvfrom() + TCP reno
//	1. packets fields: source dest ip + port + flags + ack/seq#
//	2. recv_buffer, send_buffer 
//	3. Reno: 
/*************************************************************/

/* abstract class
- contains pure virtual function
- cannot be instantiated without child class */
class TCP {

protected:
	struct packet_status{
		Packet p; // free() when packet is ACKed by client
		struct timeval m_time_sent; 
    	bool m_sent = false;
    	bool m_acked = false;
	};

	struct tcp_connection{ // fd
		uint32_t m_seq_num; // uint32_t 1 more bit, prevent overflow
		uint32_t m_ack_num;		
		uint32_t m_duplicate_ACKs;

		float m_cwnd; // Tahoe computed server congestion window
		uint16_t m_client_window = START_WINDOW/PACKET_SIZE; 
		uint32_t m_remain_data_packets; // size of vector of data packets
		ssize_t m_current_window; // no. packets client send per round

		struct sockaddr_in m_sockaddr;
		socklen_t m_socklen = sizeof(m_sockaddr);
		int m_FD; // int for OS to map socket msg queue to process
		
		uint16_t m_port;
		std::string m_filename;
		std::vector<packet_status> m_data_packets; // Tahoe retransmit
	};
	std::vector<tcp_connection> *connections; // int fd -> random access	
	
	struct tcp_handshake{ // handshake status		
		struct sockaddr_in m_sockaddr;				
		bool syned; // server received SYN packet
		bool synacked;	// client received SYN ACK
		bool acked;	// server received ACK, server start to send data packet
	};	
	std::unordered_map<std::string, tcp_handshake> *handshakes; // ip : handshake status

	char m_recv_buffer[MSS]; // recv()
	char m_send_buffer[MSS]; // send()   
		
	// fd_set master_fds; // select()
    // fd_set working_fds;

	// temporary storing packets from another client
    std::vector<Packet> stored_handshake_packets; 
	std::vector<Packet> stored_data_packets; 

public:
	TCP(){};	
	virtual ~TCP(){ // virtual destructor -> virtual to garbage collect
		// if(m_packet){
		// 	delete m_packet;
		// }
	}
	// Accessors	
	// uint8_t* getReceivedPacket() { return m_recv_buffer; }
	// uint8_t* getSentPacket() { return m_send_buffer; }
	
	virtual void test() = 0;
	virtual void handshake_retransmit()=0;
	// virtual void segment_retransmit()=0;
	// virtual bool ifExpectedHandshakePacket(Packet pkt, sockaddr_in m_client_info)=0;
	// virtual bool ifExpectedSegmentPacket(Packet pkt, sockaddr_in m_client_info)=0;
};

/*************************************************************/
// SERVER
//
/*************************************************************/

class TCP_Server: public TCP {

private:
	uint16_t m_server_port; // client's port is randomly assigned
	
	
	struct sockaddr_in m_server_info; // yourself
	socklen_t m_server_len = sizeof(m_server_len);    
	int m_server_fd; // both server n client use server fd send(), recv()
		
	struct sockaddr_in m_client_info; // connected client
	socklen_t m_client_len = sizeof(m_client_info);
	std::string m_client_ip; // inet_ntoa(m_server_info.sin_addr)

	// TODO: should be struct of tcp not just client addr info
    // std::map<uint32_t, sockaddr_in> client_fd_map; // fd to client addr info
    
	ssize_t m_ssthresh;
	float m_cwnd;
	uint8_t m_curr_mode = SS;

public:
	TCP_Server(uint16_t m_server_port); // register this process in OS as server
	
	// main TCP methods
	void test();
	void TCP_listen();
	int TCP_accept();
	void TCP_recv();
	void TCP_send(const char* filename);
	void test_send(const char* filename);
	
	// helper		
	void send_SYN_ACK(sockaddr_in m_client_info);	
	bool ifSYNPacket(Packet pkt, sockaddr_in m_client_info);
	bool ifACKPacket(Packet pkt, sockaddr_in m_client_info);
	void handshake_retransmit();

	// Accessors
	int getSocketFD() const { return m_server_fd; }
	
	// Mutators	
};

/*************************************************************/
// CLIENT
//
/*************************************************************/

class TCP_Client: public TCP {

private:
	uint16_t m_server_port;
	std::string m_server_ip; // inet_ntoa(m_server_info.sin_addr)
	
	struct sockaddr_in m_server_info;
	socklen_t m_server_len = sizeof(m_server_info);    
	int m_server_fd; // both server n client use server fd send(), recv()


    // Expected sequence used based on previous + data size
	// uint16_t m_expected_seq;
    // Buffer to hold written packets, to prevent re-writes
    // std::vector<uint16_t> m_written;
	
public:
	TCP_Client(std::string m_server_ip, uint16_t m_server_port);

	void test();
	int TCP_connect();
	void TCP_recv();
	void test_recv();
	void TCP_send();

	// helper
	void send_SYN(sockaddr_in m_server_info);
	void send_ACK(sockaddr_in m_server_info);
	bool ifSYNACKPacket(Packet pkt, sockaddr_in m_server_info);
	void handshake_retransmit(); 

	// Accessors
	int get_socket_fd() const { return m_server_fd; }
	std::string getm_server_ip() const { return m_server_ip; }

	std::map<uint16_t, Packet> packets_in_sequence;
};

#endif // TCP_H