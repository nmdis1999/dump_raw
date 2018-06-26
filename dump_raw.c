// Copyright(C) 2018 Iti Shree

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>

static char *cmd_name = NULL;

static uint32_t cmv_base = 0x60000000;
static uint32_t cmv_size = 0x00400000;
static uint32_t cmv_addr = 0x00000000;

static uint32_t map_base = 0x18000000;
static uint32_t map_size = 0x08000000;

static uint32_t map_addr = 0x00000000;


static char *dev_mem = "/dev/mem";

static bool out_buf = false;


int main(int argc, char *argv[]) {
    extern int optind;
    extern char *optarg;
    
    cmd_name = argv[0];

    out_buf = true;

    int fd = open(dev_mem, O_RDWR | O_SYNC);
    if (fd == -1) {
        fprintf(stderr,
                "error opening >%s<.\n%s\n",
                dev_mem, strerror(errno));
        exit(1);
    }

    if (cmv_addr == 0)
        cmv_addr = cmv_base;

    void *base = mmap((void *) cmv_addr, cmv_size,
                      PROT_READ | PROT_WRITE, MAP_SHARED,
                      fd, cmv_base);
    if (base == (void *) -1) {
        fprintf(stderr,
                "error mapping 0x%08lX+0x%08lX @0x%08lX.\n%s\n",
                (long) cmv_base, (long) cmv_size, (long) cmv_addr,
                strerror(errno));
        exit(2);
    } else
        cmv_addr = (long unsigned) base;

    void *buf = mmap((void *) map_addr, map_size,
                     PROT_READ | PROT_WRITE, MAP_SHARED,
                     fd, map_base);
    if (buf == (void *) -1) {
        fprintf(stderr,
                "error mapping 0x%08lX+0x%08lX @0x%08lX.\n%s\n",
                (long) map_base, (long) map_size, (long) map_addr,
                strerror(errno));
        exit(2);
    } else
        map_addr = (long unsigned) buf;

    uint64_t *dp = (uint64_t *) (map_addr);
    size_t ds = 0x121eac0;

    if (out_buf) {
        fprintf(stderr, "dumping buffer data ...\n");
        fprintf(stderr, "%p %ld\n", dp, ds);

        while (ds > 0) {
            ssize_t cnt = write(1, dp, ds);

            fprintf(stderr, "%p %ld -> %ld\n", dp, ds, cnt);
            dp = (uint64_t *) (((char *) dp) + cnt);
            ds -= cnt;
        }

    }

}