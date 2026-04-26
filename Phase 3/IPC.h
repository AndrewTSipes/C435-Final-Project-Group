#ifndef IPC_CLASS
#define IPC_CLASS

#include <time.h>
#include <string>
#include <cstring>
#include "Sema.h"
#include "Queue.h"

struct Message_Type {
    int Message_Type_Id;
    char Message_Type_Description[64];
};

const Message_Type MESSAGE_TYPE_TEXT {0, "Text message, no response"};
const Message_Type MESSAGE_TYPE_SERVICE_REQUEST {1, "Request for service, send service notification back"};
const Message_Type MESSAGE_TYPE_SERVICE_NOTIFICATION {2, "Notification of service request result, no response"};

struct Message {
    int Source_Task_Id;
    int Destination_Task_Id;
    time_t Message_Arrival_Time;
    Message_Type Msg_Type;
    unsigned long Msg_Size;
    char *Msg_Text;

    Message();
    Message(int s_id, int d_id, Message_Type msg_t, char *msg);
    Message(const Message &msg);

    Message& operator=(const Message &msg) {
        if (this == &msg) return *this;

        Source_Task_Id = msg.Source_Task_Id;
        Destination_Task_Id = msg.Destination_Task_Id;
        Message_Arrival_Time = msg.Message_Arrival_Time;
        Msg_Type = msg.Msg_Type;
        Msg_Size = msg.Msg_Size;

        delete[] Msg_Text;

        if (msg.Msg_Text && Msg_Size > 0) {
            Msg_Text = new char[Msg_Size];
            memcpy(Msg_Text, msg.Msg_Text, Msg_Size);
        } else {
            Msg_Text = nullptr;
        }

        return *this;
    }

    std::string print();
    ~Message();
};

class ipc {
private:
    class Mailbox : public Queue<Message> {
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
