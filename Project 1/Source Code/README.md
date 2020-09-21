Đây là code tạo một module dùng để tạo một số nguyên 4 bytes ngẫu nhiên. Module này tạo một character device và các tiến trình ở user-space có thể open và read số ngẫu nhiên được tạo.

Code có tham khảo nguồn tại link: [http://derekmolloy.ie/writing-a-linux-kernel-module-part-2-a-character-device/]

**Các thành phần của source**
* File randNum.c để tạo module sinh số nguyên ngẫu nhiên, đồng thời tạo character device để các tiến trình khác có thể truy cập
* File testRandNum.c là chương trình ở user-space open và đọc số ngẫu nhiên được sinh ra
* Makefile để biên dịch 2 thành phần trên.

**Hướng dẫn vận hành**
1. Đặt các file Makefile, randNum.c và testRandNum.c ở cùng một thư  mục
2. Mở Terminal tại thư mục chứa các file trên, chạy lệnh make
```console
anhquoc@anhquoc:~/Documents/RandNum$ make
make -C /lib/modules/4.15.0-34-generic/build M=/home/anhquoc/Documents/RandNum modules
make[1]: Entering directory '/usr/src/linux-headers-4.15.0-34-generic'
  CC [M]  /home/anhquoc/Documents/RandNum/randNum.o
  Building modules, stage 2.
  MODPOST 1 modules
  CC      /home/anhquoc/Documents/RandNum/randNum.mod.o
  LD [M]  /home/anhquoc/Documents/RandNum/randNum.ko
make[1]: Leaving directory '/usr/src/linux-headers-4.15.0-34-generic'
cc testRandNum.c -o test
```

3. Chạy lệnh sudo insmod randNum.ko để load module này vào kernel, và dùng lệnh lsmod để kiểm tra module đã được load hay chưa
```console
anhquoc@anhquoc:~/Documents/RandNum$ sudo insmod randNum.ko
[sudo] password for anhquoc: 
anhquoc@anhquoc:~/Documents/RandNum$ lsmod
Module                  Size  Used by
randNum                16384  0
```

4. Chạy lệnh sudo ./test nhiều lần để chạy ứng dụng testRandNum và kiểm tra kết quả
```console
anhquoc@anhquoc:~/Documents/RandNum$ sudo ./test
Starting randomNumber device test ...
Reading from the device...
The number received is: [198866395]
End of the program
anhquoc@anhquoc:~/Documents/RandNum$ sudo ./test
Starting randomNumber device test ...
Reading from the device...
The number received is: [1472622461]
End of the program
anhquoc@anhquoc:~/Documents/RandNum$ sudo ./test
Starting randomNumber device test ...
Reading from the device...
The number received is: [607486130]
End of the program
```
Mỗi lần chạy ta nhận được một số khác nhau. 

