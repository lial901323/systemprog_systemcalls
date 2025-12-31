
This folder includes the source code file my_copy.c, which implements a file copy program using Linux system calls only :open
 read
 write
 close
The program receives two command-line arguments: a source file (the file to copy from) and a destination file (the file to copy to).
The source file must already exist in the working directory, while the destination file is created automatically if it does not exist or overwritten after user confirmation if it already exists.
