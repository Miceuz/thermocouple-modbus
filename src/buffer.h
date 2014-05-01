#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#define RING_BUFFER_SIZE 64

#define RING_BUFFER_STATUS_OK 0
#define RING_BUFFER_STATUS_OVERFLOW 1
#define RING_BUFFER_STATUS_UNDERRUN 2

#define ringBufferIsEmpty(buffer) (0 == (buffer)->size)
#define ringBufferIsFull(buffer) (RING_BUFFER_SIZE == (buffer)->size)


typedef struct {
	uint8_t storage[RING_BUFFER_SIZE];
	uint8_t pIn;
	uint8_t pOut;
	uint8_t size;
} RingBuffer;

void ringBufferInit(volatile RingBuffer *buffer);
uint8_t ringBufferWrite(volatile RingBuffer *buffer, uint8_t toWrite);
uint8_t ringBufferRead(volatile RingBuffer *buffer, uint8_t *to);

#endif
