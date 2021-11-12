#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <iterator>
#include <vector>
#include <stack>
#include <unistd.h>
#include <string>

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

#define R0      32768
#define R1      32769
#define R2      32770
#define R3      32771
#define R4      32772
#define R5      32773
#define R6      32774
#define R7      32775

union bytes_to_u16
{
    struct
    {
        char low;
        char high;
    } bytes;
    uint16_t u16;
};

class RAM
{
    public:
        
        
        int read_memory(const char* filepath)
        {
            std::ifstream input(filepath, std::ios::in | std::ios::binary);

            program = new uint16_t[32768];
            /*int index = 0;
            for(auto i = 1; i < program_size; i += 2) {
                char c1 = buffer[i-1];
                char c2 = buffer[i];
                uint16_t byte = c1 | (c2 << 8);
                program[index] = byte;
                index++;
            }*/
            size_t index = 0;
            char bytes[2];
            bytes_to_u16 u;
            while(input.read(bytes, 2)) {
                u.bytes.low = bytes[0];
                u.bytes.high = bytes[1];
                program[index] = u.u16;
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
                //Simulate slower CPU for the sake of debugging
                //usleep(1000);
                uint16_t op = ram->read_program(cursor);
                switch (op)
                {
                    case HALT:
                        // HALT
                        printf("\nHALT\n");
                        exit(0);
                        cursor++;
                        break;
                    case SET:
                        set_register(ram->read_program(cursor + 1), ram->read_program(cursor + 2));
                        cursor += 3;
                        break;
                    case PUSH:
                        push_to_stack(ram->read_program(cursor + 1));
                        cursor += 2;
                        break;
                    case POP:
                        if(stack.empty()) {
                            printf("ERROR\nStack is empty\n");
                            exit(-1);
                        }
                        pop(cursor, stack.top());
                        stack.pop();
                        cursor += 2;
                        break;
                    case EQ:
                        compare(EQ, cursor);
                        cursor += 4;
                        break;
                    case GT:
                        compare(GT, cursor);
                        cursor += 4;
                        break;
                    case JMP:
                        cursor = perform_jmp(cursor);
                        break;
                    case JT:
                        if(check_zero(JT, cursor)) {
                            cursor++;
                            cursor = perform_jmp(cursor);
                            break;
                        }
                        cursor += 3;
                        break;
                    case JF:
                        if(check_zero(JF, cursor)) {
                            cursor++;
                            cursor = perform_jmp(cursor);
                            break;
                        }
                        cursor += 3;
                        break;
                    case ADD:
                        add(cursor);
                        cursor += 4;
                        break;
                    case MULT:
                        multiply(cursor);
                        cursor += 4;
                        break;
                    case MOD:
                        mod(cursor);
                        cursor += 4;
                        break;
                    case AND:
                        bitwise_op(AND, cursor);
                        cursor += 4;
                        break;
                    case OR:
                        bitwise_op(OR, cursor);
                        cursor += 4;
                        break;
                    case NOT:
                        bitwise_op(NOT, cursor);
                        cursor += 3;
                        break;
                    case RMEM:
                        mem_op(RMEM, cursor);
                        cursor += 3;
                        break;
                    case WMEM:
                        mem_op(WMEM, cursor);
                        cursor += 3;
                        break;
                    case CALL:
                        call_op(cursor);
                        cursor = perform_jmp(cursor);
                        break;
                    case RET:
                        cursor = ret_op(cursor);
                        break;
                    case OUT:
                        // OUT
                        cursor++;
                        handle_print(ram->read_program(cursor));
                        cursor++;
                        break;
                    case IN:
                        in_op(cursor);
                        cursor += 2;
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
        std::stack<uint16_t> stack;
        std::string saved_out = "";
        void handle_print(uint16_t val)
        {
            is_register(&val);
            char low = val & 0xFF;
            char high = val >> 8;
            std::cout << low;
            saved_out += low;
            if(low == '\n') {
                size_t found = saved_out.find("billion");
                if(found != std::string::npos) {
                        int x = 0;
                }
                saved_out = "";
            }
            /*if(low == '!' || low == '.')
                std::cout << std::endl;*/
            //printf("%c\n", low);
            /*if(isprint(low))
                std::cout << low;
            if(isprint(high))
                printf("%c", high);*/
        }

        void multiply(uint16_t i) 
        {
            uint16_t addr = ram->read_program(i + 1);
            uint16_t val1 = read_ram(i + 2);
            uint16_t val2 = read_ram(i + 3);

            uint16_t val3 = ((int)val1 * (int)val2) % 32768;
            if(!is_valid(val3))
                return;
            is_register(&val3);
            set_register(addr, val3);
        }
        void add(uint16_t i)
        {
            uint16_t addr = ram->read_program(i + 1);
            uint16_t val1 = read_ram(i + 2);
            uint16_t val2 = read_ram(i + 3);
            
            uint16_t val3 = ((int)val1 + (int)val2) % 32768;
            if(!is_valid(val3))
                return;
            is_register(&val3);
            set_register(addr, val3);
        }
        void mod(uint16_t i)
        {
            uint16_t addr = ram->read_program(i + 1);
            uint16_t val1 = read_ram(i + 2);
            uint16_t val2 = read_ram(i + 3);
            
            uint16_t val3 = ((int)val1 % (int)val2);
            if(!is_valid(val3))
                return;
            is_register(&val3);
            set_register(addr, val3);
        }
        int perform_jmp(int i)
        {
            int check = read_ram(i);
            int n = read_ram(i + 1);
            if(is_valid(n)) {
                //printf("jmp to %i", n + 1);
                //std::cout << "jmp to " << n << std::endl;
                //n++;
                return n;
            } else {
                i += 2;
                return i;
            }
                
        }
        void set_register(uint16_t reg, uint16_t val)
        {
            if(!is_valid(reg))
                return;
            is_register(&val);
            switch(reg) {
                case R0:
                    registers[0] = val;
                    break;
                case R1:
                    registers[1] = val;
                    break;
                case R2:
                    registers[2] = val;
                    break;
                case R3:
                    registers[3] = val;
                    break;
                case R4:
                    registers[4] = val;
                    break;
                case R5:
                    registers[5] = val;
                    break;
                case R6:
                    registers[6] = val;
                    break;
                case R7:
                    registers[7] = val;
                    break;
                default:
                    return;
            }
        }
        uint16_t get_register(uint16_t reg)
        {
            if(!is_valid(reg))
                return -1;
            switch(reg) {
                case R0:
                    return registers[0];
                    break;
                case R1:
                    return registers[1];
                    break;
                case R2:
                    return registers[2];
                    break;
                case R3:
                    return registers[3];
                    break;
                case R4:
                    return registers[4];
                    break;
                case R5:
                    return registers[5];
                    break;
                case R6:
                    return registers[6];
                    break;
                case R7:
                    return registers[7];
                    break;
                default:
                    return -1;
            }
        }
        void push_to_stack(uint16_t val) 
        {
            if(is_valid(val)) {
                is_register(&val);
                stack.push(val);
            }
        }
        void pop(uint16_t cursor, uint16_t val)
        {
            uint16_t addr = ram->read_program(cursor + 1);
            is_register(&val);
            set_register(addr, val);
        }
        // Checks
        void compare(int cmp, uint16_t cursor)
        {
            uint16_t pos = ram->read_program(cursor + 1);
            uint16_t val1 = read_ram(cursor + 2);
            uint16_t val2 = read_ram(cursor + 3);
            bool cmp_val = false;

            switch(cmp) {
                case EQ:
                    cmp_val = val1 == val2;
                    break;
                case GT:
                    cmp_val = val1 > val2;
                    break;
            }
            if(cmp_val) {
                set_register(pos, 1);
            } else {
                set_register(pos, 0);
            }
        }
        void bitwise_op(int bwise, uint16_t cursor)
        {
            uint16_t pos = ram->read_program(cursor + 1);
            uint16_t val1 = read_ram(cursor + 2);
            uint16_t val2 = read_ram(cursor + 3);
            uint16_t val3 = 0;
            switch (bwise)
            {
            case OR:
                val3 = val1 | val2;
                is_register(&val3);
                set_register(pos, val3);
                break;
            case AND:
                val3 = val1 & val2;
                is_register(&val3);
                set_register(pos, val3);
                break;
            case NOT:
                val3 = (~val1) & 0x7fff;
                is_register(&val3);
                set_register(pos, val3);
                break;
            default:
                break;
            }
        }

        bool check_zero(int z, uint16_t cursor)
        {
            uint16_t val = read_ram(cursor + 1);
            switch(z) {
                case JT:
                    if(val > 0) return true;
                    break;
                case JF:
                    if(val == 0) return true;
                    break;
            }
            return false;
        }

        void mem_op(int o, uint16_t cursor)
        {
            uint16_t a = ram->read_program(cursor + 1);
            uint16_t b_addr = ram->read_program(cursor + 2);
            uint16_t b = read_ram(b_addr);
            switch(o) {
                case RMEM:
                    set_register(a, b);
                    break;
                case WMEM:
                    b = read_ram(cursor + 2);
                    a = read_ram(cursor + 1);
                    ram->modify_program(a, b);
                    break;
            }

        }

        void call_op(uint16_t cursor)
        {
            uint16_t addr = cursor + 2;
            stack.push(addr);
        }

        uint16_t ret_op(uint16_t cursor)
        {
            if(stack.empty())
                exit(1);
            uint16_t val = stack.top();
            stack.pop();
            return val;
        }

        void in_op(uint16_t cursor)
        {
            uint16_t in = (uint16_t)getchar();
            if(in == (uint16_t)'x') {
                std::string rvs = "";
                uint16_t reg_val = 0;
                std::cin >> rvs;
                reg_val = std::stoi(rvs);
                set_register(R7, reg_val);
            }
            set_register(ram->read_program(cursor + 1), in);
        }

        bool is_valid(uint16_t x)
        {
            if(x < 32776)
                return true;
            exit(-1);
        }
        void is_register(uint16_t* x) 
        {
            if(*x >= 32768 && *x <= 32775) {
                *x = get_register(*x);
            }
        }

        bool check_is_register(uint16_t x)
        {
            return x >= 32768 && x <= 32775;
        }
        
        uint16_t read_ram(uint16_t pos)
        {
            uint16_t val = 0;
            is_register(&pos);
            val = ram->read_program(pos);
            is_register(&val);
            return val;
        }
};

int main()
{
    RAM* ram = new RAM;
    ram->read_memory("challenge.bin");
    CPU* cpu = new CPU(ram);
    cpu->run_program();
}