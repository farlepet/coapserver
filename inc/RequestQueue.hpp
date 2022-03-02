#ifndef _REQUEST_QUEUE_HPP_
#define _REQUEST_QUEUE_HPP_

#include <string>
#include <atomic>
#include <vector>
#include <memory>

struct RequestQueueItem;
class RequestQueue;

#include "ResourceMethod.hpp"

struct RequestQueueItem {
    /** Resource method that handled the request */
    ResourceMethod      *src;
    /** Value of request */
    std::vector<uint8_t> data;
    /** Time of request */
    time_t               time;

    RequestQueueItem() {}
    
    RequestQueueItem(ResourceMethod *_src, std::vector<uint8_t> _data, time_t _time) {
        this->src  = _src;
        this->data = _data;
        this->time = _time;
    }
    
    RequestQueueItem(ResourceMethod *_src, std::string _str, time_t _time) {
        this->src  = _src;
        this->data = std::vector<uint8_t>(_str.begin(), _str.end());
        this->time = _time;
    }
};

/**
 * @brief Queue of CoAP requests to handle
 *
 * Initial handling of the request and the response are handled by the applicable
 * ResourceMethod class. These requests are additionally queued here in order to
 * make longer-running commands more acceptable without holding up the CoAP
 * server thread.
 *
 * This queue is implemented as a ring-buffer that is inherently thread-safe when
 * only using a single consumer and producer. If multiple server threads, or
 * multiple request handler threads, are desired in the future, this whill need
 * to be reworked.
 */
class RequestQueue {
    private:
        /** Maximum number of items queue can hold */
        const size_t                        maxSize;
        /** Ring-buffer of queue items */
        std::unique_ptr<RequestQueueItem[]> items;
        /** Location of first item in the queue */
        std::atomic<size_t>                 start;
        /** Location of the last item in the queue */
        std::atomic<size_t>                 end;

    public:
        /**
         * @brief RequestQueue constructor
         * 
         * @param _maxSize Maximum size of queue in number of items
         */
        RequestQueue(size_t _maxSize);

        /**
         * @brief Adds a RequestQueueItem to the queue
         *
         * @param item Item to add to the list
         *
         * @return int 0 on success, non-zero on failure
         */
        int enqueue(const RequestQueueItem &item);

        /**
         * @brief Retrieves the first item from the queue, and removes it
         *
         * @return RequestQueueItem First item in list
         * 
         * @throws <todo>
         */
        RequestQueueItem dequeue();

        /**
         * @brief Returns the number of items in the queue
         * 
         * @return size_t Number of items in the queue
         */
        size_t count(void);
};

#endif /* _REQUEST_QUEUE_HPP_ */
