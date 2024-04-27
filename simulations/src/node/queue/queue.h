#ifndef QUEUE_H_INCLUDED
#define QUEUE_H_INCLUDED

#include <omnetpp/cqueue.h>
#include <omnetpp/csimplemodule.h>
#include <stdexcept>
#include "DataMsg_m.h"
#include "QueueDataRequest_m.h"
#include "QueueDataResponse_m.h"
#include "QueueStateUpdate_m.h"

using namespace std;
using namespace omnetpp;

class DecCQueue : public cQueue {
protected:
    cQueue *queue;
    
public:
    DecCQueue(cQueue *queue){
        this->queue = queue;
    }  
    ~DecCQueue(){
        delete queue;
        EV_DEBUG << "Queue deleted" << endl;
    }
   

    /**
     * Creates and returns an exact copy of this object.
     * Contained objects that are owned by the queue will be duplicated
     * so that the new queue will have its own copy of them.
     */
    virtual cQueue *dup() const override  {return queue->dup();}

    /**
     * Produces a one-line description of the object's contents.
     * See cObject for more details.
     */
    virtual std::string str() const override {return queue->str();};

    /**
     * Calls v->visit(this) for each contained object.
     * See cObject for more details.
     */
    virtual void forEachChild(cVisitor *v) override {queue->forEachChild(v);};


    /** @name Setup, insertion and removal functions. */
    //@{
    /**
     * Sets the comparator. This only affects future insertions,
     * i.e. the queue's current content will not be re-sorted.
     */
    virtual void setup(Comparator *cmp) {queue->setup(cmp);};

    /**
     * Sets the comparator function. This only affects future insertions,
     * i.e. the queue's current content will not be re-sorted.
     */
    virtual void setup(CompareFunc cmp) {queue->setup(cmp);};

    /**
     * Adds an element to the back of the queue. Trying to insert nullptr
     * is an error (throws cRuntimeError).
     */
    virtual void insert(cObject *obj) {
        queue->insert(obj);

    };

    /**
     * Inserts exactly before the given object. If the given position
     * does not exist or if you try to insert nullptr, a cRuntimeError
     * is thrown.
     */
    virtual void insertBefore(cObject *where, cObject *obj) {queue->insertBefore(where, obj);};

    /**
     * Inserts exactly after the given object. If the given position
     * does not exist or if you try to insert nullptr, a cRuntimeError
     * is thrown.
     */
    virtual void insertAfter(cObject *where, cObject *obj) {queue->insertAfter(where, obj);};

    /**
     * Unlinks and returns the object given. If the object is not in the
     * queue, nullptr is returned.
     */
    virtual cObject *remove(cObject *obj) {return queue->remove(obj);};

    /**
     * Unlinks and returns the front element in the queue. If the queue
     * was empty, cRuntimeError is thrown.
     */
    virtual cObject *pop() {return queue->pop();};

    /**
     * Empties the container. Contained objects that were owned by the
     * queue (see getTakeOwnership()) will be deleted.
     */
    virtual void clear() {queue->clear();};
    //@}

    /** @name Query functions. */
    //@{
    /**
     * Returns pointer to the object at the front of the queue.
     * This is the element to be returned by pop().
     * Returns nullptr if the queue is empty.
     */
    virtual cObject *front() const {return queue->front();};

    /**
     * Returns pointer to the last (back) element in the queue.
     * This is the element most recently added by insert().
     * Returns nullptr if the queue is empty.
     */
    virtual cObject *back() const {return queue->back();}

    /**
     * Returns the number of objects contained in the queue.
     */
    virtual int getLength() const {return queue->getLength();};

    /**
     * Returns true if the queue is empty.
     */
    bool isEmpty() const {return queue->getLength()==0;}

    /**
     * DEPRECATED. Use getLength() instead.
     */
    [[deprecated("use getLength() instead")]] int length() const {return queue->getLength();}

    /**
     * DEPRECATED. Use isEmpty() instead.
     */
    [[deprecated("use isEmpty() instead")]] bool empty() const {return queue->isEmpty();}

    /**
     * Returns the ith element in the queue, or nullptr if i is out of range.
     * get(0) returns the front element. This method performs linear
     * search.
     */
    virtual cObject *get(int i) const {return queue->get(i);};

    /**
     * Returns true if the queue contains the given object.
     */
    virtual bool contains(cObject *obj) const {return queue->contains(obj);};
    //@}

    /** @name Ownership control flag. */
    //@{

    /**
     * Sets the flag which determines whether the container object should
     * automatically take ownership of the objects that are inserted into it.
     * It does not affect objects already in the queue. When an inserted
     * object is owned by the queue, that means it will be deleted when
     * the queue object is deleted or cleared, and will be duplicated when
     * the queue object is duplicated or copied.
     *
     * Setting the flag to false does not affect the treatment of objects
     * that are NOT cOwnedObject. Since they do not support the ownership
     * protocol, they will always be treated by the queue as owned objects.
     */
    void setTakeOwnership(bool tk) {queue->setTakeOwnership(tk);}

    /**
     * Returns the flag which determines whether the container object
     * should automatically take ownership of the objects that are inserted
     * into it. See setTakeOwnedship() for more details.
     */
    bool getTakeOwnership() const {return queue->getTakeOwnership();}
    //@}

};

/**
 * Adds a priority to a queue
*/
class PriorityCQueue : public DecCQueue {
protected:
    const int priority;

public:
    PriorityCQueue(cQueue *queue, int priority) : DecCQueue(queue), priority(priority) { 
    }

    int getPriority(){
        return this->priority;
    }    
};

/**
 * Adds a fixed capacity to a queue
*/
class FixedCapCQueue : public DecCQueue {
protected:
    const size_t capacity;
public:
    FixedCapCQueue(cQueue *queue, size_t capacity) : DecCQueue(queue), capacity(capacity){
    }
    void insert(cObject *msg) override {
        if (queue->getLength() < capacity) {
            cObject *data_msg= msg->dup();
            DecCQueue::insert(data_msg);
        }
        else
            throw std::out_of_range("Queue is full");
    }
    void insertBefore(cObject *where, cObject *msg) override {
        if (queue->getLength() < capacity) {
            cObject *data_msg= msg->dup();
            queue->insertBefore(where, data_msg);
        }
        else
            throw std::out_of_range("Queue is full");
    }
    void insertAfter(cObject *where, cObject *msg) override {
        if (queue->getLength() < capacity) {
            cObject *data_msg= msg->dup();
            queue->insertAfter(where, data_msg);
        }
        else 
            throw std::out_of_range("Queue is full");
    }
};

class Queue : public cSimpleModule {
protected:
    cQueue *data_buffer;

    size_t capacity;
    int priority;

    /**
     * Holds the number of dropped packets since last queue state sampling.
    */
    unsigned int dropped = 0;

    virtual void initialize() override;
    void init_module_params();
    void init_data_buffer();

    virtual void handleMessage(cMessage *msg) override;
    void handleDataMsg(DataMsg *msg);
    void handleQueueDataRequest(QueueDataRequest *msg);

    void drop_data(DataMsg *msg);
    void accept_data(DataMsg *msg);
    void send_data(QueueDataResponse *response, cGate *server_gate);
    void fetch_data(QueueDataResponse *response, size_t desired_n);

    void sample_queue_state(QueueStateUpdate *msg);
    void send_queue_state(QueueStateUpdate *msg);

    ~Queue();

};

#endif // QUEUE_H_INCLUDED