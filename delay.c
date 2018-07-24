#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>


#include "ioctl.h"

void get_vars(int fd)
{
    query_arg_t q;

    if (ioctl(fd, DELAY_GET, &q) == -1)
    {
        perror("DELAY_apps ioctl get");
    }
    else
    {
        printf("delay : %ld\n", q.delay);
        printf("present no of characters in read_buffer_size: %ld\n", q.queue_buffer_size);
    }
}
void clr_vars(int fd)
{
    if (ioctl(fd, DELAY_CLR) == -1)
    {
        perror("DELAY_apps ioctl clr");
    }
}
void set_vars(int fd)
{
    long v;
    query_arg_t q;

    printf("Enter delay: ");
    scanf("%ld", &v);
    getchar();
    q.delay = v;
    printf("Enter queue_buffer_size: ");
    scanf("%ld", &v);
    getchar();
    q.queue_buffer_size = v;

    if (ioctl(fd, DELAY_SET, &q) == -1)
    {
        perror("DELAY_apps ioctl set");
    }
}

int main(int argc, char *argv[])
{
    char *file_name = "/dev/iitpipe_ioctl";
    int fd;
    enum
    {
        e_get,
        e_clr,
        e_set
    } option;

    if (argc == 1)
    {
        option = e_get;
    }
    else if (argc == 2)
    {
        if (strcmp(argv[1], "-g") == 0)
        {
            option = e_get;
        }
        else if (strcmp(argv[1], "-c") == 0)
        {
            option = e_clr;
        }
        else if (strcmp(argv[1], "-s") == 0)
        {
            option = e_set;
        }
        else
        {
            fprintf(stderr, "Usage: %s [-g | -c | -s]\n", argv[0]);
            return 1;
        }
    }
    else
    {
        fprintf(stderr, "Usage: %s [-g | -c | -s]\n", argv[0]);
        return 1;
    }
    fd = open(file_name, O_RDWR);
    if (fd == -1)
    {
        perror("DELAY_apps open");
        return 2;
    }

    switch (option)
    {
        case e_get:
            get_vars(fd);
            break;
        case e_clr:
            clr_vars(fd);
            break;
        case e_set:
            set_vars(fd);
            break;
        default:
            break;
    }

    close (fd);

    return 0;
}
