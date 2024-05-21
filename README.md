# SPSCQueue implementations

In this repository, I practice making Single Producer Single Consumer (SPSC) Queue implementations in C++, with and without locks.

## To run

```
g++ -std=c++17 -pthread <file_name>.cpp -o spsc_queue_test
```

## Why a lock-free queue?

A lock-free queue allows multiple threads to access the queue without using locks, avoiding potential performance bottlenecks and issues like deadlocks.

## Differences

### With locks

Implementing the SPSC Queue with locks is simpler. We have a mutex and two condition variables, representing the "not full" and "not empty" states. For each operation (`enqueue` or `dequeue`), we wait on one condition variable as appropriate. Once we get a signal, we perform the operation and associated cleanup. Lastly, we notify the other condition variable so that the next operation can proceed.

### Without locks

This case is more interesting. Since we can't use locks but still need to guarantee consistency, we need to ensure that the operations are executed without interruptions. This is done by using `std::atomic` operations.

Atomic operations are used alongside `std::memory_order` enumerations:

1. `std::memory_order_relaxed`: No synchronization or ordering constraints. The atomic operation itself is guaranteed to be atomic, but there are no guarantees about the ordering of this operation relative to other memory accesses.
1. `std::memory_order_acquire`: Ensures that all memory writes made by other threads before releasing the same atomic variable are visible to the current thread after the acquire operation. Prevents memory operations before the acquire from being reordered after it.
1. `std::memory_order_release`: Ensures that all memory writes made by the current thread before the release operation are visible to other threads that acquire the same atomic variable. Prevents memory operations after the release from being reordered before it.
