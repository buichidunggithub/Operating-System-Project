#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>
 
int main(){
   int ret, fd;
     int randnum;
   fd = open("/dev/RandomNumberDevice", O_RDWR);//Open the device with read/write access
   if (fd < 0){
      perror("Opening failed");
      return errno;
   }

  //printf("Reading from the device.\n");	

   ret = read(fd, &randnum, sizeof(int)); 			//Read the reponse from the LKM
   if (ret < 0){
      perror("Reading failed.");
      return errno;
   }
   printf("The randomly number is: [%d]\n", randnum);
   return 0;
}
