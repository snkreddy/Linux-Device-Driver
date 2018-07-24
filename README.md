## Linux Device Driver With Non Blocking Read/Write
[Software Requirement Specification Document](./SRS.pdf) <br />
[Design Document](./Design Document.pdf) <br />
[Device Driver Results](./Device Driver Results.pdf) <br />
*The Github Repo consists of the following files*:

- `Makefile` :  to load the file into kernel devices.
- `iitpipe.c`: this is the device file the loads 
- `write.c`  : code to write in to the device.
- `read.c`   : code to read from the device.
- `ioctl.c`  : code to control the delay in device read write 
- `queue.h`  : This is the header file to maintain the Buffer as queue data structure 



