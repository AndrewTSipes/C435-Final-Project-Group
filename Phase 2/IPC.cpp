#include "IPC.h"
#include <cstring>
#include <iostream>
#include <iomanip>
#include <sstream>


std::string Message::print()
{
    std::ostringstream msg;

    msg << std::left << std::setw(20) << Source_Task_Id;
    msg << std::left << std::setw(30) << Destination_Task_Id;
    msg << std::left << std::setw(64) << Msg_Text;
    msg << std::left << std::setw(20) << Msg_Size;
    msg << std::left << std::setw(20) << Msg_Type.Message_Type_Description;
    msg << std::left << std::setw(20) << std::ctime(&Message_Arrival_Time);
    msg << std::endl;
    return msg.str();
}
Message::Message()
{
    Source_Task_Id = 0;
    Destination_Task_Id = 0;
    Message_Arrival_Time = 0;
    Msg_Size = 0;
    Msg_Text = nullptr;
}
Message::Message(int s_id, int d_id, Message_Type msg_t, char *msg) 
    : Source_Task_Id(s_id),
    Destination_Task_Id(d_id),
    Msg_Type(msg_t)
{
    if(msg == nullptr) return;
    Msg_Size = strlen(msg) + 1;// Note: strlen() returns the length of a c string NOT INCLUDING THE NULL TERMINATOR.
    //For this reason, Msg_Size is incremented to include the null terminator in the copy.
    Msg_Text = new char[Msg_Size];
    memcpy(Msg_Text, msg, Msg_Size);
}

Message::Message(Message &msg)
    : Source_Task_Id(msg.Source_Task_Id),
    Destination_Task_Id(msg.Destination_Task_Id),
    Msg_Type(msg.Msg_Type),
    Msg_Size(msg.Msg_Size)
{
    Msg_Text = new char[Msg_Size];
    memcpy(Msg_Text, msg.Msg_Text, Msg_Size);
}
Message::~Message()
{
}
ipc::Mailbox::Mailbox(scheduler *sched, std::string name) : sema(semaphore(1, name, sched))
{}

ipc::Mailbox::Mailbox() : sema(semaphore(1, "mailbox", nullptr))
{}

void ipc::Mailbox::print_all()
{
    std::cout << std::left << std::setw(20) << "Source Task-id";
    std::cout << std::left << std::setw(30) << "Destination Task-id";
    std::cout << std::left << std::setw(64) << "Message Content";
    std::cout << std::left << std::setw(20) << "Message Size";
    std::cout << std::left << std::setw(20) << "Message Type";
    std::cout << std::left << std::setw(20) << "Message Arrival Time";
    std::cout << std::endl;
    std::cout << std::left << std::setfill('#') << std::setw(174) << '#' << std::endl;
    node *ptr = head;
    while(ptr != nullptr)
    {
        std::cout << ptr->value.print();
        ptr = ptr->next;
    }
}
ipc::ipc(int max_tasks, int &error_code, scheduler *sched) : max_tasks(max_tasks)
{
    mailbox = new ipc::Mailbox[max_tasks];
    for(int i = 0; i < max_tasks; i++)
    {
        std::stringstream name;
        name << "mailbox_" << i;
        mailbox[i] = Mailbox(sched, name.str());
    }
    msg_count = new int[max_tasks]{};
}

int ipc::Message_Send(Message* msg)
{
    if(msg == nullptr) return -1;
    int d_id = msg->Destination_Task_Id;
    msg->Message_Arrival_Time = time(nullptr);
    mailbox[d_id].En_Q(*msg);
    msg_count[d_id]++;
    return 1;
}
int ipc::Message_Send(int Sender_Id, int Destination_Id, char *message, int message_type)
{
    if(message == nullptr) return -1;
    Message_Type msg_type;
    
    switch(message_type)
    {
        case 0: msg_type = MESSAGE_TYPE_TEXT; break;
        case 1: msg_type = MESSAGE_TYPE_SERVICE_REQUEST; break;
        case 2: msg_type = MESSAGE_TYPE_SERVICE_NOTIFICATION; break;
        default: return -1;
    }
    //This assumes that the task_id will be the same as the task's mailbox' position in the array
    //This will also require that the number of tasks stays below max_tasks
    //I recommend creating a global variable called MAX_TASKS for this purpose
    auto msg_length = strlen(message) + 1;
    char *msg = new char[msg_length];
    memcpy(msg, message, msg_length);
    Message new_msg = Message(Sender_Id, Destination_Id, msg_type, message);
    new_msg.Message_Arrival_Time = time(nullptr);
    mailbox[Destination_Id].En_Q(new_msg);
    msg_count[Destination_Id]++;
    return 1;
}

int ipc::Message_Receive(int Task_Id, char* message, int *msg_type)
{
    if(mailbox[Task_Id].isEmpty()) return 0;
    Message msg = mailbox[Task_Id].De_Q();
    memcpy(message, msg.Msg_Text, msg.Msg_Size);
    *msg_type = msg.Msg_Type.Message_Type_Id;
    msg_count[Task_Id]--;
    return 1;
}

int ipc::Message_Receive(int Task_Id, Message *message)
{
    if(mailbox[Task_Id].isEmpty()) return 0;
    *message = mailbox[Task_Id].De_Q();
    msg_count[Task_Id]--;
    return 1;
}
int ipc::Message_Count(int Task_id)
{
    return msg_count[Task_id];
}

int ipc::Message_Count()
{
    int msg_count_total;
    for(int i = 0; i < max_tasks; i++)
    {
        msg_count_total += msg_count[i];
    }
    return msg_count_total;
}

void ipc::Message_Print(int Task_id)
{
    if(!mailbox[Task_id].isEmpty())
    {
        mailbox[Task_id].print_all();
    }
}

int ipc::Message_DeleteAll(int Task_id)
{
    mailbox[Task_id].Reset();
}

void ipc::ipc_Message_Dump()
{
    for(int i = 0; i < max_tasks; i++)
    {
        Message_Print(i);
    }
}