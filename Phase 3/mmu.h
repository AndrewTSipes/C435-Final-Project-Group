#ifndef MMU_H
#define MMU_H

#include <iostream>
#include "Sched.h"     // ===== PHASE 3 ADDITION =====
#include "Sema.h"      // ===== PHASE 3 ADDITION =====
using namespace std;

//Represents one chunk of memory
class MemoryBlock {
public:
    int handle;
    int start;
    int end;
    int size;
    int current_location;
    int task_id;
    bool is_free;
    MemoryBlock* next;

    MemoryBlock(int h, int s, int e, int sz, int owner, bool freeFlag);
    bool isValidAccess(int offset);
    void resetCurrent();
};

class mmu {
private:
    char* memory;
    int mem_size;
    int block_size;
    int next_handle;
    char default_initial_value;
    MemoryBlock* head;

    MemoryBlock* findBlock(int memory_handle);

    // ===== PHASE 3 ADDITIONS =====
    scheduler* sched_ptr;        // so MMU can unblock tasks
    semaphore* core_sema;        // protects memory operations

public:

    // ===== PHASE 3 ADDITION =====
    // Constructor now accepts scheduler + semaphore
    mmu(int size, char default_initial_value, int block_size,
        scheduler* sched = nullptr,
        semaphore* core = nullptr);

    ~mmu();

    int Mem_Alloc(int size);
    int Mem_Free(int memory_handle);

    int Mem_Read(int memory_handle, char* ch);
    int Mem_Write(int memory_handle, char ch);

    int Mem_Read(int memory_handle, int offset_from_beg, int text_size, char* text);
    int Mem_Write(int memory_handle, int offset_from_beg, int text_size, char* text);

    int Mem_Left();
    int Mem_Largest();
    int Mem_Smallest();
    int Mem_Coalesce();
    int Mem_Dump(int starting_from, int num_bytes);

    void printBlocks();
    int ResetCurrent(int memory_handle);

    // ===== PHASE 3 ADDITIONS =====
    char* getMemoryPtr() { return memory; }     // for ncurses dump
    MemoryBlock* getHead() { return head; }     // for block list window
};

#endif
