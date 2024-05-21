#include <iostream>
#include <vector>
#include <optional>
#include <thread>
#include <chrono>
#include <atomic>

template <typename T>
class LockFreeCircularBuffer {
public:
    LockFreeCircularBuffer(size_t size) : buffer_(size), capacity_(size), head_(0), tail_(0) {}

    bool enqueue(const T& elem) {
        size_t tail = tail_.load(std::memory_order_relaxed);
        size_t next_tail = (tail + 1) % capacity_;

        if (next_tail == head_.load(std::memory_order_acquire)) {
            // Queue is full
            return false;
        }

        buffer_[tail] = elem;
        tail_.store(next_tail, std::memory_order_release);
        return true;
    }

    bool dequeue(T& item) {
        size_t head = head_.load(std::memory_order_relaxed);
        if (head == tail_.load(std::memory_order_acquire)) {
            // Queue is empty
            return false;
        }

        // Store the dequeued item in the given variable
        item = buffer_[head];

        head_.store((head + 1) % capacity_, std::memory_order_release);
        return true;
    }

private:
    std::vector<T> buffer_;
    const size_t capacity_;
    std::atomic<size_t> head_;
    std::atomic<size_t> tail_;
};

void producer(LockFreeCircularBuffer<int>& queue, int items_to_produce) {
    for (int i = 0; i < items_to_produce; ++i) {
        while (!queue.enqueue(i)) {
            std::this_thread::yield(); // Wait until there is space
        }
        std::cout << "Produced: " << i << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Simulate work
    }
}

void consumer(LockFreeCircularBuffer<int>& queue, int items_to_consume) {
    for (int i = 0; i < items_to_consume; ++i) {
        int item;
        while (!queue.dequeue(item)) {
            std::this_thread::yield(); // Wait until there is an item
        }
        std::cout << "Consumed: " << item << std::endl;
    }
}

int main() {
    const int queue_capacity = 5;
    const int items_to_produce = 10;
    const int items_to_consume = 10;

    LockFreeCircularBuffer<int> queue(queue_capacity);

    std::thread producer_thread(producer, std::ref(queue), items_to_produce);
    std::thread consumer_thread(consumer, std::ref(queue), items_to_consume);

    producer_thread.join();
    consumer_thread.join();

    return 0;
}
