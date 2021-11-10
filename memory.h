#pragma once
#include <sys/stat.h>
class Memory {
    public:
        int read_memory(const char* filepath);
    private:
        long get_filesize(const char* filepath);
};