#ifndef queue_h_INCLUDED
#define queue_h_INCLUDED

#include <linux/types.h>
#include <linux/slab.h>
#include <linux/time.h>
#include "ioctl.h"

struct circ_queue_node {
	char byte;
	long timestamp; // stored in jiffies
};

struct circ_queue {
	struct circ_queue_node *buffer;
	struct circ_queue_node *begin, *end;
	size_t size;
	size_t count;
};

struct circ_queue_node *decrement_node(struct circ_queue_node *node, struct circ_queue queue)
{
	// If node is at the beginning of the buffer, go to the end of the buffer
	if (node == queue.buffer) {
		node = queue.buffer + queue.size - 1;
	} else {
    		node--;
	}

	return node;
}

void copy_node(struct circ_queue_node *to, struct circ_queue_node from)
{
	to->byte = from.byte;
	to->timestamp = from.timestamp;
}

size_t circ_queue_count(struct circ_queue queue)
{
	return queue.count;
}

void circ_queue_print(struct circ_queue queue)
{
	printk(KERN_INFO "Buffer: %llx Start: %llx End: %llx Size: %zu Capacity:%zu\n",
		queue.buffer, queue.begin, queue.end, queue.count, queue.size);
}

struct circ_queue circ_queue_init(size_t size)
{
	struct circ_queue_node *buffer = vmalloc(size * sizeof(*buffer));
	struct circ_queue queue = {
		.buffer = buffer,
		.begin = buffer + size - 1,
		.end = buffer + size - 1,
		.size = size,
		.count = 0
	};

	return queue;
}

void circ_queue_free(struct circ_queue queue)
{
	vfree(queue.buffer);
	queue.buffer = queue.begin = queue.end = NULL;
	queue.size = 0;
}

bool circ_queue_push(struct circ_queue *queue, struct circ_queue_node node)
{
    	if (circ_queue_count(*queue) == queue->size) {
		return false;
	}

	copy_node(queue->begin, node);
	queue->begin = decrement_node(queue->begin, *queue);
	queue->count++;

	return true;
}

bool circ_queue_peek(struct circ_queue *queue, struct circ_queue_node *node)
{
	if (circ_queue_count(*queue) == 0) {
		return false;
	}

	copy_node(node, *queue->end);
	return true;
}

bool circ_queue_pop(struct circ_queue *queue, struct circ_queue_node *node)
{
	if (!circ_queue_peek(queue, node)) {
		return false;
	}
	
	queue->end = decrement_node(queue->end, *queue);
	queue->count--;

	return true;
}
#endif // queue_h_INCLUDED

