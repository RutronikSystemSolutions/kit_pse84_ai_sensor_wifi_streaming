/*
 * ring_buffer.h
 *
 *  Created on: Nov 28, 2025
 *      Author: ROJ030
 */

#ifndef RING_BUFFER_H_
#define RING_BUFFER_H_

#include <stdint.h>
#include <stdlib.h>

int ring_buffer_init(size_t size);

int ring_buffer_enqueue(uint8_t* buffer, size_t size);

int ring_buffer_get_used_memory();

int ring_buffer_get_free_memory();

int ring_buffer_dequeue(uint8_t* buffer, size_t size);


#endif /* RING_BUFFER_H_ */
