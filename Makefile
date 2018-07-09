.PHONY: all clean simple-tcp-server simple-tcp-client
CXX=g++
CXXOPTIMIZE= -O2
CXXFLAGS= -g -Wall -pthread -std=c++14 $(CXXOPTIMIZE)
# CLASSES=packet.cpp

all: simple-tcp-client simple-tcp-server 

clean:
	rm -rf *.o *~ *.gch *.swp *.dSYM simple-tcp-server simple-tcp-client

simple-tcp-client:
	$(CXX) -o $@ $(CXXFLAGS) tcpclient.cc client.cc

simple-tcp-server:
	$(CXX) -o $@ $(CXXFLAGS) tcpserver.cc server.cc