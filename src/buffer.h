#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#define RING_BUFFER_SIZE 64

#define RING_BUFFER_STATUS_OK 0
#define RING_BUFFER_STATUS_OVERFLOW 1
#define RING_BUFFER_STATUS_UNDERRUN 2

typedef struct {
	uint8_t storage[RING_BUFFER_SIZE];
	uint8_t pIn;
	uint8_t pOut;
	uint8_t size;
} RingBuffer;

void ringBufferInit(RingBuffer *buffer);
uint8_t ringBufferWrite(RingBuffer *buffer, uint8_t toWrite);
uint8_t ringBufferRead(RingBuffer *buffer, uint8_t *to);

#endif
