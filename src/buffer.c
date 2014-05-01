#include <inttypes.h>
#include "buffer.h"

void ringBufferInit(volatile RingBuffer *buffer) {
	buffer->pIn = 0;
	buffer->pOut = 0;
	buffer->size = 0;
}

uint8_t ringBufferWrite(volatile RingBuffer *buffer, uint8_t toWrite) {
	if(buffer->size < RING_BUFFER_SIZE) {
		buffer->storage[buffer->pIn] = toWrite;
		buffer->pIn = (buffer->pIn + 1) % RING_BUFFER_SIZE;
		buffer->size++;
		return RING_BUFFER_STATUS_OK;
	} else {
		return RING_BUFFER_STATUS_OVERFLOW;
	}
}

uint8_t ringBufferRead(volatile RingBuffer *buffer, uint8_t *to) {
	if(buffer->size > 0) {
		*to = buffer->storage[buffer->pOut];
		buffer->pOut = (buffer->pOut + 1) % RING_BUFFER_SIZE;
		buffer->size --;
		return RING_BUFFER_STATUS_OK;
	} else {
		return RING_BUFFER_STATUS_UNDERRUN;
	}
}

/*
#include <assert.h>
#include <stdio.h>

int main() {
	RingBuffer buf;
	uint8_t val, i;
	ringBufferInit(&buf);

	assert(!ringBufferIsFull(&buf));
	
	assert(ringBufferRead(&buf, &val) == RING_BUFFER_STATUS_UNDERRUN);
	assert(ringBufferIsEmpty(&buf));
	
	assert(ringBufferWrite(&buf, 42) == RING_BUFFER_STATUS_OK);
	assert(!ringBufferIsEmpty(&buf));
	assert(!ringBufferIsFull(&buf));
	assert(ringBufferRead(&buf, &val) == RING_BUFFER_STATUS_OK);
	assert(ringBufferIsEmpty(&buf));
	
	ringBufferInit(&buf);
	for(i = 0; i < RING_BUFFER_SIZE; i++) {
		assert(ringBufferWrite(&buf, i) == RING_BUFFER_STATUS_OK);
		assert(buf.pIn == (i + 1) % RING_BUFFER_SIZE);
//		printf("%d\n", i);
		if(i < RING_BUFFER_SIZE-1) 
			assert(ringBufferIsFull(&buf) == 0);
		assert(ringBufferIsEmpty(&buf) == 0);
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
	assert(ringBufferIsEmpty(&buf) == 1);

	for(i = 0; i < 255; i++) {
		assert(ringBufferWrite(&buf, i) == RING_BUFFER_STATUS_OK);
		assert(ringBufferRead(&buf, &val) == RING_BUFFER_STATUS_OK);
		assert(val == i);
	}
}
//*/

