#include <reent.h>
#include <errno.h>
#include <signal.h>

#define WEAK __attribute__ ((__used__, weak))
#define WEAK_INIT __attribute__ ((__used__, weak, constructor))
#define USED __attribute__ ((__used__))

#define UNUSED(x) (void)x
#define SET_ERR(e) (r->_errno = e)

#ifndef USE_SEMIHOST

WEAK void __stdio_putchar(int c);
WEAK int __stdio_getchar();
WEAK_INIT void __stdio_init();

void __stdio_init() {
}

void __stdio_putchar(int c) {
   UNUSED(c);
}

int __stdio_getchar() {
   return -1;
}

USED void _exit(int code) {
   register int __params__ __asm__("r0") = code;
   while (1)
       __asm__ __volatile__("bkpt 0");

   (void) __params__;
}

USED int _close_r(struct _reent *r, int fd) {
   UNUSED(fd);
   SET_ERR(EBADF);
   return -1;
}

USED int _execve_r(struct _reent *r, const char *f, char * const *args,
       char * const *env) {
   UNUSED(f);
   UNUSED(args);
   UNUSED(env);
   SET_ERR(ENOSYS);
   return -1;
}

USED int _fcntl_r(struct _reent *r, int fd, int cmd, int arg) {
   UNUSED(fd);
   UNUSED(cmd);
   UNUSED(arg);
   SET_ERR(ENOSYS);
   return -1;
}

USED int _fork_r(struct _reent *r) {
   SET_ERR(ENOSYS);
   return -1;
}

USED int _fstat_r(struct _reent *r, int fd, struct stat *st) {
   UNUSED(fd);
   UNUSED(st);
   SET_ERR(ENOSYS);
   return -1;
}

USED int _getpid_r(struct _reent *r) {
   UNUSED(r);
   return 1;
}

USED int _isatty_r(struct _reent *r, int fd) {
   switch (fd) {
   case 0:
   case 1:
   case 2:
       return 1;
   default:
       SET_ERR(EBADF);
       return -1;
   }
}

USED int _kill_r(struct _reent *r, int pid, int signal) {
   if (pid == _getpid_r(r)) {
       switch (signal) {
       case SIGHUP:
       case SIGINT:
       case SIGQUIT:
       case SIGILL:
       case SIGTRAP:
       case SIGEMT:
       case SIGFPE:
       case SIGKILL:
       case SIGBUS:
       case SIGSEGV:
       case SIGSYS:
       case SIGPIPE:
       case SIGALRM:
       case SIGTERM:
       default:
           _exit(0);
       }
   } else {
       SET_ERR(ECHILD);
   }
   return -1;
}

USED int _link_r(struct _reent *r, const char *oldf, const char *newf) {
   UNUSED(oldf);
   UNUSED(newf);
   SET_ERR(ENOSYS);
   return -1;
}

USED _off_t _lseek_r(struct _reent *r, int fd, _off_t off, int w) {
   UNUSED(fd);
   UNUSED(off);
   UNUSED(w);
   SET_ERR(ENOSYS);
   return -1;
}

USED int _mkdir_r(struct _reent *r, const char *name, int m) {
   UNUSED(name);
   UNUSED(m);
   SET_ERR(ENOSYS);
   return -1;
}

USED int _open_r(struct _reent *r, const char *name, int f, int m) {
   UNUSED(name);
   UNUSED(f);
   UNUSED(m);
   SET_ERR(EBADF);
   return -1;
}

/*
_ssize_t _read_r(struct _reent *r, int fd, void *b, size_t n) {
   size_t i;
   switch (fd) {
   case 0:
   case 1:
   case 2:
       for (i = 0; i < n; i++)
           ((char*) b)[i] = Board_UARTGetChar();
       return n;
   default:
       SET_ERR(ENODEV);
       return -1;
   }
}
*/
USED _ssize_t _read_r(struct _reent *r, int fd, void *b, size_t n) {
  size_t i = 0;
  switch (fd) {
  case 0:
  case 1:
  case 2:
      while( i < n ){
         int c = __stdio_getchar();
         if( c != -1 ){
            ((char*) b)[i++] = (char) c;
            if( c == '\r' || c == '\n' ){
               // read anotherone to prevent \r\n
               (void) __stdio_getchar();
               return i;
            }
         }
      }
      SET_ERR(ENODEV);
      return -1;
  default:
      SET_ERR(ENODEV);
      return -1;
  }
}

USED int _rename_r(struct _reent *r, const char *oldf, const char *newf) {
   UNUSED(oldf);
   UNUSED(newf);
   SET_ERR(ENOSYS);
   return -1;
}

USED int _stat_r(struct _reent *r, const char *name, struct stat *s) {
   UNUSED(name);
   UNUSED(s);
   SET_ERR(ENOSYS);
   return -1;
}

USED _CLOCK_T_ _times_r(struct _reent *r, struct tms *tm) {
   UNUSED(tm);
   SET_ERR(ENOSYS);
   return -1;
}

USED int _unlink_r(struct _reent *r, const char *name) {
   UNUSED(name);
   SET_ERR(ENOSYS);
   return -1;
}

USED int _wait_r(struct _reent *r, int *st) {
   UNUSED(st);
   SET_ERR(ENOSYS);
   return -1;
}

USED _ssize_t _write_r(struct _reent *r, int fd, const void *b, size_t n) {
   size_t i;
   switch (fd) {
   case 0:
   case 1:
   case 2:
       for (i = 0; i < n; i++)
           __stdio_putchar(((char*) b)[i]);
       return n;
   default:
       SET_ERR(ENODEV);
       return -1;
   }
}

/* This one is not guaranteed to be available on all targets.  */
USED int _gettimeofday_r(struct _reent *r, struct timeval *__tp, void *__tzp) {
   UNUSED(__tp);
   UNUSED(__tzp);
   SET_ERR(ENOSYS);
   return -1;
}

#endif

USED void *_sbrk_r(struct _reent *r, ptrdiff_t incr) {
   extern int _pvHeapStart;
   static void *heap_end;
   void *prev_heap_end;
   if (heap_end == 0) {
       heap_end = &_pvHeapStart;
   }
   prev_heap_end = heap_end;
   heap_end += incr;
   return prev_heap_end;
}
