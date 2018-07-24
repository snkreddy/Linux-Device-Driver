#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>

#define BUFFER_LENGTH 256               ///< The buffer length (crude but fine)
 
int main(int argc, char** argv)
{
	int ret, fd;
	char* stringToSend;
	 
	printf("Starting device test code example...\n");
	fd = open(argv[1], O_RDWR);    

	if (fd < 0){
		perror("Failed to open the device...");
		return errno;
	}

	printf("Type in a short string to send to the kernel module:\n");
	scanf("%m[^\n]%*c", &stringToSend);  
	printf("Writing message to the device %ld.\n", strlen(stringToSend));

	ret = write(fd, stringToSend, strlen(stringToSend));

	if (ret < 0){
		perror("Failed to write the message to the device.");
		return errno;
	}
 
	close(fd);
	 
	printf("End of the program\n");
	return 0;
}

