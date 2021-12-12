#include <linux/perf_event.h>
#include <linux/hw_breakpoint.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <poll.h>
#include <stdint.h>

#include <signal.h>
#include "parse_event.h"

#define PAGE_SIZE 4096
#define BASE_POW 4
#define AUX_POW 12

int check_type() {
  char *type_path = "/sys/bus/event_source/devices/intel_pt/type";
  char type[2];
  int fd = open (type_path, O_RDONLY);
  if (fd < 0) {
    perror("Failed to open type path!");
    exit(1);
  }

  if (read(fd, &type, sizeof(char)) < 0) {
    perror("Failed to read type from type path!");
    exit(1);
  }
  close(fd);
  type[1] = '\x00';
  return atoi(type);
}

int check_args(int argc, char** argv) {
  if (argc < 2) {
    perror("Usage: ./tracer <program> <program args>");
    exit(1);
  }
  return 1;
}

struct perf_event_attr *get_attr() {
  struct perf_event_attr *attr = malloc(sizeof(struct perf_event_attr));
  memset(attr, 0, sizeof(struct perf_event_attr));
  attr->size = sizeof(*attr);
  attr->type = check_type();
  attr->disabled = 1;
  attr->exclude_kernel = 1;
  attr->enable_on_exec = 1;
  attr->freq = 1;
  attr->sample_freq = 1;
  attr->wakeup_watermark = 1;
  return attr;
}


int setup_tracer(pid_t pid, struct perf_event_mmap_page **header, void **aux) {
  struct perf_event_attr *attr;
  struct perf_event_mmap_page *base;
  void *data;
  int fd;
  unsigned long flags = 0;
  int cpu = -1;
  int group_fd = -1;

  attr = get_attr();
  // attr, pid, cpu, group_fd, flags
  fd = syscall(SYS_perf_event_open, attr, pid, cpu, group_fd, flags);
  if (fd < 0) {
    perror("Failed to sys_perf_event_open");
    exit(1);

  }
  
  // AUX area must be 2^m pages of size and DATA area  must be 1 + 2^n

  // 1st page is metadata page
  // 2^n is events
  // 2^n is AUX
  base = mmap(NULL, (1 + (2 << BASE_POW)) * PAGE_SIZE, PROT_WRITE, MAP_SHARED, fd, 0);
  if (base == MAP_FAILED) {
    perror("Failed to mmap tracing buffer!");
    exit(1);
  }

  *header = base;
  data = base + base->data_offset;
  base->aux_offset = base->data_offset + base->data_size;
  base->aux_size = (2 << AUX_POW) * PAGE_SIZE;

  *aux = mmap(NULL, base->aux_size, PROT_WRITE, MAP_SHARED, fd, base->aux_offset);
  if (*aux == MAP_FAILED) {
    perror("Failed to mmap aux area!");
    exit(1);
  }

  return fd;
}

int main(int argc, char** argv) {
  struct perf_event_mmap_page *header;
  int perf_fd;
  void *aux;
  struct perf_event_header *data;
  uint64_t new_tail;
  parse_info parse_info;
  int prev_hup = 0;


  if (!check_args(argc, argv)) {
    perror("Usage: ./tracer <program> <program args>");
    exit(1);
  }

  parse_info.aux_data_fd = open("./aux.data", O_CREAT | O_WRONLY, S_IRWXU);

  sem_t *start_sync = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  sem_init(start_sync, 1, 0);

  pid_t pid = fork();

  if (pid > 0) {
    perf_fd = setup_tracer(pid, &header, &aux);
    data = ((void *) header) + header->data_offset;

    parse_info.data = data;
    parse_info.header = header;
    parse_info.aux = aux;
    parse_info.data_read_count = 0;
    parse_info.aux_read_count = 0;
    parse_info.cur_aux = aux;
    parse_info.cur_data = data;

    // Makes sure we're done with setup before
    // child execs
    sem_post(start_sync);
  } else {
    // CHILD - Target
    // Wait for parent to be ready
    sem_wait(start_sync);
    execv(argv[1], &argv[1]);
  }

  struct pollfd fds;
  int timeout;
  fds.fd = perf_fd;
  fds.events = POLLIN;
  fds.revents = 0;

  while (1) {
    if (poll(&fds, 1, 10) > 0 && (fds.revents & POLLIN) || (fds.revents & POLLHUP)) {
      kill(pid, SIGSTOP);

      // POLLHIN signals tracing event
      // POLLHUP signals tracing has ceased
      if (fds.revents & POLLHUP) {
        if (!prev_hup) {
          prev_hup = 1;
        } else {
          break;
        }
      }
      new_tail = header->data_head;
      while (parse_info.data_read_count < new_tail) {
        parse_event(&parse_info); 
      }

      header->data_tail = new_tail;
      kill(pid, SIGCONT);
    }
  }

  // Cloeanup
  munmap(start_sync, sizeof(sem_t));
  munmap(parse_info.aux, parse_info.header->aux_size);
  munmap(parse_info.header, parse_info.header->data_size);
}
