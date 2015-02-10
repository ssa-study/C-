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

typedef uint8_t NullData[0] ;

template <size_t PayloadSize, typename PayloadType=NullData>
struct Packet {
  BOOST_STATIC_ASSERT(PayLoadSize <= sizeof(PayLoadType));
  template <typename Extend>
  struct ExtendPayload : Packet<PayloadSize, Extend> {

	operator Packet<PayloadSize,PayloadType>& () {
	  return reinterpret_cast<Packet<PayloadSize,PayloadType>&>(*this);
	}
  };
  
  PacketHeader header;
  uint8_t badbooy;
  union {
	uint8_t buffer[PayloadSize];
	PayloadType payload;
  };
};


typedef Packet<512> Packet512;

template <typename PacketType>
struct PacketMaker {
  struct SubHeader {
	uint32_t length;
	uint8_t body[];
  };

  typename PacketType::template ExtendPayload<SubHeader> PacketWidhSubHeader;
  PacketWidhSubHeader packetData;

  void makePacket(char* buffer, size_t size) {
	SubHeader&  subheader = reinterpret_cast<SubHeader&>(packetData.payload);
	subheader.length = size;
	memcpy(subheader.body, buffer, size);
  };

  PacketType& getPacket() { return packetData; }

};

int main(int ac, char* av[]) {

  PacketMaker<Packet512> packetMaker;

  packetMaker.makePacket(av[0], strlen(av[0]));

  
}


  
  

  

