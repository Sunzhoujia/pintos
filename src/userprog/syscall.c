#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "debug.h"
#include "devices/shutdown.h"
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"

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




void syscall_init (void) {
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}


static void syscall_halt(struct intr_frame* f UNUSED) {
  shutdown_power_off();
}

static void syscall_exit(struct intr_frame *f) {
  // exit_code is passed as ARG0, after syscall number
  int exit_code = *(int *)(f->esp + sizeof(int));
  thread_current()->exit_code = exit_code;
  printf ("%s: exit(%d)\n", thread_current()->name, thread_current()->exit_code);
  thread_exit(); 
}

static void 
syscall_write(struct intr_frame *f)
{
  int ptr_size = sizeof(void *);
  int fd = *(int *)(f->esp + ptr_size);
  char *buf = *(char **)(f->esp + 2*ptr_size);
  int size = *(int *)(f->esp + 3*ptr_size);

  if (fd == 1) {
    putbuf(buf, size);
    f->eax = size;
  }
}



static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  // printf ("system call!\n");
  int syscall_type = *(int*)f->esp;
  switch (syscall_type) {
    case SYS_HALT:
      syscall_halt(f);
      break;
    case SYS_EXIT:
      syscall_exit(f);
      break;
    case SYS_WRITE:
      syscall_write(f);
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
    if (!get_user(buffer + i)) {
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