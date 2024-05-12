/*
        Stav Macri 324084722 - סתיו מכרי
        Nadav Swartz 208296400 - נדב שוורץ
*/

#include <stdio.h>
#include <iostream>
#include <sys/stat.h>
#include <stdlib.h>


int compareFiles(const char* file1, const char* file2)
{
    int pos = 0;
    int fd1 = open(file1, "rb");
    int fd2 = open(file2, "rb");

    struct stat File1;
    if (fstat(fd1, &File1)) {
        perror("There is an error");
        close(fd1);
    }

    struct stat File2;
    if (fstat(fd2, &File2)) {
        perror("There is an error");
        close(fd2);
    }

    char* buffer1 = (char*)malloc(File1.st_size + 1);
    char* buffer2 = (char*)malloc(File2.st_size + 1);

    int read1 = read(fd1, buffer1, File1.st_size);
    int read2 = read(fd2, buffer2, File2.st_size);

    buffer1[read1] = '\0';
    buffer2[read2] = '\0';

    if (File1.st_size != File2.st_size)
    {
        free(buffer1);
        free(buffer2);
        close(fd1);
        close(fd2);
        return 1;
    }
    while (buffer1[pos] != '\0' && buffer2[pos] != '\0')
    {
        if (buffer1[pos] == buffer2[pos])
        {
            ++pos;
        }
        else
        {
            free(buffer1);
            free(buffer2);
            close(fd1);
            close(fd2);
            return 1;
        }
    }
    free(buffer1);
    free(buffer2);
    close(fd1);
    close(fd2);
    return 2;
}


int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf(stderr, "Usage: %s <file1> <file2>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int result = compareFiles(argv[1], argv[2]);

    if (result == 1) {
        printf("Files are different.\n");
        return 1;
    }
    else {
        printf("files are indentical.\n");
        return 2;
    }
}