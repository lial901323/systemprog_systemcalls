#include <unistd.h>     
#include <fcntl.h>      
#include <sys/stat.h>   
#include <string.h>     

#define BUF_SIZE 8192   // Buffer size used for copying data in chunks

/*
 * This function writes exactly 'count' bytes to the given file descriptor.
 * It handles the case where write() writes fewer bytes than requested.
 */
static int write_all(int fd, const char *buf, ssize_t count) {
    ssize_t written = 0;

    while (written < count) {
        ssize_t w = write(fd, buf + written, count - written);
        if (w < 0) {
            return -1;
        }
        written += w;
    }
    return 0;
}

/*
 * Writes a string to the given file descriptor.
 * Used for printing messages to stdout or stderr.
 */
static void write_str(int fd, const char *msg) {
    write_all(fd, msg, (ssize_t)strlen(msg));
}

// Checks if a file already exists.
static int file_exists(const char *path) {
    int fd = open(path, O_RDONLY);
    if (fd >= 0) {
        close(fd);
        return 1;
    }
    return 0;
}

/*
 * Asks the user whether to overwrite the destination file.
 * The function keeps asking until the user enters 'y' or 'n'.
 * Returns:
 * 1 - user chose to overwrite the file
 * 0 - user chose not to overwrite the file
 */
static int ask_overwrite(const char *dst) {
    (void)dst;   // Parameter not used, prevents unused-parameter warning
    char c;

    while (1) {
        write_str(STDOUT_FILENO,
                  "The destination file already exists.\n"
                  "Overwriting it will erase its contents.\n"
                  "Do you want to continue? (y/n)\n");

        if (read(STDIN_FILENO, &c, 1) <= 0) {
            return 0;
        }

        if (c == 'y' || c == 'Y') {
            return 1;
        }
        if (c == 'n' || c == 'N') {
            return 0;
        }

        write_str(STDOUT_FILENO,
                  "Invalid input. Please enter 'y' or 'n'.\n");
    }
}

/*
 * Copies data from the source file to the destination file.
 * Data is copied using a fixed-size buffer until end-of-file is reached.
 */
static int copy_file(int src_fd, int dst_fd) {
    char buffer[BUF_SIZE];
    ssize_t bytes_read;

    while ((bytes_read = read(src_fd, buffer, sizeof(buffer))) > 0) {
        if (write_all(dst_fd, buffer, bytes_read) < 0) {
            write_str(STDERR_FILENO, "Error: write failed\n");
            return -1;
        }
    }

    if (bytes_read < 0) {
        write_str(STDERR_FILENO, "Error: read failed\n");
        return -1;
    }

    return 0;
}

int main(int argc, char *argv[]) {
    //Check that exactly two arguments were provided 
    if (argc != 3) {
        write_str(STDERR_FILENO,
                  "Usage: ./my_copy <source_file> <dest_file>\n");
        return 1;
    }

    const char *src = argv[1];
    const char *dst = argv[2];

    //Open the source file for reading 
    int src_fd = open(src, O_RDONLY);
    if (src_fd < 0) {
        write_str(STDERR_FILENO,
                  "Error: source file does not exist or cannot be read\n");
        return 1;
    }

    // If the destination file exists, ask the user whether to overwrite it 
    if (file_exists(dst)) {
        if (!ask_overwrite(dst)) {
            write_str(STDOUT_FILENO,
                      "Copy operation canceled by the user.\n");
            close(src_fd);
            return 0;
        }
    }

    // Open (or create) the destination file for writing 
    int dst_fd = open(dst, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dst_fd < 0) {
        write_str(STDERR_FILENO,
                  "Error: cannot open destination file\n");
        close(src_fd);
        return 1;
    }

    //Perform the file copy 
    if (copy_file(src_fd, dst_fd) < 0) {
        close(src_fd);
        close(dst_fd);
        return 1;
    }

    //Close both files 
    close(src_fd);
    close(dst_fd);

    return 0;
}
