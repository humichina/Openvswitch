#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#define DUR_TIME 100 //ms

int main( int argc, char *argv[])
{
	int fd;
	char *start;
	struct stat sb;

	if (argc == 1) {
		printf("enter with the host number!\n");
		return;
	}
	int i=atoi(argv[1]),j;
	struct timeval tv;
    struct timezone tz;
	float curr_time,pre_time,dur_time;

	fd = open("vi.txt", O_RDONLY); /* 打开vi.txt */
	fstat(fd, &sb); /* 取得文件大小 */
	start = mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);
	if(start == MAP_FAILED) /* 判断是否映射成功 */
		return;

	while (1) {
		gettimeofday (&tv, &tz);
    	pre_time = tv.tv_sec*1000000+tv.tv_usec;
		char ta[7];
		float fc;
		for (j = 1; j <= 16; j++) {
			strncpy( ta, start+(i-1)*129+(j-1)*8, 7);
			fc = atof(ta);
			if (!(fc < 0.01)) {
				//printf("%s\n", ta);
				char cmd[300] = "tc class change dev h";
				strcat(cmd,argv[1]);
				strcat(cmd,"-eth0 classid 1:");
				char ptr[2];
				gcvt(j,2,ptr);
				strcat(cmd,ptr);
				strcat(cmd," cbq bandwidth 1000Mbit rate ");
				strcat(cmd,ta);
				strcat(cmd,"Mbit maxburst 20 allot 1514 prio 3 avpkt 1000 cell 8 weight 1Mbit split 1:0 bounded");
				system(cmd);

				//printf("%f\n", fc);
			}
		}
		gettimeofday (&tv, &tz);
        curr_time = tv.tv_sec*1000000+tv.tv_usec;

		usleep(DUR_TIME*1000-(curr_time - pre_time)); //time um
	}
	
	//printf("%s", start);
	munmap(start, sb.st_size); /* 解除映射 */
	close(fd);
}
