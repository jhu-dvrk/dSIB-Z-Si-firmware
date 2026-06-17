#include <errno.h>
#include <stdint.h>
#include <sys/stat.h>
#include <unistd.h>

extern unsigned char _end;
static unsigned char *heap_end;

caddr_t _sbrk(int incr)
{
  if (heap_end == 0) {
    heap_end = &_end;
  }
  unsigned char *prev = heap_end;
  heap_end += incr;
  return (caddr_t)prev;
}

int _write(int file, char *ptr, int len)
{
  (void)file;
  (void)ptr;
  return len;
}

int _close(int file)
{
  (void)file;
  return 0;
}

int _fstat(int file, struct stat *st)
{
  (void)file;
  st->st_mode = S_IFCHR;
  return 0;
}

int _isatty(int file)
{
  (void)file;
  return 1;
}

off_t _lseek(int file, off_t ptr, int dir)
{
  (void)file;
  (void)ptr;
  (void)dir;
  return 0;
}

int _read(int file, char *ptr, int len)
{
  (void)file;
  (void)ptr;
  (void)len;
  return 0;
}

void _exit(int status)
{
  (void)status;
  for (;;) {
  }
}

int _getpid(void)
{
  return 1;
}

int _kill(int pid, int sig)
{
  (void)pid;
  (void)sig;
  errno = EINVAL;
  return -1;
}

void _init(void) {}
void _fini(void) {}
