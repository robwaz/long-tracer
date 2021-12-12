#include "parse_event.h"
#include <stdlib.h>

void parse_event(parse_info *parse_info) {
      struct perf_event_header *cur_data = parse_info->cur_data;
      struct perf_event_mmap_page *header = parse_info->header;

      parse_type(parse_info);
      parse_info->data_read_count += cur_data->size;
      parse_info->cur_data = ((char *) parse_info->data) + (parse_info->data_read_count % parse_info->header->data_size);
}

void read_aux(parse_info *parse_info, uint64_t aux_offset, uint64_t aux_size) {
  int wrote = write(parse_info->aux_data_fd, parse_info->aux + (aux_offset % parse_info->header->aux_size), aux_size);
  if (wrote < 0) {
    perror("Writing aux file failed");
    exit(1);
  }
}

void print_aux_info(parse_info *parse_info) {
  struct perf_event_header *cur_data = parse_info->cur_data + sizeof(struct perf_event_header);
  void *aux = parse_info->aux;
  uint64_t new_tail = parse_info->header->aux_head;

  parse_info->cur_aux = ((char *) parse_info->aux) + (parse_info->aux_read_count % parse_info->header->aux_size);

  uint64_t *ptr = (uint64_t *) (cur_data);
  uint64_t aux_offset = *ptr;
  ptr++;
  uint64_t aux_size = *ptr;
  ptr++;
  uint64_t flags = *ptr;

  if (flags & PERF_AUX_FLAG_TRUNCATED) {
    printf("Failure: flags PERF_AUX_FLAG_TRUNCATED\n");
    exit(3);
  }
  // This shouldn't occur 
  if (flags & PERF_AUX_FLAG_OVERWRITE) {
    printf("Failure: flags PERF_AUX_FLAG_OVERWRITE\n");
    exit(3);
  }

  while(parse_info->aux_read_count < new_tail) {
    read_aux(parse_info, aux_offset, aux_size);
    parse_info->aux_read_count += aux_size;
  }
  parse_info->header->aux_tail = new_tail;
}


void parse_type(parse_info *parse_info) {
  struct perf_event_header *data = (struct perf_event_header *) parse_info->cur_data;
  struct perf_event_mmap_page *header = parse_info->header;

  uint64_t new_tail = header->aux_head;
  if (data->type == PERF_RECORD_MMAP) {
    printf("PERF_RECORD_MMAP\n");
  }
  else if (data->type == PERF_RECORD_LOST) {
    printf("PERF_RECORD_LOST\n");
    print_aux_info(parse_info);
  }
  else if (data->type == PERF_RECORD_COMM) {
    printf("PERF_RECORD_COMM\n");
  }
  else if (data->type == PERF_RECORD_EXIT) {
    printf("PERF_RECORD_EXIT\n");
  }
  else if (data->type == PERF_RECORD_THROTTLE) {
    printf("PERF_RECORD_THROTTLE\n");
  }
  else if (data->type == PERF_RECORD_UNTHROTTLE) {
    printf("PERF_RECORD_UNTHROTTLE\n");
  }
  else if (data->type == PERF_RECORD_FORK) {
    printf("PERF_RECORD_FORK\n");
  }
  else if (data->type == PERF_RECORD_READ) {
    printf("PERF_RECORD_READ\n");
  }
  else if (data->type == PERF_RECORD_SAMPLE) {
    printf("PERF_RECORD_SAMPLE\n");
  }
  else if (data->type == PERF_RECORD_MMAP2) {
    printf("PERF_RECORD_MMAP2\n");
  }
  else if (data->type == PERF_RECORD_AUX) {
    //printf("PERF_RECORD_AUX\n");
    print_aux_info(parse_info);
  }
  else if (data->type == PERF_RECORD_ITRACE_START) {
    printf("PERF_RECORD_ITRACE_START\n");
  }
  else if (data->type == PERF_RECORD_LOST_SAMPLES) {
    printf("PERF_RECORD_LOST_SAMPLES\n");
  }
  else if (data->type == PERF_RECORD_SWITCH) {
    printf("PERF_RECORD_SWITCH\n");
  }
  else if (data->type == PERF_RECORD_SWITCH_CPU_WIDE) {
    printf("PERF_RECORD_SWITCH_CPU_WIDE\n");
  }
}
