#ifndef FC_CQUEUE_H_INCLUDED
#define FC_CQUEUE_H_INCLUDED

#include <omnetpp/cqueue.h>
#include <omnetpp/cmessage.h>
#include <stdexcept>

using namespace std;
using namespace omnetpp;

class FixedCapCQueue : public cQueue {
protected:
    size_t capacity;
public:
    FixedCapCQueue(size_t capacity){
        this->capacity = capacity;
    }
    void insert(cObject *msg) override {
        if (this->getLength() < capacity) {
            cObject *data_msg= msg->dup();
            cQueue::insert(data_msg);
        }
        else
            throw std::out_of_range("Queue is full");
    }
    void insertBefore(cObject *where, cObject *msg) override {
        if (this->getLength() < capacity) {
            cObject *data_msg= msg->dup();
            cQueue::insertBefore(where, data_msg);
        }
        else
            throw std::out_of_range("Queue is full");
    }
    void insertAfter(cObject *where, cObject *msg) override {
        if (this->getLength() < capacity) {
            cObject *data_msg= msg->dup();
            cQueue::insertAfter(where, data_msg);
        }
        else 
            throw std::out_of_range("Queue is full");
    }
};

#endif // FC_CQUEUE