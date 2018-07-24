#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <pthread.h>

long long bytes_written_global[2] = {0, 0};

bool should_run = true;

void* handler(void* fds)
{
	int* fds_int = (int*) fds;
	int fdr = fds_int[0];
	int fdw = fds_int[1];
	int id = fds_int[2];

	char buffer[256];

	while(should_run) {
		long bytes_read = read(fdr, buffer, 256);
		long bytes_written = write(fdw, buffer, bytes_read);

		bytes_written_global[id] += bytes_written;
	}
}

int main()
{
	bytes_written_global[0] = 0;
	bytes_written_global[1] = 0;

	int fd0 = open("/dev/iitpipe0", O_RDWR);
	int fd1 = open("/dev/iitpipe1", O_RDWR);

	char seed[] = "Hello, world!";
	for (int i = 0; i < 10000; i++)
		write(fd0, seed, strlen(seed));
		write(fd1, seed, strlen(seed));
	
	int args0[3] = {fd0, fd1, 0};
	int args1[3] = {fd1, fd0, 1};

	pthread_t threads[2];
	pthread_create(&threads[0], NULL, handler, args0);
	pthread_create(&threads[1], NULL, handler, args1);

	int i = 0;
	while(i < 1000) {
		sleep(1);
		printf("%lld %lld\n", bytes_written_global[0], bytes_written_global[1]);

		bytes_written_global[0] = 0;
		bytes_written_global[1] = 0;

		i++;
	}

	should_run = false;

	pthread_join(threads[0], NULL);
	pthread_join(threads[1], NULL);


	return 0;
}
