#ifndef QUEUE_H
#define QUEUE_H

#include <iostream>
#include <sstream>
using namespace std;

#define TRUE 1
#define FALSE 0

template <class TYPE>
class Queue {

protected:   // changed from private → protected for Mailbox inheritance
    struct node {
        TYPE value;
        node *next;
    };

    node *head;
    node *tail;
    int   size;

public:

    // Constructor
    // ------------------------------------------------
    Queue() {
        head = nullptr;
        tail = nullptr;
        size = 0;
    }

    // Destructor
    // ------------------------------------------------
    ~Queue() {
        Reset();
    }

    // Add item to queue
    // ------------------------------------------------
    void En_Q(TYPE value) {
        node *temp = new node;
        temp->value = value;
        temp->next  = nullptr;

        if (tail == nullptr) {
            head = tail = temp;
        } else {
            tail->next = temp;
            tail = temp;
        }

        size++;
        cout << "En_Q operation completed " << endl;
    }

    // Remove item from queue
    // ------------------------------------------------
    TYPE De_Q() {
        if (head == nullptr) {
            cout << "De_Q ERROR: Queue is empty!" << endl;
            return TYPE();
        }

        node *temp = head;
        TYPE value = temp->value;

        head = head->next;
        if (head == nullptr)
            tail = nullptr;

        delete temp;
        size--;

        cout << "De_Q operation completed " << endl;
        return value;
    }

    // Check if empty
    // ------------------------------------------------
    int isEmpty() {
        return (size == 0);
    }

    // Reset queue
    // ------------------------------------------------
    void Reset() {
        while (!isEmpty())
            De_Q();
    }

    // Print queue contents
    // ------------------------------------------------
    void Print() {
        cout << "Queue Size: " << size << endl;
        cout << "Queue Head <- ";

        node *ptr = head;
        while (ptr != nullptr) {
            cout << "[item] <- ";
            ptr = ptr->next;
        }

        cout << "Tail" << endl;
    }

    // Return queue as string
    // ------------------------------------------------
    string Get_Q_String() {
        stringstream ss;
        node *ptr = head;

        while (ptr != nullptr) {
            ss << "[item] ";
            ptr = ptr->next;
        }

        return ss.str();
    }
};

#endif
