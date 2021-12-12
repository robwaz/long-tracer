#ifndef PARSE_EVENT
#define PARSE_EVENT


#include <linux/perf_event.h>
#include <linux/hw_breakpoint.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

struct parse_info {
    struct perf_event_mmap_page *header;
    struct perf_event_header *data;
    void *aux;
    uint64_t data_read_count;
    uint64_t aux_read_count;
    void *cur_data;
    void *cur_aux;
    int aux_data_fd;
} typedef parse_info;


void parse_event(parse_info *parse_info);
void parse_type(parse_info *parse_info);

#endif
