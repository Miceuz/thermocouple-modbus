#include <inttypes.h>
#include "buffer.h"

void ringBufferInit(RingBuffer *buffer) {
	buffer->pIn = 0;
	buffer->pOut = 0;
	buffer->size = 0;
}

uint8_t ringBufferWrite(RingBuffer *buffer, uint8_t toWrite) {
	if(buffer->size < RING_BUFFER_SIZE) {
		buffer->storage[buffer->pIn] = toWrite;
		buffer->pIn = (buffer->pIn + 1) % RING_BUFFER_SIZE;
		buffer->size++;
		return RING_BUFFER_STATUS_OK;
	} else {
		return RING_BUFFER_STATUS_OVERFLOW;
	}
}

uint8_t ringBufferRead(RingBuffer *buffer, uint8_t *to) {
	if(buffer->size > 0) {
		*to = buffer->storage[buffer->pOut];
		buffer->pOut = (buffer->pOut + 1) % RING_BUFFER_SIZE;
		buffer->size --;
		return RING_BUFFER_STATUS_OK;
	} else {
		return RING_BUFFER_STATUS_UNDERRUN;
	}
}

uint8_t ringBufferIsEmpty(RingBuffer *buffer) {
	return 0 == buffer->size;
}

uint8_t ringBufferIsFull(RingBuffer *buffer) {
	return RING_BUFFER_SIZE == buffer->size;
}

/*
#include <assert.h>
#include <stdio.h>

int main() {
	RingBuffer buf;
	uint8_t val, i;
	ringBufferInit(&buf);
	
	assert(ringBufferRead(&buf, &val) == RING_BUFFER_STATUS_UNDERRUN);
	assert(ringBufferIsEmpty(&buf));
	
	for(i = 0; i < RING_BUFFER_SIZE; i++) {
		assert(ringBufferWrite(&buf, i) == RING_BUFFER_STATUS_OK);
		assert(buf.pIn == (i + 1) % RING_BUFFER_SIZE);
	}
	
	assert(buf.pOut == 0);
	assert(buf.pIn == 0);
	assert(buf.size == RING_BUFFER_SIZE);
	assert(ringBufferIsFull(&buf));
	assert(ringBufferWrite(&buf, i) == RING_BUFFER_STATUS_OVERFLOW);
	
	for(i = 0; i < RING_BUFFER_SIZE; i++) {
		assert(ringBufferRead(&buf, &val) == RING_BUFFER_STATUS_OK);
		assert(val == i);
	}

	for(i = 0; i < 255; i++) {
		assert(ringBufferWrite(&buf, i) == RING_BUFFER_STATUS_OK);
		assert(ringBufferRead(&buf, &val) == RING_BUFFER_STATUS_OK);
		assert(val == i);
	}
}
//*/

