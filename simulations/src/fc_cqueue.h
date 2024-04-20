#ifndef FC_CQUEUE_H_INCLUDED
#define FC_CQUEUE_H_INCLUDED

#include <omnetpp/cqueue.h>

using namespace std;
using namespace omnetpp;

class FixedCapCQueue : public cQueue {
protected:
    size_t capacity;
public:
    FixedCapCQueue(size_t capacity) {
        this->capacity = capacity;
    }
    void insert(cObject *msg) override {
        if (this->getLength() < capacity) {
            cQueue::insert(msg);
        }
    }
    void insertBefore(cObject *where, cObject *msg) override {
        if (this->getLength() < capacity) {
            cQueue::insertBefore(where, msg);
        }
    }
    void insertAfter(cObject *where, cObject *msg) override {
        if (this->getLength() < capacity) {
            cQueue::insertAfter(where, msg);
        }
    }
};

#endif // FC_CQUEUE