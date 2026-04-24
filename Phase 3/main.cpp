#include "Sched.h"
#include "Sema.h"
#include "IPC.h"
#include "mmu.h"   // ===== PHASE 3 ADDITION =====
#include <ncurses.h>
#include <pthread.h>
#include <unistd.h>
#include <string>
#include <vector>

// Global mutex for ncurses drawing (from Lab 7)
pthread_mutex_t myMutex = PTHREAD_MUTEX_INITIALIZER;

// -----------------------------
// Safe window write helpers
// -----------------------------
void safe_write(WINDOW* win, const char* text) {
    pthread_mutex_lock(&myMutex);
    wmove(win, 1, 1);
    wprintw(win, "%s", text);
    box(win, 0, 0);
    wrefresh(win);
    pthread_mutex_unlock(&myMutex);
}

void safe_write_xy(WINDOW* win, int y, int x, const char* text) {
    pthread_mutex_lock(&myMutex);
    mvwprintw(win, y, x, text);
    box(win, 0, 0);
    wrefresh(win);
    pthread_mutex_unlock(&myMutex);
}

// -----------------------------
// Create window wrapper
// -----------------------------
WINDOW* create_window(int h, int w, int y, int x) {
    pthread_mutex_lock(&myMutex);
    WINDOW* win = newwin(h, w, y, x);
    scrollok(win, TRUE);
    box(win, 0, 0);
    wrefresh(win);
    pthread_mutex_unlock(&myMutex);
    return win;
}

// -----------------------------
// MAIN
// -----------------------------
int main() {

    // -----------------------------
    // Initialize ncurses
    // -----------------------------
    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);

    // -----------------------------
    // CLEAN, NON-OVERLAPPING LAYOUT
    // -----------------------------
    WINDOW* Heading_Win = create_window(7, 80, 1, 2);
    safe_write_xy(Heading_Win, 1, 20, "ULTIMA 2.0 (Phase 3 Demo)");
    safe_write_xy(Heading_Win, 2, 2, "q = quit, y = yield, d = dump scheduler, g = garbage collect");
    safe_write_xy(Heading_Win, 3, 2, "1/2/3 = kill A/B/C, 4 = A->B, 5 = C->B, 6 = B recv, 7 = B count");
    safe_write_xy(Heading_Win, 4, 2, "m = alloc, f = free, w = write, r = read, D = dump mem, B = blocks");

    // Task windows
    std::vector<WINDOW*> task_windows;
    task_windows.push_back(create_window(8, 25, 9, 2));
    task_windows.push_back(create_window(8, 25, 9, 28));
    task_windows.push_back(create_window(8, 25, 9, 54));

    safe_write(task_windows[0], "Task A Window\n");
    safe_write(task_windows[1], "Task B Window\n");
    safe_write(task_windows[2], "Task C Window\n");

    // Log window
    WINDOW* Log_Win = create_window(12, 80, 18, 2);
    safe_write(Log_Win, "Log Window Initialized...\n");

    // Console window
    WINDOW* Console_Win = create_window(6, 80, 31, 2);
    keypad(Console_Win, TRUE);
    wtimeout(Console_Win, -1);
    safe_write_xy(Console_Win, 1, 1, "Console Ready");
    safe_write_xy(Console_Win, 2, 1, "Ultima # ");
    wmove(Console_Win, 2, 10);

    // ===== PHASE 3 ADDITION =====
    WINDOW* MemDump_Win = create_window(12, 80, 38, 2);
    safe_write(MemDump_Win, "Memory Dump Window Ready...\n");

    WINDOW* BlockList_Win = create_window(12, 80, 51, 2);
    safe_write(BlockList_Win, "Block List Window Ready...\n");

    // -----------------------------
    // Create scheduler + tasks
    // -----------------------------
    scheduler S;
    S.set_log_window(Log_Win);

    int tA = S.create_task("A");
    int tB = S.create_task("B");
    int tC = S.create_task("C");

    // ------------------------------
    // Create IPC
    // ------------------------------
    int ipc_error = 0;
    ipc messenger(3, ipc_error, &S);

    // -----------------------------
    // Semaphore for testing
    // -----------------------------
    semaphore sem(1, "R1", &S);
    sem.set_log_window(Log_Win);

    // ===== PHASE 3 ADDITION =====
    semaphore CoreMemory(1, "Core Memory", &S);
    CoreMemory.set_log_window(Log_Win);

    mmu MemMgr(1024, '.', 64, &S, &CoreMemory);

    // -----------------------------
    // Main input loop
    // -----------------------------
    int ch;
    while ((ch = wgetch(Console_Win)) != 'q') {

        switch (ch) {

            case 'y':
                safe_write(Log_Win, "Yield called...\n");
                S.yield();
                break;

            case 'd':
                safe_write(Log_Win, "Scheduler dump:\n");
                S.dump_to_window(Log_Win);
                break;

            case 'g':
                safe_write(Log_Win, "Running garbage_collect...\n");
                S.garbage_collect();
                break;

            case '1':
                safe_write(Log_Win, "Killing Task A...\n");
                S.kill_task(tA);
                break;

            case '2':
                safe_write(Log_Win, "Killing Task B...\n");
                S.kill_task(tB);
                break;

            case '3':
                safe_write(Log_Win, "Killing Task C...\n");
                S.kill_task(tC);
                break;

            case '4':
                messenger.Message_Send(tA, tB, (char*)"Hello from A...", 0);
                safe_write(Log_Win, "A sent message to B.\n");
                break;

            case '5':
                messenger.Message_Send(tC, tB, (char*)"Hello from C...", 0);
                safe_write(Log_Win, "C sent message to B.\n");
                break;

            case '6': {
                char buffer[128];
                int msg_type;
                if (messenger.Message_Receive(tB, buffer, &msg_type))
                    safe_write(Log_Win, "Task B received a message.\n");
                else
                    safe_write(Log_Win, "Task B has no messages.\n");
                break;
            }

            case '7': {
                char msg[100];
                sprintf(msg, "Mailbox B count: %d\n", messenger.Message_Count(tB));
                safe_write(Log_Win, msg);
                break;
            }

            // ===== PHASE 3 MEMORY COMMANDS =====

            case 'm': {
                safe_write(Log_Win, "Allocating 64 bytes...\n");
                int h = MemMgr.Mem_Alloc(64);
                char msg[80];
                sprintf(msg, "Mem_Alloc returned handle %d\n", h);
                safe_write(Log_Win, msg);
                break;
            }

            case 'f': {
                safe_write(Log_Win, "Freeing handle 1...\n");
                MemMgr.Mem_Free(1);
                break;
            }

            case 'w': {
                safe_write(Log_Win, "Writing 'X' to handle 1...\n");
                MemMgr.Mem_Write(1, 'X');
                break;
            }

            case 'r': {
                char ch;
                if (MemMgr.Mem_Read(1, &ch) == 0) {
                    char msg[80];
                    sprintf(msg, "Read char: %c\n", ch);
                    safe_write(Log_Win, msg);
                } else {
                    safe_write(Log_Win, "Read failed.\n");
                }
                break;
            }

            case 'D': {
                wclear(MemDump_Win);
                box(MemDump_Win, 0, 0);
                wmove(MemDump_Win, 1, 1);

                char* mem = MemMgr.getMemoryPtr();
                for (int i = 0; i < 256; i++)
                    waddch(MemDump_Win, mem[i]);

                wrefresh(MemDump_Win);
                break;
            }

            case 'B': {
                wclear(BlockList_Win);
                box(BlockList_Win, 0, 0);

                MemoryBlock* cur = MemMgr.getHead();
                int row = 1;

                while (cur != nullptr && row < 10) {
                    mvwprintw(BlockList_Win, row++, 1,
                        "%s H:%d S:%d E:%d Sz:%d",
                        cur->is_free ? "Free" : "Used",
                        cur->handle, cur->start, cur->end, cur->size
                    );
                    cur = cur->next;
                }

                wrefresh(BlockList_Win);
                break;
            }

            default:
                break;
        }

        safe_write_xy(Console_Win, 2, 1, "Ultima # ");
        wmove(Console_Win, 2, 10);
    }

    endwin();
    return 0;
}
