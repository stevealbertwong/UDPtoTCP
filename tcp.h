#ifndef TCP_H
#define TCP_H

#include "globals.h"
#include <fstream>
#include <string.h>
#include <string>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <vector>

// Abstract base TCP class
class TCP {
protected:
    
	uint16_t m_port;
        
	// TCP_Packet* m_packet;
	char m_recvBuffer[MSS];
	char m_sendBuffer[MSS];

    // Size of the data read from the socket
    int m_recvSize;

public:
	TCP(uint16_t port): m_port(port) {};

	virtual ~TCP(){
		// if(m_packet){
		// 	delete m_packet;
		// }
	}
	virtual void test() = 0;

	// Accessors
	uint16_t getPort() const { return m_port; }
	// uint8_t* getReceivedPacket() { return m_recvBuffer; }
	// uint8_t* getSentPacket() { return m_sendBuffer; }
};


class TCP_Server: public TCP {
    
	std::string m_filename;

	struct sockaddr_in m_clientInfo;
	socklen_t m_cliLen = sizeof(m_clientInfo);
	struct sockaddr_in m_serverInfo;
	socklen_t m_serverLen = sizeof(m_serverInfo);    
	int m_sockFD;



public:
	TCP_Server(uint16_t serverPort, std::string filename);

	void test() override;
	void UDP_listen();
	void UDP_accept();
	void UDP_recv();
	void UDP_send();

	// Accessors
	int getSocketFD() const { return m_sockFD; }
	

};


class TCP_Client: public TCP {
    
	std::string m_serverHost;    
	struct sockaddr_in m_serverInfo;
	socklen_t m_serverLen = sizeof(m_serverInfo);    
	int m_sockFD;
    // Expected sequence used based on previous + data size
	// uint16_t m_expected_seq;
    // Buffer to hold written packets, to prevent re-writes
    std::vector<uint16_t> m_written;

public:
	TCP_Client(std::string serverHost, uint16_t serverPort);

	void test() override;
	void UDP_connect();
	void UDP_recv();
	void UDP_send();

	// Accessors
	int getSocketFD() const { return m_sockFD; }
	std::string getServerHost() const { return m_serverHost; }

};

#endif // TCP_H