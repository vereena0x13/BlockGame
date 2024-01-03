struct Ticket_Mutex {
    void lock() {
        u64 ticket = atomic_add(&this->ticket, 1);
        while(ticket != serving) _mm_pause();
    }

    void unlock() {
        atomic_add(&serving, 1);
    }

private:
    u64 volatile ticket = 0;
    u64 volatile serving = 0;
};