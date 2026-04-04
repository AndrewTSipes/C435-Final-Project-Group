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

void Mailbox::print_all()
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
ipc::ipc(int max_tasks, int &error_code) : max_tasks(max_tasks)
{
    mailbox = new Mailbox[max_tasks];
    msg_count = new int[max_tasks]{};
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
    mailbox[Destination_Id].En_Q(
        Message {
            Sender_Id,
            Destination_Id,
            time(nullptr),
            msg_type,
            std::strlen(message),
            message,
        }
    );
    msg_count[Destination_Id]++;
    return 1;
}

int ipc::Message_Receive(int Task_Id, char* message, int *msg_type)
{
    if(mailbox[Task_Id].isEmpty()) return 0;
    Message msg = mailbox[Task_Id].De_Q();
    message = msg.Msg_Text;
    *msg_type = msg.Msg_Type.Message_Type_Id;
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