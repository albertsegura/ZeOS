#ifndef _CIRCULAR_BUFFER
#define _CIRCULAR_BUFFER

typedef struct {
		int start, end, size;
		char *buffer;
}Circular_Buffer;


// TODO revisar static inline
static inline void circularbInit(Circular_Buffer* cb, char *buf, int size) {
	cb->buffer = buf;
	cb->start = 0;
	cb->end = 0;
	cb->size = size;
}

static inline void circularbFree(Circular_Buffer *cb) {
	cb->start = 0;
	cb->end = 0;
}

static inline int circularbIsFull(Circular_Buffer *cb) {
	return (cb->end + 1)%cb->size == cb->start;
}

static inline int circularbIsEmpty(Circular_Buffer *cb) {
	return cb->end == cb->start;
}

static inline int circularbWrite(Circular_Buffer *cb, char *element) {
	if (circularbIsFull(cb)) return -1;
	else {
		cb->buffer[cb->end] = *element;
		cb->end = (cb->end + 1)%cb->size;
		return 0;
	}
}

static inline void circularbRead(Circular_Buffer *cb, char *element) {
	*element = cb->buffer[cb->start];
	cb->start = (cb->start + 1) % cb->size;
}

#endif /* _CIRCULAR_BUFFER */
