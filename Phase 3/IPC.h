#ifndef IPC_CLASS
#define IPC_CLASS

#include <time.h>
#include <string>
#include "Sema.h"
#include "Queue.h"

//Message Type struct
struct Message_Type {
    int Message_Type_Id;
    char Message_Type_Description[64];
};

//Message type constants, will be added to if more message types become necessary
const Message_Type MESSAGE_TYPE_TEXT {0, "Text message, no response"};
const Message_Type MESSAGE_TYPE_SERVICE_REQUEST {1, "Request for service, send service notification back"};
const Message_Type MESSAGE_TYPE_SERVICE_NOTIFICATION {2, "Notification of service request result, no response"};

//Message struct
struct Message {
    int Source_Task_Id;
    int Destination_Task_Id;
    time_t Message_Arrival_Time;
    Message_Type Msg_Type;
    unsigned long Msg_Size;
    char *Msg_Text;

    Message();
    Message(int s_id, int d_id, Message_Type msg_t, char *msg);

    // FIX: must take const Message& so it can copy temporaries
    Message(const Message &msg);

    std::string print();
    ~Message();
};

//Inter Process Communication class.
//Holds and manages the mailboxes for every task.
//Assumes that task id and position in the mailbox array are the same value for every task.
class ipc {
private:
    class Mailbox : public Queue<Message>
    {
        private:
        semaphore sema;
        friend class ipc;
        public:
        Mailbox();
        Mailbox(scheduler *sched, std::string name);
        void print_all(); 
    };

    Mailbox *mailbox;
    int *msg_count;
    int max_tasks;
    scheduler *sched;

public:
    ipc(int max_tasks, int &error_code, scheduler *sched);
    int Message_Send(Message *msg);
    int Message_Send(int Sender_Id, int Destination_Id, char *Message, int Message_Type);
    int Message_Receive(int Task_Id, Message *Message);
    int Message_Receive(int Task_Id, char *Message, int *Message_Type);
    int Message_Count(int Task_id);
    int Message_Count();
    void Message_Print(int Task_id);
    int Message_DeleteAll(int Task_id);
    void ipc_Message_Dump();
};

#endif
