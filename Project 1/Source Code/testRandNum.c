#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>
 
int main(){
   int ret, deviceFile;
   printf("Starting randomNumber device test ...\n");
   deviceFile = open("/dev/RandomNumberDevice", O_RDWR);
   if (deviceFile < 0){
      perror("Failed to open the device...");
      return errno;
   }

   printf("Reading from the device...\n");
   int x;
   ret = read(deviceFile, &x, sizeof(int)); 
   if (ret < 0){
      perror("Failed to read the number from the device.");
      return errno;
   }
   printf("The number received is: [%d]\n", x);
   printf("End of the program\n");
   return 0;
}
