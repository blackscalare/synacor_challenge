#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <iterator>
#include <vector>

#define HALT    0
#define SET     1
#define PUSH    2
#define POP     3
#define EQ      4
#define GT      5
#define JMP     6
// if a is nonzero, jmp to b
#define JT      7
// if b is zero, jmp to b
#define JF      8
#define ADD     9
#define MULT    10
#define MOD     11
#define AND     12
#define OR      13
#define NOT     14
#define RMEM    15
#define WMEM    16
#define CALL    17
#define RET     18
#define OUT     19
#define IN      20
#define NOOP    21


class RAM
{
    public:
        int read_memory(const char* filepath)
        {
            long filesize = get_filesize(filepath);
            loaded_size = filesize;
            buffer = new char[filesize];
            std::ifstream input(filepath, std::ios::in | std::ios::binary);
            if(!input.read(buffer, filesize)) return -1;
            size_t size = input.tellg();
            for(auto i = 0; i < filesize; ++i) {
                char c = buffer[i];
                /*if(isprint(c))  {
                    if(c == '!' || c == '.')
                        printf("\n");
                    printf("%c", c);
                }*/   
            }
            program_size = filesize / 2;
            program = new uint16_t[program_size];
            int index = 0;
            for(auto i = 1; i < program_size; i += 2) {
                char c1 = buffer[i-1];
                char c2 = buffer[i];
                uint16_t byte = c1 | (c2 << 8);
                program[index] = byte;
                index++;
            }
            return 1;
        }

        size_t get_program_size(){return program_size;}
        uint16_t read_program(int i) {return program[i];}
        void modify_program(int i, uint16_t val)
        {
            program[i] = val;
        }

    private:
        long get_filesize(const char* filepath)
        {
            struct stat stat_buf;
            int rc = stat(filepath, &stat_buf);
            return rc == 0 ? stat_buf.st_size : -1;
        }   
    char* buffer;
    uint16_t* program;
    size_t program_size = 0;
    long loaded_size = 0;
    // Registers
    char r0;
    char r1;
    char r2;
    char r3;
    char r4;
    char r5;
    char r6;
    char r7;
};

class CPU
{
    public:
        CPU(RAM* r) {ram = r;}
        void load_ram(RAM* r) { ram = r; }
        void run_program()
        {
            size_t size = ram->get_program_size();
            bool running = true;
            size_t cursor = 0;
            while(running) {
                uint16_t op = ram->read_program(cursor);
                switch (op)
                {
                    case HALT:
                        // HALT
                        printf("\nHALT\n");
                        //exit(0);
                        cursor++;
                        break;
                    case JMP:
                        cursor = perform_jmp(cursor);
                        break;
                    case MULT:
                        multiply(cursor);
                        cursor += 4;
                        break;
                    case OUT:
                        // OUT
                        cursor++;
                        handle_print(ram->read_program(cursor));
                        cursor++;
                        break;
                    case NOOP:
                        cursor++;
                        break;
                    default:
                        cursor++;
                        break;
                }
            }
        }
            
    private:
        RAM* ram;
        std::vector<uint16_t> registers{0, 0, 0, 0, 0, 0, 0, 0};
        
        void handle_print(uint16_t val)
        {
            char low = val & 0xFF;
            char high = val >> 8;
            std::cout << low;
            if(low == '!' || low == '.')
                std::cout << std::endl;
            //printf("%c\n", low);
            /*if(isprint(low))
                std::cout << low;
            if(isprint(high))
                printf("%c", high);*/
        }
        void multiply(uint16_t i) {
            uint16_t addr = ram->read_program(i + 1);
            uint16_t val1 = ram->read_program(i + 2);
            uint16_t val2 = ram->read_program(i + 3);
            
            uint16_t val3 = ((int)val1 * (int)val2) % 32768;
            ram->modify_program(addr, val3);
        }
        int perform_jmp(int i)
        {
            int n = ram->read_program(i + 1);
            if(n < 32776) {
                //printf("jmp to %i", n + 1);
                std::cout << "jmp to " << n << std::endl;
                return n;
            } else {
                i++;
                return i;
            }
                
        }
};

int main()
{
    RAM* ram = new RAM;
    ram->read_memory("challenge.bin");
    CPU* cpu = new CPU(ram);
    cpu->run_program();
}