#include <condition_variable>
#include <iostream>
#include <mutex>
#include <vector>
#include <optional>
#include <thread>
#include <chrono>

template <typename T>
class CircularBuffer {
public:
    CircularBuffer(size_t size) : buffer_(size), capacity_(size), head_(0), tail_(0), size_(0) {}

    void enqueue(const T& elem) {
        std::unique_lock<std::mutex> lock(mutex_);

        // Wait until there's an empty slot
        /**
         * The predicate (lambda function) is repeatedly checked for its truth value.
         * The condition variable continues to wait until the predicate is true.
         * Using a predicate, we can avoid spurious wakeup (i.e. a case when
         * no notify() was called but the thread still woke up.)
        */
        cond_not_full_.wait(lock, [this]() { return size_ < capacity_; });

        // At this point we are assured that there's an empty slot
        // So we enqueue the new element
        buffer_[tail_] = elem;
        tail_ = (tail_ + 1) % capacity_;
        ++size_;

        // Notify
        cond_not_empty_.notify_one();        
    }

    std::optional<T> dequeue() {
        std::unique_lock<std::mutex> lock(mutex_);

        // Wait until queue is non-empty
        cond_not_empty_.wait(lock, [this]() { return size_ > 0; });

        // At this point, we're guaranteed that the queue is non-empty
        T item = buffer_[head_];
        head_ = (head_ + 1) % capacity_;
        --size_;

        // Notify
        cond_not_empty_.notify_one();

        return item;
    }

private:
    std::vector<T> buffer_;
    const size_t capacity_;
    size_t head_;
    size_t tail_;
    size_t size_;
    std::mutex mutex_;
    std::condition_variable cond_not_full_;
    std::condition_variable cond_not_empty_;
};

void producer(CircularBuffer<int>& queue, int items_to_produce) {
    for (int i = 0; i < items_to_produce; ++i) {
        queue.enqueue(i);
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Simulate work
        std::cout << "Produced: " << i << std::endl;
    }
}

void consumer(CircularBuffer<int>& queue, int items_to_consume) {
    for (int i = 0; i < items_to_consume; ++i) {
        std::optional<int> item;
        while (!(item = queue.dequeue())) {
            std::this_thread::yield(); // Wait until the item is available
        }
        std::cout << "Consumed: " << *item << std::endl;
    }
}

int main() {
    const int queue_capacity = 5;
    const int items_to_produce = 10;
    const int items_to_consume = 10;

    CircularBuffer<int> queue(queue_capacity);

    std::thread producer_thread(producer, std::ref(queue), items_to_produce);
    std::thread consumer_thread(consumer, std::ref(queue), items_to_consume);

    producer_thread.join();
    consumer_thread.join();

    return 0;
}
