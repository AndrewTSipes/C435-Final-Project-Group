#include "mmu.h"

// ------------------------------------------------------------
// MemoryBlock Functions
// ------------------------------------------------------------

MemoryBlock::MemoryBlock(int h ,int s, int e, int sz, int owner, bool freeFlag) {
    handle = h;
    start = s;
    end = e;
    size = sz;
    current_location = s;
    task_id = owner;
    is_free = freeFlag;
    next = nullptr;
}

bool MemoryBlock::isValidAccess(int offset) {
    return offset >= 0 && offset < size;
}

void MemoryBlock::resetCurrent() {
    current_location = start;
}

// ------------------------------------------------------------
// MMU Constructor (Phase 3 updated)
// ------------------------------------------------------------
mmu::mmu(int size, char default_initial_value, int block_size,
         scheduler* sched, semaphore* core)
{
    mem_size = size;
    this->block_size = block_size;
    next_handle = 1;

    // ===== PHASE 3 ADDITION =====
    sched_ptr = sched;
    core_sema = core;

    memory = new char[mem_size];

    for (int i = 0; i < mem_size; i++)
        memory[i] = default_initial_value;

    head = new MemoryBlock(0, 0, mem_size - 1, mem_size, -1, true);
}

// ------------------------------------------------------------
mmu::~mmu() {
    delete[] memory;

    MemoryBlock* current = head;
    while (current != nullptr) {
        MemoryBlock* temp = current;
        current = current->next;
        delete temp;
    }
}

// ------------------------------------------------------------
// PHASE 3 semaphore wrappers
// ------------------------------------------------------------
#define CORE_DOWN(task) if(core_sema) core_sema->down(task)
#define CORE_UP()       if(core_sema) core_sema->up()

// ------------------------------------------------------------
// Helper: find block by handle
// ------------------------------------------------------------
MemoryBlock* mmu::findBlock(int memory_handle) {
    MemoryBlock* current = head;

    while (current != nullptr) {
        if (!current->is_free && current->handle == memory_handle)
            return current;
        current = current->next;
    }
    return nullptr;
}

// ------------------------------------------------------------
// Mem_Alloc
// ------------------------------------------------------------
int mmu::Mem_Alloc(int size) {

    int task_id = sched_ptr ? sched_ptr->get_task_id() : 0;
    CORE_DOWN(task_id);

    if (size <= 0) {
        CORE_UP();
        return -1;
    }

    int alloc_size = ((size + block_size - 1) / block_size) * block_size;

    MemoryBlock* current = head;

    while (current != nullptr) {

        if (current->is_free && current->size >= alloc_size) {

            // exact fit
            if (current->size == alloc_size) {
                current->is_free = false;
                current->handle = next_handle++;
                current->task_id = task_id;
                current->current_location = current->start;

                // ===== PHASE 3 ADDITION =====
                if (sched_ptr) {
                    tcb* t = sched_ptr->current;
                    t->memory_handle = current->handle;
                    t->base = current->start;
                    t->limit = current->end;
                    t->current_location = current->start;
                    t->waiting_for_memory = false;
                }

                CORE_UP();
                return current->handle;
            }

            // split block
            MemoryBlock* newBlock = new MemoryBlock(
                next_handle++,
                current->start,
                current->start + alloc_size - 1,
                alloc_size,
                task_id,
                false
            );

            current->start = newBlock->end + 1;
            current->size = current->end - current->start + 1;
            current->current_location = current->start;

            newBlock->next = current;

            if (head == current)
                head = newBlock;
            else {
                MemoryBlock* prev = head;
                while (prev->next != current)
                    prev = prev->next;
                prev->next = newBlock;
            }

            // ===== PHASE 3 ADDITION =====
            if (sched_ptr) {
                tcb* t = sched_ptr->current;
                t->memory_handle = newBlock->handle;
                t->base = newBlock->start;
                t->limit = newBlock->end;
                t->current_location = newBlock->start;
                t->waiting_for_memory = false;
            }

            CORE_UP();
            return newBlock->handle;
        }

        current = current->next;
    }

    // ===== PHASE 3 ADDITION =====
    // Not enough memory → block task (shark pond)
    if (sched_ptr) {
        tcb* t = sched_ptr->current;
        t->waiting_for_memory = true;
        sched_ptr->set_state(task_id, BLOCKED);
        sched_ptr->yield();
    }

    CORE_UP();
    return -1;
}

// ------------------------------------------------------------
// Mem_Free
// ------------------------------------------------------------
int mmu::Mem_Free(int memory_handle) {

    int task_id = sched_ptr ? sched_ptr->get_task_id() : 0;
    CORE_DOWN(task_id);

    MemoryBlock* current = head;

    while (current != nullptr) {

        if (!current->is_free && current->handle == memory_handle) {

            for (int i = current->start; i <= current->end; i++)
                memory[i] = '#';

            current->is_free = true;
            current->task_id = -1;
            current->handle = 0;
            current->current_location = current->start;

            Mem_Coalesce();

            // ===== PHASE 3 ADDITION =====
            // Unblock tasks waiting for memory
            if (sched_ptr) {
                tcb* ptr = sched_ptr->current;
                // Scheduler will handle waking tasks in main loop
            }

            CORE_UP();
            return 0;
        }

        current = current->next;
    }

    CORE_UP();
    return -1;
}

// ------------------------------------------------------------
// Mem_Coalesce
// ------------------------------------------------------------
int mmu::Mem_Coalesce() {
    MemoryBlock* current = head;

    while (current != nullptr && current->next != nullptr) {

        if (current->is_free && current->next->is_free) {
            MemoryBlock* temp = current->next;

            current->end = temp->end;
            current->size = current->end - current->start + 1;
            current->next = temp->next;
            current->current_location = current->start;

            delete temp;
        } else {
            current = current->next;
        }
    }
    return 0;
}

// ------------------------------------------------------------
// Mem_Left
// ------------------------------------------------------------
int mmu::Mem_Left() {
    int total = 0;
    MemoryBlock* current = head;

    while (current != nullptr) {
        if (current->is_free)
            total += current->size;
        current = current->next;
    }

    return total;
}

// ------------------------------------------------------------
// Mem_Largest
// ------------------------------------------------------------
int mmu::Mem_Largest() {
    int largest = 0;
    MemoryBlock* current = head;

    while (current != nullptr) {
        if (current->is_free && current->size > largest)
            largest = current->size;
        current = current->next;
    }

    return largest;
}

// ------------------------------------------------------------
// Mem_Smallest
// ------------------------------------------------------------
int mmu::Mem_Smallest() {
    int smallest = -1;
    MemoryBlock* current = head;

    while (current != nullptr) {
        if (current->is_free) {
            if (smallest == -1 || current->size < smallest)
                smallest = current->size;
        }
        current = current->next;
    }

    return smallest;
}

// ------------------------------------------------------------
// Mem_Write (single char)
// ------------------------------------------------------------
int mmu::Mem_Write(int memory_handle, char ch) {

    int task_id = sched_ptr ? sched_ptr->get_task_id() : 0;
    CORE_DOWN(task_id);

    MemoryBlock* block = findBlock(memory_handle);

    if (!block) {
        CORE_UP();
        return -1;
    }

    // ===== PHASE 3 ADDITION =====
    if (block->task_id != task_id) {
        CORE_UP();
        return -1; // segmentation fault
    }

    if (block->current_location > block->end) {
        CORE_UP();
        return -1;
    }

    memory[block->current_location] = ch;
    block->current_location++;

    CORE_UP();
    return 0;
}

// ------------------------------------------------------------
// Mem_Read (single char)
// ------------------------------------------------------------
int mmu::Mem_Read(int memory_handle, char* ch) {

    int task_id = sched_ptr ? sched_ptr->get_task_id() : 0;
    CORE_DOWN(task_id);

    MemoryBlock* block = findBlock(memory_handle);

    if (!block || !ch) {
        CORE_UP();
        return -1;
    }

    // ===== PHASE 3 ADDITION =====
    if (block->task_id != task_id) {
        CORE_UP();
        return -1; // segmentation fault
    }

    if (block->current_location > block->end) {
        CORE_UP();
        return -1;
    }

    *ch = memory[block->current_location];
    block->current_location++;

    CORE_UP();
    return 0;
}

// ------------------------------------------------------------
// ResetCurrent
// ------------------------------------------------------------
int mmu::ResetCurrent(int memory_handle) {
    MemoryBlock* block = findBlock(memory_handle);

    if (!block)
        return -1;

    block->resetCurrent();
    return 0;
}

// ------------------------------------------------------------
// Mem_Write (multi-byte)
// ------------------------------------------------------------
int mmu::Mem_Write(int memory_handle, int offset_from_beg, int text_size, char* text) {

    int task_id = sched_ptr ? sched_ptr->get_task_id() : 0;
    CORE_DOWN(task_id);

    MemoryBlock* block = findBlock(memory_handle);

    if (!block) {
        CORE_UP();
        return -1;
    }

    // ===== PHASE 3 ADDITION =====
    if (block->task_id != task_id) {
        CORE_UP();
        return -1; // segmentation fault
    }

    if (offset_from_beg < 0 || text_size < 0) {
        CORE_UP();
        return -1;
    }

    if (offset_from_beg + text_size > block->size) {
        CORE_UP();
        return -1;
    }

    int start_pos = block->start + offset_from_beg;

    for (int i = 0; i < text_size; i++)
        memory[start_pos + i] = text[i];

    CORE_UP();
    return 0;
}

// ------------------------------------------------------------
// Mem_Read (multi-byte)
// ------------------------------------------------------------
int mmu::Mem_Read(int memory_handle, int offset_from_beg, int text_size, char* text) {

    int task_id = sched_ptr ? sched_ptr->get_task_id() : 0;
    CORE_DOWN(task_id);

    MemoryBlock* block = findBlock(memory_handle);

    if (!block) {
        CORE_UP();
        return -1;
    }

    // ===== PHASE 3 ADDITION =====
    if (block->task_id != task_id) {
        CORE_UP();
        return -1; // segmentation fault
    }

    if (offset_from_beg < 0 || text_size < 0) {
        CORE_UP();
        return -1;
    }

    if (offset_from_beg + text_size > block->size) {
        CORE_UP();
        return -1;
    }

    int start_pos = block->start + offset_from_beg;

    for (int i = 0; i < text_size; i++)
        text[i] = memory[start_pos + i];

    CORE_UP();
    return 0;
}

// ------------------------------------------------------------
// Mem_Dump
// ------------------------------------------------------------
int mmu::Mem_Dump(int starting_from, int num_bytes) {

    if (starting_from < 0 || starting_from >= mem_size)
        return -1;

    int end = starting_from + num_bytes;
    if (end > mem_size)
        end = mem_size;

    for (int i = starting_from; i < end; i++) {
        cout << memory[i];
        if ((i - starting_from + 1) % 64 == 0)
            cout << endl;
    }

    cout << endl;
    return 0;
}

// ------------------------------------------------------------
// printBlocks
// ------------------------------------------------------------
void mmu::printBlocks() {
    MemoryBlock* current = head;

    cout << "\n -- Memory Blocks -- \n";

    while (current != nullptr) {
        cout << (current->is_free ? "Free" : "Used")
             << " | Handle: " << current->handle
             << " | Start: " << current->start
             << " | End: " << current->end
             << " | Size: " << current->size
             << " | Current: " << current->current_location
             << " | Task: " << current->task_id
             << endl;

        current = current->next;
    }
}
