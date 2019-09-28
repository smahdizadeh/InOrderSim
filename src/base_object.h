/*
 * base_object.h
 *
 *  Created on: Sep 28, 2019
 *      Author: sara
 */

#ifndef BASE_OBJECT_H_
#define BASE_OBJECT_H_

#include "util.h"

class Packet {
public:

	Packet(bool _isReq, bool _isWrite, PacketSrcType _type, uint32_t _addr,
			uint32_t _size, uint8_t* _data, uint32_t _ready_time) {
		isReq = _isReq;
		isWrite = _isWrite;
		type = _type;
		addr = _addr;
		size = _size;
		data = _data;
		ready_time = _ready_time;
	}
	virtual ~Packet() {
		if (data != nullptr)
			delete data;
	}

	bool isReq;
	bool isWrite;
	PacketSrcType type;
	uint32_t addr;
	uint32_t size;
	uint8_t* data;
	uint32_t ready_time;
};

class BaseObject {
public:
	BaseObject();
	virtual ~BaseObject();

	virtual bool sendReq(Packet * pkt) = 0;
	virtual void recvResp(Packet* readRespPkt) = 0;
};

#endif /* BASE_OBJECT_H_ */
