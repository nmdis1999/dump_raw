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

static char *cmd_name = NULL;

static uint32_t map_base = 0x18000000;
static uint32_t map_size = 0x08000000;

static uint32_t map_addr = 0x00000000;


static char *dev_mem = "/dev/mem";

static bool out_buf = false;


int main(int argc, char *argv[]) {
    extern int optind; /* set by getopt to the index of the next element of the argv array to be processed */
    extern char *optarg; /* set by getopt to point at the value of the option argument */

    cmd_name = argv[0];

    int c, err_flag = 0;

#define    OPTIONS "hd:B:S"

    cmd_name = argv[0];
    while ((c = getopt(argc, argv, OPTIONS)) != EOF) {
        switch (c) {
            case 'h':
                fprintf(stderr,
                        "options are:\n"
                        "-h        print this help message\n"
                        "-d        dump buffer data\n"
                        "-B <val>  memory mapping base\n"
                        "-S <val>  memory mapping size\n", cmd_name);
                exit(0);
                break;

            case 'r':
                out_buf = true;
                break;
            case 'B' :
                map_base = strtoll(optarg, NULL, 16); /* produces a number of type long long int */
            case 'S' :
                map_size = strtoll(optarg, NULL, 16);
            default:
                err_flag++;
                break;
        }
    }

    /* If no option is matched print this message */
    if (err_flag) {
        fprintf(stderr,
                "Usage: %s -[" OPTIONS "] [file]\n"
                "%s -h for help.\n",
                cmd_name, cmd_name);
        exit(2);
    }

    /* Opening dev/mem with read/write permission */
    int fd = open(dev_mem, O_RDWR | O_SYNC);
    if (fd == -1) {
        fprintf(stderr,
                "error opening >%s<.\n%s\n",
                dev_mem, strerror(errno));
        exit(1);
    }

    /* Mapping frane buffer */
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

    uint64_t *dp = (uint64_t *) (map_addr);  /* Data pointer */
    size_t ds = 0x1200100;    /* Hex notation for 18MB file i.e 18,000,000 bytes */

    /* Starting to write image */
    if (out_buf) {
        fprintf(stderr, "dumping buffer data ...\n");
        fprintf(stderr, "%p %ld\n", dp, ds);

        while (ds > 0) {
            ssize_t cnt = write(1, dp, ds); /* 1 is for standard output, dp is pointer to buf of ds bytes, and ds is number of
            bytes */

            fprintf(stderr, "%p %ld -> %ld\n", dp, ds, cnt);
            dp = (uint64_t *) (((char *) dp) + cnt);
            ds -= cnt;
        }

    }

    printf("%u\n", map_base);

    exit((err_flag) ? 1 : 0);
}
