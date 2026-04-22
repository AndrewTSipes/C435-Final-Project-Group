#include <iostream>
#include "mmu.h"
using namespace std;


int main() {
    mmu mem(1024, '.', 64); //Start by creating MMU with 1024 bytes total, default '.', and block size 64


    cout << "--- [PHASE 3 MMU DEBUG SESSION] ---\n";

    //Initial State
    cout << "\n[Initial State]\n";
    mem.printBlocks();
    mem.Mem_Dump(0, 128);

    //Allocates 3 blocks (h1, h2, h3)
    cout << "\n[Allocation]\n";
    int h1 = mem.Mem_Alloc(35);
    int h2 = mem.Mem_Alloc(100);
    int h3 = mem.Mem_Alloc(50);

    cout << "h1 = " << h1 << ", h2 = " << h2 << ", h3 = " << h3 << endl;
    mem.printBlocks();

    //Single-char Write/Read
    cout << "\n[Single Char Write / Read]\n";
    mem.Mem_Write(h1, 'T');
    mem.Mem_Write(h1, 'E');
    mem.Mem_Write(h1, 'S');
    mem.Mem_Write(h1, 'T');
    mem.Mem_Write(h1, '!');

    //Prints memory
    mem.Mem_Dump(0, 128);


    //Resets block's current read / write position
    mem.ResetCurrent(h1);
    char ch;
    cout << "Read back: ";
    while (mem.Mem_Read(h1, &ch) == 0) {
        cout << ch;
        if (ch == '!') break;
    }
    cout << endl;

    //Multi-char Write/Read
    cout << "\n[Multi Char Write / Read]\n";
    char msg[] = "ULTIMA";
    mem.Mem_Write(h2, 0, 6, msg);


    //Prints updated memory
    mem.Mem_Dump(0, 256);

    char buffer[7];
    buffer[6] = '\0';
    mem.Mem_Read(h2, 0, 6, buffer);

    cout << "Read back: " << buffer << endl;

    
    
    //Free blocks and shows merging of adjacent blocks
    cout << "\n[Free + Coalesce]\n";
    mem.Mem_Free(h1);
    mem.printBlocks();

    mem.Mem_Free(h2);
    mem.printBlocks();

    
    //Memory Stats
    cout << "\n[Memory Stats]\n";
    cout << "Memory left: " << mem.Mem_Left() << endl;
    cout << "Largest block: " << mem.Mem_Largest() << endl;
    cout << "Smallest block: " << mem.Mem_Smallest() << endl;

    
    //Failure Case
    cout << "\n[Failure Case]\n";
    int bad = mem.Mem_Alloc(2000); //Produces error, 2000 is too much
    cout << "Alloc too large result: " << bad << endl;

    cout << "\n--- [TEST END] ---\n";

    return 0;
}