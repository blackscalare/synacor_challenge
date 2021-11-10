#include "memory.h"
#include <fstream>

long get_filesize(const char* filepath)
{
    struct stat stat_buf;
    int rc = stat(filepath, &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}

int read_memory(const char* filepath)
{
    long filesize = get_filesize(filepath);
    printf("%ld", filesize);
    return 1;
}

