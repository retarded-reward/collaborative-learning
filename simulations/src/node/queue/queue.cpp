#include "queue.h"
#include <omnetpp/cqueue.h>
#include "units.h"
#include "statistics.h"
#include <string>

using namespace std;
using namespace std::string_literals;

Define_Module(Queue);

class QueuePacketDropPercentageStatisticListener : cListener
{
public:    

    Queue *queue;
    
    simsignal_t pkt_drop_signal;
    simsignal_t pkt_inbound_signal;
    simsignal_t pkt_drop_perc_signal;

    size_t pkt_drop_count = 0;
    size_t pkt_inbound_count = 0;

    QueuePacketDropPercentageStatisticListener(Queue *queue)
    {
        this->queue = queue;
        pkt_drop_signal
         = queue->registerSignal(queue->queue_pkt_drop_name);
        pkt_inbound_signal
         = queue->registerSignal(queue->queue_pkt_inbound_name);
        pkt_drop_perc_signal
         = queue->registerSignal(queue->queue_pkt_drop_perc_name);
                
        queue->subscribe(pkt_drop_signal, this);
        queue->subscribe(pkt_inbound_signal, this);
    }

    virtual void receiveSignal(cComponent *src, simsignal_t id, 
     intval_t value, cObject *details)
    {
        percentage_t pkt_drop_perc;
        
        if (id == pkt_drop_signal){
            pkt_drop_count ++;
        }
        else if (id == pkt_inbound_signal){
            pkt_inbound_count ++;
        }

        pkt_drop_perc = pkt_inbound_count? ((double) pkt_drop_count / pkt_inbound_count) * 100 : 0;
        queue->measure_quantity_by_sid(pkt_drop_perc_signal, pkt_drop_perc);
    }    
};

#define sample_and_send_queue_state(_queue_state_update)\
{\
    queue_state_update = new QueueStateUpdate();\
    sample_queue_state(_queue_state_update);\
    send_queue_state(_queue_state_update);   \
}

void Queue::initialize()
{    
    init_module_params();
    init_data_buffer();
    init_statistic_templates();

    EV_DEBUG << "Queue initialized with capacity "
     << capacity << " and priority " << priority << endl;

}

void Queue::init_statistic_templates()
{
    init_statistic_template(queue_pop_percentage_name, "queue_pop_percentage_over_time",
     "queue%d_pop_percentage", priority);
    init_statistic_template(queue_time_name, "queue_time_over_time", "queue%d_queue_time",
     priority);
    
    init_statistic_template(queue_pkt_drop_name,"queue_pkt_drop",
     "queue%d_pkt_drop", priority);
    init_statistic_template(queue_pkt_inbound_name,"queue_pkt_inbound",
     "queue%d_pkt_inbound", priority);
    init_statistic_template(queue_pkt_drop_perc_name,
     "queue_pkt_drop_percentage_over_time", "queue%d_pkt_drop_percentage", priority);
    queuePacketDropPercentageStatisticListener
     = new QueuePacketDropPercentageStatisticListener(this); 
    
}

void Queue::init_module_params()
{
    capacity = par("capacity").intValue();
    priority = par("priority").intValue();
}

void Queue::init_data_buffer()
{
    data_buffer =
     new FixedCapCQueue(new PriorityCQueue(new cQueue(), priority),capacity);
}

void Queue::handleMessage(cMessage *msg)
{    
    if (is_sim_msg(msg)){
        switch (msg->getKind())
        {
        case DATA_MSG:
            handleDataMsg((DataMsg *)msg);
            /**
             * FIXME: per qualche oscuro motivo, se non si cancella il messaggio dati
             * anche quando è stato accettato e quindi accodato, si ha un memory leak.
             * Questo non dovrebbe succedere perché, quando si invoca la delete sulla
             * coda, in teoria dovrebbe distruggere tutti i messaggi accodati, e infatti
             * lo fa. Tuttavia, rimane comunque un riferimento al messaggio dati.
             * È come se il messaggio dati fosse duplicato quando è inserito nella coda,
             * ma in teoria un oggetto cQueue non lo fa! (Nella documentazione e negli
             * esempi non c'è traccia di un comportamento del genere).
             * BOH!
            */
            break;
        default:
            break;
        }
    }
    else if (is_queue_msg(msg)){
        switch (msg->getKind())
        {
        case QUEUE_DATA_REQUEST:
            handleQueueDataRequest((QueueDataRequest *)msg);
            break;
        default:
            break;
        }
    }
    else{
        EV_ERROR << getName() << ": message name is not an expected topic: "
         << msg->getName() << endl;
    }

    delete msg;    
}

void Queue::handleDataMsg(DataMsg *msg)
{
    QueueStateUpdate *queue_state_update;

    // tries to enqueue the message. If there is no space, drops the message.
    try
    {
        accept_data(msg);
    }
    catch(const std::out_of_range& e)
    {
        drop_data(msg);
    }

    measure_quantity("pkt_arrival_time", simTime().dbl());
    measure_quantity(queue_pkt_inbound_name, 1);

    // state might have changed, so we sample it and send it to servers
    sample_and_send_queue_state(queue_state_update);
    EV_DEBUG << "queue full at " << queue_state_update->getBuffer_pop_percentage() 
     << "%, num of dropped packets: " << queue_state_update->getNum_of_dropped() 
     << endl;   
}

void Queue::handleQueueDataRequest(QueueDataRequest *msg)
{
    QueueDataResponse *queueDataResponse = new QueueDataResponse();

    // try to fetch the desired number of packets from the queue
    fetch_data(queueDataResponse, msg->getData_n());
    
    // send the fetched data to the server requesting it
    send_data(queueDataResponse, msg->getArrivalGate()->getOtherHalf());
}

void Queue::drop_data(DataMsg *msg)
{
    EV_DEBUG << "Data message dropped: id=" << msg->getId() << endl;
    dropped ++;

    measure_quantity(queue_pkt_drop_name, 1);
}

void Queue::accept_data(DataMsg *msg)
{
    msg->setQueueing_time(msg->getArrivalTime());
    data_buffer->insert(msg);
    EV_DEBUG << "Data message accepted: id=" << msg->getId() << endl;
}

void Queue::fetch_data(QueueDataResponse *response, size_t desired_n)
{
    try {
        for (int i = 0; i < desired_n; i ++){
            response->appendData((DataMsg *)data_buffer->pop());
            measure_quantity(queue_time_name, simTime().dbl() - response->getData(i)->getQueueing_time());
        }
    }
    catch (cRuntimeError e){
        // not enough elements in queue, we return the ones we managed to fetch
    }
}

void Queue::send_data(QueueDataResponse *response, cGate *server_gate)
{
    QueueStateUpdate *queueStateUpdate;

    queueStateUpdate = new QueueStateUpdate();
    sample_queue_state(queueStateUpdate);
    response->setStateUpdate(queueStateUpdate);

    EV_DEBUG << "Sending " << response->getDataArraySize() << " data messages to server" << endl;

    send(response, server_gate); 
}

void Queue::sample_queue_state(QueueStateUpdate *msg)
{
    percentage_t buffer_pop_percentage;

    buffer_pop_percentage = (capacity == 0) ? 100.0 : data_buffer->getLength() * 100.0 / capacity;
    measure_quantity(queue_pop_percentage_name, buffer_pop_percentage);
    
    // calcs percentage of queue occupation
    msg->setBuffer_pop_percentage(buffer_pop_percentage);
    
    // samples num of dropped packets and resets counter
    msg->setNum_of_dropped(dropped);
    dropped = 0;
}

void Queue::send_queue_state(QueueStateUpdate *msg)
{
    int server_port_id_start;
    int num_of_servers;
    const char *server_gate_array_name = "servers_inout$o";

    // Sends queue state update to all servers connected to it
    server_port_id_start = gateBaseId(server_gate_array_name);
    num_of_servers = gateSize(server_gate_array_name);
    for (int i = 0; i < num_of_servers; i ++){
        send(i == num_of_servers - 1 ? msg : msg->dup(), server_port_id_start + i); 
    }
}

Queue::~Queue()
{       
    delete data_buffer;
}

