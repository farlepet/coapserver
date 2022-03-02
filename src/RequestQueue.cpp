#include <stdexcept>

#include "RequestQueue.hpp"

RequestQueue::RequestQueue(size_t _maxSize) :
maxSize(_maxSize),
items(new RequestQueueItem[_maxSize]) {
    this->start   = 0;
    this->end     = 0;
}

int RequestQueue::enqueue(const RequestQueueItem &item) {
    if(this->start == (this->end + 1) % this->maxSize) {
        /* No room */
        return -1;
    }

    this->items[this->end] = item;
    this->end++;
    if(this->end >= this->maxSize) {
        this->end = 0;
    }

    return 0;
}

RequestQueueItem RequestQueue::dequeue(void) {
    if(this->start == this->end) {
        throw std::runtime_error("Attempt to dequeue on empty RequestQueue!");
    }

    RequestQueueItem item = this->items[this->start];
    this->start++;
    if(this->start >= this->maxSize) {
        this->start = 0;
    }

    return item;
}

size_t RequestQueue::count(void) {
    return (this->end + this->maxSize - this->start) % this->maxSize;
}
