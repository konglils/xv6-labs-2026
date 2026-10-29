// Mutual exclusion lock.
struct spinlock {
  uint locked; // Is the lock held?

  // For debugging:
  char *name;      // Name of lock.
  struct cpu *cpu; // The cpu holding the lock.
#ifdef LAB_LOCK
  int nts;
  int n;
#endif
};

#define RWLK_P (1 << 31)

#ifdef LAB_LOCK
// Reader-writer lock.
struct rwspinlock {
  // Put your implementation here.
  uint rlock;
  uint wlock;
};
#endif
