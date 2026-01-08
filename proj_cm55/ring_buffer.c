/*
 * ring_buffer.c
 *
 *  Created on: Nov 28, 2025
 *      Author: ROJ030
 */


#include "ring_buffer.h"

#include <stdlib.h>
#include <string.h>

typedef struct
{
	uint8_t* buffer;
	int read_index;
	int write_index;
	int free_memory;
	int used_memory;
	int total_size;
} ring_buffer_t;

static ring_buffer_t ring_buffer;

int ring_buffer_init(size_t size)
{
	ring_buffer.buffer = malloc(size);
	if (ring_buffer.buffer == NULL)
	{
		return -1;
	}

	ring_buffer.read_index = 0;
	ring_buffer.write_index = 0;
	ring_buffer.free_memory = size;
	ring_buffer.used_memory = 0;
	ring_buffer.total_size = size;

	return 0;
}

int ring_buffer_enqueue(uint8_t* buffer, size_t size)
{
	// Enough size?
	if (size > ring_buffer.free_memory) return -1;

	// Enqueue
	// One or 2 memcpy?
	if (size > (ring_buffer.total_size - ring_buffer.write_index))
	{
		// 2 memcpy needed
		int first_chunk_size = (ring_buffer.total_size - ring_buffer.write_index);
		int second_chunk_size = size - first_chunk_size;

		uint8_t* write_addr = ring_buffer.buffer + ring_buffer.write_index;
		memcpy(write_addr, buffer, first_chunk_size);

		uint8_t* second_chunk_addr = buffer + first_chunk_size;
		memcpy(ring_buffer.buffer, second_chunk_addr, second_chunk_size);

		ring_buffer.write_index = second_chunk_size;
	}
	else
	{
		// Only 1 needed!
		uint8_t* write_addr = ring_buffer.buffer + ring_buffer.write_index;
		memcpy(write_addr, buffer, size);
		ring_buffer.write_index += size;
	}

	if (ring_buffer.write_index >= ring_buffer.total_size)
		ring_buffer.write_index = 0;

	ring_buffer.used_memory += size;
	ring_buffer.free_memory -= size;

	return 0;
}

int ring_buffer_get_used_memory()
{
	return ring_buffer.used_memory;
}

int ring_buffer_get_free_memory()
{
	return ring_buffer.free_memory;
}

int ring_buffer_dequeue(uint8_t* buffer, size_t size)
{
	// Enough size?
	if (size > ring_buffer.used_memory) return -1;

	// Enqueue
	// One or 2 memcpy?
	if (size > (ring_buffer.total_size - ring_buffer.read_index))
	{
		// 2 memcpy needed
		int first_chunk_size = (ring_buffer.total_size - ring_buffer.read_index);
		int second_chunk_size = size - first_chunk_size;

		uint8_t* read_addr = ring_buffer.buffer + ring_buffer.read_index;
		memcpy(buffer, read_addr, first_chunk_size);

		uint8_t* second_chunk_addr = buffer + first_chunk_size;
		memcpy(second_chunk_addr, ring_buffer.buffer, second_chunk_size);

		ring_buffer.read_index = second_chunk_size;
	}
	else
	{
		// Only 1 needed!
		uint8_t* read_addr = ring_buffer.buffer + ring_buffer.read_index;
		memcpy(buffer, read_addr, size);
		ring_buffer.read_index += size;
	}

	if (ring_buffer.read_index >= ring_buffer.total_size)
		ring_buffer.read_index = 0;

	ring_buffer.used_memory -= size;
	ring_buffer.free_memory += size;

	return 0;
}
