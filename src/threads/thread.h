#ifndef THREADS_THREAD_H
#define THREADS_THREAD_H

#include "threads/fixed-point.h"
#include "threads/synch.h"
#include <debug.h>
#include <list.h>
#include <stdint.h>

/** States in a thread's life cycle. */
enum thread_status
  {
    THREAD_RUNNING,     /**< Running thread. */
    THREAD_READY,       /**< Not running but ready to run. */
    THREAD_BLOCKED,     /**< Waiting for an event to trigger. */
    THREAD_DYING        /**< About to be destroyed. */
  };

/** Thread identifier type.
   You can redefine this to whatever type you like. */
typedef int tid_t;
#define TID_ERROR ((tid_t) -1)          /**< Error value for tid_t. */

/** Thread priorities. */
#define PRI_MIN 0                       /**< Lowest priority. */
#define PRI_DEFAULT 31                  /**< Default priority. */
#define PRI_MAX 63                      /**< Highest priority. */


/** Maximum number of files that a process can open. */
#define MAX_FD 128                    

/** A kernel thread or user process.

   Each thread structure is stored in its own 4 kB page.  The
   thread structure itself sits at the very bottom of the page
   (at offset 0).  The rest of the page is reserved for the
   thread's kernel stack, which grows downward from the top of
   the page (at offset 4 kB).  Here's an illustration:

        4 kB +---------------------------------+
             |          kernel stack           |
             |                |                |
             |                |                |
             |                V                |
             |         grows downward          |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             +---------------------------------+
             |              magic              |
             |                :                |
             |                :                |
             |               name              |
             |              status             |
        0 kB +---------------------------------+

   The upshot of this is twofold:

      1. First, `struct thread' must not be allowed to grow too
         big.  If it does, then there will not be enough room for
         the kernel stack.  Our base `struct thread' is only a
         few bytes in size.  It probably should stay well under 1
         kB.

      2. Second, kernel stacks must not be allowed to grow too
         large.  If a stack overflows, it will corrupt the thread
         state.  Thus, kernel functions should not allocate large
         structures or arrays as non-static local variables.  Use
         dynamic allocation with malloc() or palloc_get_page()
         instead.

   The first symptom of either of these problems will probably be
   an assertion failure in thread_current(), which checks that
   the `magic' member of the running thread's `struct thread' is
   set to THREAD_MAGIC.  Stack overflow will normally change this
   value, triggering the assertion. */
/** The `elem' member has a dual purpose.  It can be an element in
   the run queue (thread.c), or it can be an element in a
   semaphore wait list (synch.c).  It can be used these two ways
   only because they are mutually exclusive: only a thread in the
   ready state is on the run queue, whereas only a thread in the
   blocked state is on a semaphore wait list. */
struct thread
  {
    /* Owned by thread.c. */
    tid_t tid;                          /**< Thread identifier. */
    enum thread_status status;          /**< Thread state. */
    char name[16];                      /**< Name (for debugging purposes). */
    uint8_t *stack;                     /**< Saved stack pointer. */
    int priority;                       /**< Priority. */
    int origin_priority;
    int nice;
    fixed_t recent_cpu;
    int exit_code;
    struct thread* parent;              /**< Thread's parent. */
    struct list child_list;             /**< List of children. */
    struct child_entry *as_child;       /**< Thread itself's child_entry. This will be added 
                                           to its parent's child_list and is heap-allocated 
                                           so that it lives after the thread dies. */
    struct semaphore sema_exec;         /**< Semaphore for executing (spawning) a new process. 
                                           "UPed" after knowing whether the child has loaded 
                                           its executable successfully. */
    bool exec_success;                  /**< Whether new process successfully loaded its executable. */
    int next_fd;                        /**< Next slot to put a file object. */
    struct file* fd_table[128];         /**< File descriptor table. */
    struct file* exec_file;            /**< Pointer to the executable file. */

    struct list_elem *waiting_elem;     
    struct list lock_list;
    struct list_elem allelem;           /**< List element for all threads list. */

    /* Shared between thread.c and synch.c. */
    struct list_elem elem;              /**< List element. */

#ifdef USERPROG
    /* Owned by userprog/process.c. */
    uint32_t *pagedir;                  /**< Page directory. */
#endif

    /* Owned by thread.c. */
    unsigned magic;                     /**< Detects stack overflow. */
  };

/** Info of a thread's child  */
struct child_entry {
  tid_t tid;                        /**< Child's tid. */
  int exit_code;                    /**< Child's exit code. */
  struct thread *t;                 /**< Pointer to child thread. Set to NULL when no longer alive. */
  bool is_alive;                    /**< Whether the child is still alive. */
  bool is_waiting_on;               /**< Whether the parent is waiting on the child. */
  struct semaphore wait_sema;       /**< Semaphore to let parent wait on the child */
  struct list_elem elem;
};


/** If false (default), use round-robin scheduler.
   If true, use multi-level feedback queue scheduler.
   Controlled by kernel command-line option "-o mlfqs". */
extern bool thread_mlfqs;

void thread_init (void);
void thread_start (void);

void thread_tick (void);
void thread_print_stats (void);

typedef void thread_func (void *aux);
tid_t thread_create (const char *name, int priority, thread_func *, void *);

void thread_block (void);
void thread_unblock (struct thread *);

struct thread *thread_current (void);
tid_t thread_tid (void);
const char *thread_name (void);

void thread_exit (void) NO_RETURN;
void thread_yield (void);

/** Performs some operation on thread t, given auxiliary data AUX. */
typedef void thread_action_func (struct thread *t, void *aux);

bool compare_priority_func (const struct list_elem *a,
                             const struct list_elem *b,
                             void *aux UNUSED);

void thread_foreach (thread_action_func *, void *);

/** For BSD scheduler */
void update_load_avg_and_recent_cpu(void);
void update_priority(void);
void thread_calculate_priority(struct thread* t);


int thread_get_priority (void);
void thread_set_priority (int);

struct list* thread_get_lock_list(void);

int thread_get_nice (void);
void thread_set_nice (int);
int thread_get_recent_cpu (void);
int thread_get_load_avg (void);

/**  For accessing fd table.*/
struct file* thread_get_file(int fd);
int thread_add_file(struct file* f);
void thread_close_file(int fd);


#endif /**< threads/thread.h */
