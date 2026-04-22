#include "mmu.h"

//MemoryBlock Functions


//Constructor for a single memory block in the linked list
MemoryBlock::MemoryBlock(int h ,int s, int e, int sz, int owner, bool freeFlag) {
    handle = h; //handle for allocated block
    start = s; //starting index in arrays
    end = e; //ending index in arrays
    size = sz; //size of block in bytes
    current_location = s; //current position (for read/write)
    task_id = owner; //owner of the block, -1 means it is free
    is_free = freeFlag; //true means free, false means it is already allocated
    next = nullptr; //points to nect block in the linked list
}

//Checks if offset is valid in the block, returns true if so
//Returns false if out of bounds
bool MemoryBlock::isValidAccess(int offset) {
    return offset >= 0 && offset < size;
}


//Puts the position to the beginning of the block
void MemoryBlock::resetCurrent() {
    current_location = start;
}


//MMU Functions

//MMU Constructor
//Sets memory size, block size, initializes the array for memory, 
//gives it a default character, and creates a large free block
mmu::mmu(int size, char default_initial_value, int block_size) {
    mem_size = size; //Total memory size
    this->block_size = block_size; //had to do this-> to refer to variable
    next_handle = 1; //first memory handle starts at 1

    //Allocated the memory array
    memory = new char[mem_size];

    //Fills memory with dots (......)
    //Dots will represent unused memory and hashtags will be freed memory
    for (int i=0; i < mem_size; i++) {
        memory[i] = default_initial_value;
    }
    
    //Creating the initial memory block.
    //Starts at handle 0. Task ID is -1 which means free
    head = new MemoryBlock(0, 0, mem_size -1, mem_size, -1, true);
}


//Destructor
//Deletes the nodes in the linked list and frees the memory array
mmu::~mmu() {
    delete[] memory;

    MemoryBlock* current = head;
    while (current != nullptr) {
        MemoryBlock* temp = current;
        current = current->next;
        delete temp;
    }
}


//Dumps memory and displays on screen (for debugging)
int mmu::Mem_Dump(int starting_from, int num_bytes) {
    if(starting_from < 0 || starting_from >= mem_size) { //Position error check
        return -1;
    }

    // Determines the ending position
    int end = starting_from + num_bytes; 
    if (end > mem_size) {
        end = mem_size;
    }

    //For loop to print each character in the selected memory
    for (int i = starting_from; i < end; i++) {
        cout << memory[i];

        //This is to print a newline every 64 characters
        //Makes the output way cleaner
        if ((i - starting_from +1)%64==0) {
            cout << endl;
        }
    }
    cout << endl;
    return 0;
}


//Prints linked list of the memory blocks
//This is for testing so we can see what blocks are free or allocated
void mmu::printBlocks() {
    MemoryBlock* current = head;

    cout << "\n -- Memory Blocks -- \n";

    //This goes through each memory block and prints the information below
    while(current != nullptr) {
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



//This searches the linked list for a block with the given handle
MemoryBlock* mmu::findBlock(int memory_handle) {
    MemoryBlock* current = head;

    while (current != nullptr) { //Will not return a free block
        if (!current->is_free && current->handle == memory_handle) {
            return current;
        }
        current = current->next;
    }

    return nullptr; //No match found
}



//Uses First-fit method 
//Returns a unique handle if successful, -1 if fails
int mmu::Mem_Alloc(int size) {
    //Denies invalid request size
    if (size <=0) {
        return -1;
    }

    //rounding up to nearest block sz
    int alloc_size = ((size + block_size - 1) / block_size) * block_size;

    MemoryBlock* current = head;

    
    //Scans linked list from the front and uses the first compatible + free block
    //This is the "First-fit" strategy
    while (current != nullptr) {
        if(current->is_free && current->size >= alloc_size) {
            //exact fit
            if(current->size == alloc_size) {
               current->is_free = false;
               current->handle = next_handle++;
               current->task_id = 1; //debug
               current->current_location = current -> start;
               return current->handle;
            }

            //If block is larged than needed, we split it
            //We will split it into a new used block and a free block
            MemoryBlock* newBlock = new MemoryBlock( 
                next_handle++, //New handle
                current->start, //Sets the new used block where the free block was
                current->start + alloc_size -1, //Gets new ending index
                alloc_size, // size of a newly allocated block
                1, //placeholder task id for testing purposes
                false //means the block is allocated already
            );

            //Shrinks the current free block so it begins right after the new used block
            current->start = newBlock->end +1;
            current->size = current->end - current->start +1;
            current->current_location = current->start;


            //Links the new block to the free block 
            newBlock->next = current;

            //If we split it, we need to reassign the head
            if(head == current) {
               head = newBlock;
            } else { //Else we search for newBlock to insert
                MemoryBlock* prev = head;
                while(prev->next != current) { 
                    prev = prev->next;
                }
                prev->next = newBlock;
            }
            return newBlock->handle;
        }
        current = current->next;
    }
    return -1; //Returns -1 if out of memory
}


//Frees an allocated block
//Fills the memory with '#'s to represent free memory
int mmu::Mem_Free(int memory_handle) {
    MemoryBlock* current = head;

    while (current != nullptr) {
        if (!current->is_free && current->handle == memory_handle) {


            //Makes all the bytes '#', which means free memory
            for(int i= current->start; i<=current->end; i++) {
                memory[i] = '#';
            }

            current->is_free = true;
            current->task_id = -1;
            current->handle =0;
            current->current_location = current->start;

            Mem_Coalesce();
            return 0;
        }
        current = current->next;
    }
    return -1; //returns -1 if handle could not be found
}


//Merges the adjacent free blocks into larger free blocks
int mmu::Mem_Coalesce() {
    MemoryBlock* current = head;

    while (current != nullptr && current->next != nullptr) {
        //if both are free it merges them
        if(current->is_free && current->next->is_free) {
            MemoryBlock* temp = current->next;

            current->end = temp->end;
            current->size = current->end - current-> start +1;
            current->next = temp->next;
            current->current_location = current->start;

            delete temp; //Cleans up by deleting the absorbed node
        } else {
            current = current->next; //Else it moves forward in the list
        }
    }
    return 0;
}


//Returns the number of free bytes left in memory
int mmu::Mem_Left() {
    int total = 0;
    MemoryBlock* current = head;

    while (current != nullptr) {
        if (current->is_free) {
            total += current->size;
        }
        current = current->next;
    }

    return total;
}

//Returns the size of the largest free block
int mmu::Mem_Largest() {
    int largest = 0;
    MemoryBlock* current = head;

    while (current != nullptr) {
        if (current->is_free && current->size > largest) {
            largest = current->size;
        }
        current = current->next;
    }

    return largest;
}

//Returns the size of the smallest free block
int mmu::Mem_Smallest() {
    int smallest = -1;
    MemoryBlock* current = head;

    while (current != nullptr) {
        if (current->is_free) {
            if (smallest == -1 || current->size < smallest) {
                smallest = current->size;
            }
        }
        current = current->next;
    }

    return smallest;
}


//Writes a character into the allocated block's current location
int mmu::Mem_Write(int memory_handle, char ch) {
    MemoryBlock* block = findBlock(memory_handle);

    if (block == nullptr) {
        return -1; //invalid handle
    }

    if (block->current_location > block->end) {
        return -1; //out of bounds
    }

    memory[block->current_location] = ch;
    block->current_location++; //After writing it moves forward by 1 for the next letter

    return 0;
}


//Reads a letter from the allocated block's current position
int mmu::Mem_Read(int memory_handle, char* ch) {
    MemoryBlock* block = findBlock(memory_handle);

    if (block == nullptr || ch == nullptr) {
        return -1; //invalid handle or bad pointer
    }

    if (block->current_location > block->end) {
        return -1; //out of bounds
    }

    *ch = memory[block->current_location];
    block->current_location++; //After reading it moves forward by 1 to prepare for the next letter

    return 0;
}


//Resets the current location back to start, needed for testing 
int mmu::ResetCurrent(int memory_handle) {
    MemoryBlock* block = findBlock(memory_handle);

    if (block == nullptr) {
        return -1;
    }

    block->resetCurrent();
    return 0;
}


//Writes based on offset and can write multiple characters
int mmu::Mem_Write(int memory_handle, int offset_from_beg, int text_size, char* text) {
    MemoryBlock* block = findBlock(memory_handle);

    //Make sure handle is good
    if (block == nullptr) {
        return -1;
    }

    //Rejects invalid offset or size (negative numbers)
    if (offset_from_beg < 0 || text_size < 0) {
        return -1;
    }


    //Makes sure write stays inside bounds
    if (offset_from_beg + text_size > block->size) {
        return -1; //out of bounds
    }


    //Converts offset into memory index
    int start_pos = block->start + offset_from_beg;

    //Copying letters into memory using for loop
    for (int i = 0; i < text_size; i++) {
        memory[start_pos + i] = text[i];
    }

    return 0;
}


//Reads multiple chars. at a time, relies on offset
int mmu::Mem_Read(int memory_handle, int offset_from_beg, int text_size, char* text) {
    MemoryBlock* block = findBlock(memory_handle);

    //Testing handle again
    if (block == nullptr) {
        return -1;
    }


    //Catches negative offset or size
    if (offset_from_beg < 0 || text_size < 0) {
        return -1;
    }

    if (offset_from_beg + text_size > block->size) {
        return -1; //would go past limit
    }

    //Converts offset into memory index again
    int start_pos = block->start + offset_from_beg;

    //Copies chars. from memory into buffer
    for (int i = 0; i < text_size; i++) {
        text[i] = memory[start_pos + i];
    }

    return 0;
}