#ifndef MMU_H
#define MMU_H

#include <iostream>
using namespace std;

//Represents one chunk of memory
//Each block is either free or allocated
//We connect the blocks using a linked list
class MemoryBlock {
    public:
        int handle; //ID for allocated blocks
        int start; //Starting index for memroy array
        int end; //Ending index for memory array
        int size; //Size of the block in bytes
        int current_location; //Current read / write position
        int task_id; //Owner of the block (-1 = Free)
        bool is_free; // T = Free , F = Allocated
        MemoryBlock* next; //Points to next block in list

        //Constructor for initializing a mem. block
        MemoryBlock(int h, int s, int e, int sz, int owner, bool freeFlag);
        bool isValidAccess(int offset); //Checks offset value
        void resetCurrent(); //Resets current_location
};

class mmu {
    private:
        char* memory; //Actual memory array
        int mem_size; //Size of memory
        int block_size; //Allocation unit
        int next_handle; //Used to make unique handles
        MemoryBlock* head; //Start of linked list
        MemoryBlock* findBlock(int memory_handle); //Helper to find block by its handle

    public:

        //Initializes memory and creates initial free block
        mmu(int size, char default_initial_value, int block_size);


        //Destructor
        ~mmu();

        //Allocates memory and returns a handle
        int Mem_Alloc(int size); 

        //Frees memory associated with a handle
        int Mem_Free(int memory_handle);

        //Single letter read / write. Uses current location
        int Mem_Read(int memory_handle, char* ch);
        int Mem_Write(int memory_handle, char ch);

        //Multi letter read / write. Uses offset that starts at beginning of block
        int Mem_Read(int memory_handle, int offset_from_beg, int text_size, char* text);
        int Mem_Write(int memory_handle, int offset_from_beg, int text_size, char* text);

        //Memory stats
        int Mem_Left(); //Free memory
        int Mem_Largest(); //Largest free block
        int Mem_Smallest(); //Smallest free block
        int Mem_Coalesce(); //Merges adjacent free blocks
        int Mem_Dump(int starting_from, int num_bytes); //Prints raw memory

        void printBlocks(); //prints block list for debugging
        int ResetCurrent(int memory_handle); //Resets the current_location of a block
};
#endif