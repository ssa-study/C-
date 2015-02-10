// -*-tab-width:4-*-
// g++ -O2 -Wall src1.cpp
#include <stdio.h>
#include <stdint.h>
#include <algorithm>
#include <string>
#include <assert.h>
#include <sstream>
#include <string.h>
#include <boost/static_assert.hpp>


struct PacketHeader {
  uint32_t type;
};

template <size_t PayloadSize>
struct Packet {
  PacketHeader header;
  //uint8_t badbooy;
  uint8_t payload[PayloadSize];
};



typedef Packet<512> Packet512;

struct SubHeader {
  uint32_t length;
  uint8_t body[];
};




int main(int ac, char* av[]) {

  	Packet512 pkt1;
	PacketHeader& header = pkt1.header;
	header.type = 0;
	SubHeader&  subheader = reinterpret_cast<SubHeader&>(pkt1.payload);
	subheader.length = 12;

	printf("subheader address=%p\n", &subheader.length);
  
}


  
  

  

