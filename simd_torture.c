#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <immintrin.h>

#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)

uint32_t junk[] = {0xbaad00de, 0xbaadf00d, 0xdeaddead, 0xcafebabe,
                        0x55555555, 0x66666666, 0x77777777, 0x88888888,
                        0x99999999, 0xaaaaaaaa, 0xbbbbbbbb, 0xcccccccc,
                        0xdddddddd, 0xeeeeeeee, 0xffffffff, 0xfeedface};

uint8_t *virt_addr;

void torture_write_128bit()
{
    asm("vmovdqu %1,%%xmm0\n\t"
        "xorq %%rax,%%rax\n\t"
        "torture_loop%=:\n\t"
        "vmovdqu %%xmm0,(%0)\n\t"
        "addq $1,%%rax\n\t"
        "cmpq $999,%%rax\n\t"
        "jle torture_loop%="
        :
        : "r"(virt_addr), "m"(junk)
        : "rax", "xmm0");
}

void torture_read_128bit()
{
    asm("xorq %%rax,%%rax\n\t"
        "torture_loop%=:\n\t"
        "vmovdqu (%0),%%xmm0\n\t"
        "addq $1,%%rax\n\t"
        "cmpq $999,%%rax\n\t"
        "jle torture_loop%="
        :
        : "r"(virt_addr)
        : "rax", "xmm0");
}

void torture_write_256bit()
{
    asm("vmovdqu %1,%%ymm0\n\t"
        "xorq %%rax,%%rax\n\t"
        "torture_loop%=:\n\t"
        "vmovdqu %%ymm0,(%0)\n\t"
        "addq $1,%%rax\n\t"
        "cmpq $999,%%rax\n\t"
        "jle torture_loop%="
        :
        : "r"(virt_addr), "m"(junk)
        : "rax", "ymm0");
}

void torture_read_256bit()
{
    asm("xorq %%rax,%%rax\n\t"
        "torture_loop%=:\n\t"
        "vmovdqu (%0),%%ymm0\n\t"
        "addq $1,%%rax\n\t"
        "cmpq $999,%%rax\n\t"
        "jle torture_loop%="
        :
        : "r"(virt_addr)
        : "rax", "ymm0");
}

void torture_write_512bit()
{
    asm("vmovdqu64 %1,%%zmm0\n\t"
        "xorq %%rax,%%rax\n\t"
        "torture_loop%=:\n\t"
        "vmovdqu64 %%zmm0,(%0)\n\t"
        "addq $1,%%rax\n\t"
        "cmpq $999,%%rax\n\t"
        "jle torture_loop%="
        :
        : "r"(virt_addr), "m"(junk)
        : "rax", "zmm0");
}

void torture_read_512bit()
{
    asm("xorq %%rax,%%rax\n\t"
        "torture_loop%=:\n\t"
        "vmovdqu64 (%0),%%zmm0\n\t"
        "addq $1,%%rax\n\t"
        "cmpq $999,%%rax\n\t"
        "jle torture_loop%="
        :
        : "r"(virt_addr)
        : "rax", "zmm0");
}

int main(int argc, char **argv)
{
    int fd;
    uint8_t *map_base;
    off_t target;
    char op = 'w';
    char kind = '1';

    if (argc != 4)
    {
        fprintf(stderr, "\nUsage:\t%s 0xaddress r|w instruction_kind\n"
                        "\tinstruction_kind    : 1 - 128bit, 2 - 256bit, 3 - 512bit\n",
                argv[0]);
        exit(1);
    }
    target = strtoul(argv[1], 0, 0);
    op = tolower(argv[2][0]);
    kind = tolower(argv[3][0]);

    printf("torture target address is %p.\n", (void *)target);

    if ((fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1)
        exit(1);

    map_base = (unsigned char *)mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, target & ~MAP_MASK);
    if (map_base == (void *)-1)
        exit(1);

    printf("Memory mapped at address %p.\n", map_base);
    fflush(stdout);

    virt_addr = map_base + (target & MAP_MASK);

    if (op == 'w')
    {
        switch (kind)
        {
        case '1':
            torture_write_128bit();
            break;
        case '2':
            torture_write_256bit();
            break;
        case '3':
            torture_write_512bit();
            break;
        default:
            puts("does not exist yet");
        }
    }
    else if (op == 'r')
    {
        switch (kind)
        {
        case '1':
            torture_read_128bit();
            break;
        case '2':
            torture_read_256bit();
            break;
        case '3':
            torture_read_512bit();
            break;
        default:
            puts("does not exist yet");
        }
    }
    else
    {
        puts("invalid operation");
    }
    return 0;
}