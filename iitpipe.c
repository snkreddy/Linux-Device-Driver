#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#include <linux/vmalloc.h>
#include <linux/timer.h>

#include "queue.h"
// #include "ioctl.h"
//Device 1
#define DEVICE_NAME0 "iitpipe0"
#define DEVICE_NAME1 "iitpipe1"
#define CLASS_NAME "iitpipeclass"

//Device ioctl
#define IOCTL_DEVICE_NAME "iitpipe_ioctl"


MODULE_LICENSE("GPL");
MODULE_AUTHOR("IIT Mandi System	Practicum 2018 Group 14");
MODULE_DESCRIPTION("A simple Linux driver for the BBB.");
MODULE_VERSION("0.1");

// TODO use these
static struct circ_queue input_queue[2];
static struct circ_queue output_queue[2];

// static struct circ_queue queue;

static size_t queue_size = 10000000;
// static int read_buffer_size=1024;
// static int write_buffer_size=1024;

//Decive info
static int major_num[2];
static struct class* class = NULL;
static struct device* device[2] = {NULL, NULL};

//Device ioctl info
static int ioctl_major_num;
static struct device* ioctl_device = NULL;
static long present_size = 0;

//Devices file operarions delacations

static int     open(struct inode *, struct file *);
static int     release(struct inode *, struct file *);
static ssize_t read(struct file *, char *, size_t, loff_t *);
static ssize_t write(struct file *, const char *, size_t, loff_t *);

//ioctl file declarations
static int ioctl_open(struct inode *i, struct file *f);
static int ioctl_close(struct inode *i, struct file *f);
static long ioctl(struct file *f,unsigned int cmd, unsigned long arg);
static long delay = 0; // in msecs

static struct timer_list timer;
static long timer_interval = 1; // interval for timer in msecs

//Devices file operations
static struct file_operations fops = {
	.open = open,
	.read = read,
	.write = write,
	.release = release
};

static struct file_operations delay_fops =
{
	.owner = THIS_MODULE,
	.open = ioctl_open,
	.release = ioctl_close,
	.unlocked_ioctl = ioctl
};

void update_queues(void)
{
	mod_timer(&timer, jiffies + msecs_to_jiffies(timer_interval));

	struct circ_queue_node node;
	int i;

	long long updated_nodes = 0;
	for (i = 0; i < 2; i++) {
		while (circ_queue_peek(&input_queue[i], &node)
		    && node.timestamp + msecs_to_jiffies(delay) <= jiffies) {
			circ_queue_pop(&input_queue[i], &node);
			circ_queue_push(&output_queue[i], node);

			updated_nodes++;
		}
	}
}

static int __init iitpipe_init(void)
{
	printk(KERN_INFO "iitpipe starting up\n");

	// input_queue = circ_queue_init(queue_size);
	// output_queue = circ_queue_init(queue_size);

	input_queue[0] = circ_queue_init(queue_size);
	input_queue[1] = circ_queue_init(queue_size);
	output_queue[0] = circ_queue_init(queue_size);
	output_queue[1] = circ_queue_init(queue_size);

	circ_queue_print(input_queue[0]);
	circ_queue_print(input_queue[1]);
	circ_queue_print(output_queue[0]);
	circ_queue_print(output_queue[1]);

	printk(KERN_INFO "initialized queues\n");

	// TODO error handling here in case these devices can't be registered

	class = class_create(THIS_MODULE, CLASS_NAME);
	major_num[0] = register_chrdev(0, DEVICE_NAME0, &fops);
	major_num[1] = register_chrdev(0, DEVICE_NAME1, &fops);
	device[0] = device_create(class, NULL, MKDEV(major_num[0], 0), NULL, DEVICE_NAME0);
	device[1] = device_create(class, NULL, MKDEV(major_num[1], 0), NULL, DEVICE_NAME1);

	ioctl_major_num = register_chrdev(0, IOCTL_DEVICE_NAME, &delay_fops);
	ioctl_device = device_create(class, NULL, MKDEV(ioctl_major_num, 0), NULL, IOCTL_DEVICE_NAME);

	printk(KERN_INFO "Registered device, class, etc.\n");

	setup_timer(&timer, update_queues, 0);
	mod_timer(&timer, jiffies + msecs_to_jiffies(timer_interval));

	return 0;
}

// read
static ssize_t read(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
	unsigned i;
	int dev_major_num = imajor(filep->f_path.dentry->d_inode);
	if (dev_major_num == major_num[0]) {
		i = 0;
	} else if (dev_major_num == major_num[1]) {
		i = 1;
	} else {
		printk(KERN_INFO "ERROR, WRONG MAJOR NUMBER!");
		return 0;
	}

	char *buffer_internal = kmalloc(len * sizeof(*buffer_internal), GFP_KERNEL);
	char *write_head = buffer_internal;

	size_t bytes_read = 0;
	while (len > 0) {
    		struct circ_queue_node node;
    		if (!circ_queue_pop(&output_queue[i], &node)) {
			break;
    		}

		*write_head = node.byte;

    		bytes_read++;
    		write_head++; len--;
	}

	copy_to_user(buffer, buffer_internal, len);

	kfree(buffer_internal);

	return bytes_read;
}

// write
static ssize_t write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
{
	unsigned i;
	int dev_major_num = imajor(filep->f_path.dentry->d_inode);
	if (dev_major_num == major_num[0]) {
		i = 0;
	} else if (dev_major_num == major_num[1]) {
		i = 1;
	} else {
		printk(KERN_INFO "ERROR, WRONG MAJOR NUMBER!");
		return 0;
	}

	char *message = kmalloc(len * sizeof(*message), GFP_KERNEL);
	copy_from_user(message, buffer, len);

	long time_current = jiffies;

	size_t bytes_written = 0;
	struct circ_queue_node node;
	while (len > 0) {
 		node.byte = message[bytes_written];
		node.timestamp = time_current;

		if (!circ_queue_push(&input_queue[i], node)) {
    			break;
		}

		bytes_written++; len--;
	}

	kfree(message);

	return bytes_written;
}

// open
static int open(struct inode *inodep, struct file *filep)
{
	printk(KERN_INFO "Opening a device\n");
	return 0;
}

// close
static int release(struct inode *inodep, struct file *filep)
{
	printk(KERN_INFO "Closing a device\n");
	return 0;
}

static int ioctl_open(struct inode *i, struct file *f)
{
	printk(KERN_INFO "Opening a ioctl_device\n");
	return 0;
}
static int ioctl_close(struct inode *i, struct file *f)
{
	printk(KERN_INFO "Closing a ioctl_device\n");
	return 0;
}

static long ioctl(struct file *f,unsigned int cmd, unsigned long arg){
	query_arg_t q;


	switch (cmd) {
	case DELAY_GET:
		q.delay = delay;
		q.queue_buffer_size = input_queue[0].size;
		if (copy_to_user((query_arg_t *)arg, &q, sizeof(query_arg_t))) {
			return -EACCES;
		}
		break;

	case DELAY_SET:
		if(copy_from_user(&q, (query_arg_t *)arg, sizeof(query_arg_t))) {
			return -EACCES;
		}
		delay = q.delay;
		input_queue[0] = circ_queue_init(q.queue_buffer_size);
		input_queue[1] = circ_queue_init(q.queue_buffer_size);
		output_queue[0] = circ_queue_init(q.queue_buffer_size);
		output_queue[1] = circ_queue_init(q.queue_buffer_size);
		queue_size = q.queue_buffer_size;
		break;

	case DELAY_CLR:
		delay = 0;
		circ_queue_free(input_queue[0]);
		circ_queue_free(input_queue[1]);
		circ_queue_free(output_queue[0]);
		circ_queue_free(output_queue[1]);
		q.queue_buffer_size = queue_size = 0;
		delay = q.delay = 0;
		if (copy_to_user((query_arg_t *)arg, &q, sizeof(query_arg_t))) {
			return -EACCES;
		}
		break;

	default:
		return -EINVAL;
	}

	return 0;
}

static void __exit iitpipe_exit(void)
{
	printk(KERN_INFO "iitpipe shutting down");

	del_timer(&timer);

	circ_queue_free(input_queue[0]);
	circ_queue_free(input_queue[1]);
	circ_queue_free(output_queue[0]);
	circ_queue_free(output_queue[1]);

	device_destroy(class, MKDEV(major_num[0], 0));
	device_destroy(class, MKDEV(major_num[1], 0));
	class_unregister(class);
	class_destroy(class);
	unregister_chrdev(major_num[0], DEVICE_NAME0);
	unregister_chrdev(major_num[1], DEVICE_NAME1);

	printk(KERN_INFO "goodbye!\n");
}

module_init(iitpipe_init);
module_exit(iitpipe_exit);
