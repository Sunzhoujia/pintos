#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "debug.h"
#include "devices/shutdown.h"
#include "threads/interrupt.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "user/syscall.h"
#include "filesys/filesys.h"

static void syscall_handler (struct intr_frame *);
static int get_user (const uint8_t *uaddr);
static bool put_user (uint8_t *udst, uint8_t byte);
static void *check_read_user_buffer(const void* buffer, size_t size);
static void *check_write_user_buffer(void* buffer, size_t size);
static char *check_read_user_str(const void* buffer);
static void terminate_process(void);

/* syscall */
static void syscall_halt(struct intr_frame* f);
static void syscall_exit(struct intr_frame *f);
static void syscall_exec(struct intr_frame *f);
static void syscall_wait(struct intr_frame *f);
static void syscall_create(struct intr_frame *f);
static void syscall_remove(struct intr_frame *f);
static void syscall_open(struct intr_frame *f);
static void syscall_close(struct intr_frame *f);
static void syscall_filesize(struct intr_frame *f);
static void syscall_read(struct intr_frame *f);
static void syscall_write(struct intr_frame *f);
static void syscall_seek(struct intr_frame *f);
static void syscall_tell(struct intr_frame *f);

void syscall_init (void) {
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void syscall_halt(struct intr_frame* f UNUSED) {
  shutdown_power_off();
}

static void syscall_exit(struct intr_frame *f) {
  int ptr_size = sizeof(void *);
  check_read_user_buffer(f->esp + ptr_size, ptr_size);

  int exit_code = *(int *)(f->esp + sizeof(void *));
  thread_current()->exit_code = exit_code;
  thread_exit(); 
}

static void 
syscall_write(struct intr_frame *f) {
  int ptr_size = sizeof(void *);
  check_read_user_buffer(f->esp + ptr_size, ptr_size);

  int fd = *(int *)(f->esp + ptr_size);
  char *buf = *(char **)(f->esp + 2*ptr_size);
  int size = *(int *)(f->esp + 3*ptr_size);

  check_read_user_buffer(buf, size);

  if (fd == 1) {
    putbuf(buf, size);
    f->eax = size;
  } else {
    struct file* file_ptr = thread_get_file(fd);
    if (file_ptr == NULL) {
      f->eax = -1;
    } else {
      f->eax = file_write(file_ptr, buf, size);
    }
  }
}


static void syscall_exec(struct intr_frame *f) {
  int ptr_size = sizeof(void *);
  check_read_user_buffer(f->esp + ptr_size, ptr_size);

  char* cmd = *(char**)(f->esp + ptr_size);
  check_read_user_str(cmd);

  f->eax =  process_execute(cmd);
}

static void syscall_wait(struct intr_frame *f) {
  int ptr_size = sizeof(void *);
  check_read_user_buffer(f->esp + ptr_size, ptr_size);

  int pid = *(int *)(f->esp + ptr_size);
  f->eax = process_wait(pid);
}


static void syscall_create(struct intr_frame *f) {
  int ptr_size = sizeof(void *);
  check_read_user_buffer(f->esp + ptr_size, 2 * ptr_size);

  char* file = *(char**)(f->esp + ptr_size);
  unsigned initial_size = *(unsigned *)(f->esp + 2 * ptr_size);

  check_read_user_str(file);
  if (file == NULL) {
    terminate_process();
  }

  lock_acquire(&filesys_lock);
  f->eax = filesys_create(file, initial_size);
  lock_release(&filesys_lock);

}

static void syscall_remove(struct intr_frame *f) {
  int ptr_size = sizeof(void *);
  check_read_user_buffer(f->esp + ptr_size, ptr_size);

  char* file = *(char**)(f->esp + ptr_size);
  check_read_user_str(file);

  lock_acquire(&filesys_lock);
  f->eax = filesys_remove(file);
  lock_release(&filesys_lock);
}

static void syscall_open(struct intr_frame *f) {
  int ptr_size = sizeof(void *);
  check_read_user_buffer(f->esp + ptr_size, ptr_size);

  char* file = *(char**)(f->esp + ptr_size);
  check_read_user_str(file);

  if (file == NULL) {
    terminate_process();
  }

  lock_acquire(&filesys_lock);
  struct file* open_file = filesys_open(file);
  lock_release(&filesys_lock);

  if (open_file == NULL) {
    f->eax = -1;
  } else {
    f->eax = thread_add_file(open_file);
  }
}

static void syscall_close(struct intr_frame *f) {
  int ptr_size = sizeof(void *);
  check_read_user_buffer(f->esp + ptr_size, ptr_size);

  int fd = *(int *)(f->esp + ptr_size);
  thread_close_file(fd);
}


static void syscall_filesize(struct intr_frame *f) {
  int ptr_size = sizeof(void *);
  check_read_user_buffer(f->esp + ptr_size, ptr_size);

  int fd = *(int *)(f->esp + ptr_size);
  struct file* file_ptr = thread_get_file(fd);
  if (file_ptr == NULL) {
    f->eax = -1;
  } else {
    f->eax = file_length(file_ptr);
  }
}

static void syscall_read(struct intr_frame *f) {
  int ptr_size = sizeof(void *);
  check_read_user_buffer(f->esp + ptr_size, 3 * ptr_size);

  int fd = *(int *)(f->esp + ptr_size);
  void* buffer = *(void **)(f->esp + 2*ptr_size);
  unsigned size = *(unsigned *)(f->esp + 3*ptr_size);

  check_read_user_buffer(buffer, size);

  if (fd == 0) {
    uint8_t* buf = buffer;
    unsigned i;
    for (i = 0; i < size; i++) {
      buf[i] = input_getc();
    }
    f->eax = size;
  } else {
    struct file* file_ptr = thread_get_file(fd);
    if (file_ptr == NULL) {
      f->eax = -1;
    } else {
      f->eax = file_read(file_ptr, buffer, size);
    }
  }
}

static void syscall_seek(struct intr_frame *f) {
  int ptr_size = sizeof(void *);
  check_read_user_buffer(f->esp + ptr_size, 2 * ptr_size);

  int fd = *(int *)(f->esp + ptr_size);
  unsigned position = *(unsigned *)(f->esp + 2 * ptr_size);

  struct file* file_ptr = thread_get_file(fd);
  if (file_ptr == NULL) {
    terminate_process();
  }
  file_seek(file_ptr, position);
}


static void syscall_tell(struct intr_frame *f) {
  int ptr_size = sizeof(void *);
  check_read_user_buffer(f->esp + ptr_size, ptr_size);

  int fd = *(int *)(f->esp + ptr_size);
  struct file* file_ptr = thread_get_file(fd);
  if (file_ptr == NULL) {
    terminate_process();
  }
  f->eax = file_tell(file_ptr);
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  // printf ("system call!\n");
  check_read_user_buffer(f->esp, sizeof(void*));

  int syscall_type = *(int*)f->esp;
  switch (syscall_type) {
    case SYS_HALT:
      syscall_halt(f);
      break;
    case SYS_EXIT:
      syscall_exit(f);
      break;
    case SYS_EXEC:
      syscall_exec(f);
      break;
    case SYS_WAIT:
      syscall_wait(f);
      break;
    case SYS_CREATE:
      syscall_create(f);
      break;
    case SYS_REMOVE:
      syscall_remove(f);
      break;
    case SYS_OPEN:
      syscall_open(f);
      break;
    case SYS_CLOSE:
      syscall_close(f);
      break;
    case SYS_FILESIZE:
      syscall_filesize(f);
      break;
    case SYS_READ:
      syscall_read(f);  
      break;
    case SYS_WRITE:
      syscall_write(f);
      break;
    case SYS_SEEK:
      syscall_seek(f);
      break;
    case SYS_TELL:
      syscall_tell(f);
      break;
    default:
      NOT_REACHED();
  }
  // thread_exit ();
}

static void *check_read_user_buffer(const void* buffer, size_t size) {
  size_t i;
  if (!is_user_vaddr(buffer)) {
    terminate_process();
  }

  for (i = 0; i < size; i++) {
    if (get_user(buffer + i) == -1) {
      terminate_process();
    }
  }
  return (void *)buffer;
}


static void *check_write_user_buffer(void* buffer, size_t size) {
  size_t i = 0;
  void *ptr = buffer;
  
  if (!is_user_vaddr(buffer)) {
    terminate_process();
  }

  for (i = 0; i < size; i++) {
    if (!put_user(ptr + i, 0)) {
      terminate_process();
    }
    ptr++;
  }
  return buffer;
}

static char *check_read_user_str(const void* buffer) {
  size_t i;
  int c;
  void * str = buffer;

  if (!is_user_vaddr(str)) {
    terminate_process();
  }

  while (true) {
    c = get_user(str);
    if (c == -1) {
      terminate_process();
    } else if (c == '\0') {
      return (char*)str;
    }
    str++;
  }
  NOT_REACHED();  
}


/* Reads a byte at user virtual address UADDR.
   UADDR must be below PHYS_BASE.
   Returns the byte value if successful, -1 if a segfault
   occurred. */
static int
get_user (const uint8_t *uaddr)
{
  int result;
  asm ("movl $1f, %0; movzbl %1, %0; 1:"
       : "=&a" (result) : "m" (*uaddr));
  return result;
}

/* Writes BYTE to user address UDST.
   UDST must be below PHYS_BASE.
   Returns true if successful, false if a segfault occurred. */
static bool
put_user (uint8_t *udst, uint8_t byte)
{
  int error_code;
  asm ("movl $1f, %0; movb %b2, %1; 1:"
       : "=&a" (error_code), "=m" (*udst) : "q" (byte));
  return error_code != -1;
}

static void terminate_process(void) {
  thread_current()->exit_code = -1;
  thread_exit();
  NOT_REACHED();
}