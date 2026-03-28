#include "Sched.h"
#include "Sema.h"

int main() {
    scheduler s;

    cout << "\n=== PHASE 1 DEMO START ===\n";


    // 1. Create tasks
    // ------------------------------------------------
    s.create_task("A");
    s.create_task("B");
    s.create_task("C");

    cout << "\nInitial process table:\n";
    s.dump(0);


    // 2. Basic round-robin scheduling
    // ------------------------------------------------
    cout << "\n--- Testing yield() ---\n";
    s.yield();
    s.dump(0);

    s.yield();
    s.dump(0);

    s.yield();
    s.dump(0);

    
    // 3. Semaphore blocking/unblocking
    // ------------------------------------------------
    semaphore sem(1, "R1", &s);

    cout << "\n----- A acquires R1 -----\n";
    sem.down(0);
    s.dump(0);

    cout << "\n----- B tries to acquire R1 (BLOCK) -----\n";
    sem.down(1);
    s.dump(0);

    cout << "\n----- C tries to acquire R1 (BLOCK) -----\n";
    sem.down(2);
    s.dump(0);

    cout << "\n----- A releases R1 (unblocks B) -----\n";
    sem.up();
    s.dump(0);

    cout << "\n----- A releases R1 again (unblocks C) -----\n";
    sem.up();
    s.dump(0);

    
    // 4. Kill a task + garbage collect
    // ------------------------------------------------
    cout << "\n----- Killing task B -----\n";
    s.kill_task(1);
    s.dump(0);

    cout << "\n----- Running garbage_collect() -----\n";
    s.garbage_collect();
    s.dump(0);

    
    // 5. Final scheduling after cleanup
    // ------------------------------------------------
    cout << "\n----- Final yield() test -----\n";
    s.yield();
    s.dump(0);

    cout << "\n----- PHASE 1 Complete -----\n";

    return 0;
}