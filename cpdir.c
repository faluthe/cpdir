/*
* cpdir - Copy Directory's Contents to Another Directory
*
* Based on Bruce Molay's "cp1.c" from "Understanding Unix/Linux Programming"
*
* Patrick LeBlanc - Systems Programming, Professor Matthew Haas, Spring 2022
*/

#include <dirent.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFFERSIZE 4096
#define COPYMODE 0644

// Ripped from cp1.c
void oops(const char* s1, const char* s2) {
    fprintf(stderr, "Error: %s ", s1);
    perror(s2);
    exit(1);
}

int is_dir(int fd) {
    struct stat file;
    fstat(fd, &file);
    return S_ISDIR(file.st_mode);
}

void cp(const char* name, int source_fd,  int dest_fd) {
    int in_fd, out_fd, n_chars;
    char buf[BUFFERSIZE];

    // Open input file
    if ((in_fd = openat(source_fd, name, O_RDONLY)) == -1)
        oops("Cannot open ", name);
    
    // Skip if file is a directory
    if (is_dir(in_fd))
        return;

    // Create file in destination directory, see creat(3p)
    if ((out_fd = openat(dest_fd, name, O_WRONLY|O_CREAT|O_TRUNC, COPYMODE)) == -1)
        oops("Cannot open ", name);

    // Copy information
    while ((n_chars = read(in_fd, buf, BUFFERSIZE)) > 0)
        if (write(out_fd, buf, n_chars) != n_chars)
            oops("Write error to ", name);

    if (n_chars == -1)
        oops("Read error from ", name);

    // Clean up
    if (close(in_fd) == 1 || close(out_fd) == -1)
        oops("Error closing files", "");
}

int main(int argc, char* argv[]) {
    // Check arguments
    if (argc != 3)
        oops("Useage: 'cpdir source destination'", "");

    DIR* dest_dir;
    DIR* source_dir;
    struct dirent* entry;
    struct stat st;

    // Make destination folder if not found
    if (stat(argv[2], &st) == -1)
        mkdir(argv[2], 0700);
    // Open destination folder
    if ((dest_dir = opendir(argv[2])) == NULL)
        oops("Directory not opened: ", argv[2]);
    
    // Open source folder
    if ((source_dir = opendir(argv[1])) == NULL)
        oops("Directory not opened: ", argv[1]);
    else {
        // Read entries & copy files
        while ((entry = readdir(source_dir)) != NULL)
            cp(entry->d_name, dirfd(source_dir), dirfd(dest_dir));

        // Clean up
        closedir(dest_dir);
        closedir(source_dir);
    }

    return 0;
}