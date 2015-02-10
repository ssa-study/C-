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

template <size_t PayloadSize, typename PayloadType=uint32_t>
struct Packet {
  PacketHeader header;
  uint8_t badbooy;
  union {
	uint8_t buffer[PayloadSize];
	PayloadType payload;
  };
};



typedef Packet<512> Packet512;

struct SubHeader {
  uint32_t length;
  uint8_t body[];
};




int main(int ac, char* av[]) {

  	Packet512 pkt1;
	Packet<512, SubHeader> pkt2;

	SubHeader&  subheader1 = reinterpret_cast<SubHeader&>(pkt1.payload);
	subheader1.length = 1234;
	SubHeader&  subheader2 = reinterpret_cast<SubHeader&>(pkt2.payload);
	subheader2.length = 1234;

	printf("subheader address=%p\n", &subheader2.length);
  
}


  
  

  

