
#include <linux/ioctl.h>

typedef struct{
	long delay;
	long queue_buffer_size;
} query_arg_t;

#define DELAY_GET _IOR('q',1,query_arg_t *)
#define DELAY_SET _IO('q',2)
#define DELAY_CLR _IOW('q',3,query_arg_t *)

