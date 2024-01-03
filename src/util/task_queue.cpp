void* task_queue_thread_run(void *p);

struct Task {
    virtual void run() = 0;

    virtual void deinit() {
        xfree(this);
    }
};

struct Task_Queue {
    pthread_t thread;
    bool volatile running = false;
    Array<Task*> queue; // TODO: make this a Ring_Buffer<Task*>
    pthread_mutex_t lock;
    pthread_cond_t signal;

    Task_Queue() {
        assert(pthread_mutex_init(&lock, NULL) == 0);
        assert(pthread_cond_init(&signal, NULL) == 0);
        assert(pthread_create(&thread, NULL, &task_queue_thread_run, this) == 0);
    }

    void deinit() {
        running = false;
        assert(pthread_mutex_lock(&lock) == 0);
        assert(pthread_cond_signal(&signal) == 0);
        assert(pthread_mutex_unlock(&lock) == 0);
        pthread_join(thread, NULL);
        pthread_mutex_destroy(&lock);
        queue.free();
    }

    void enqueue(Task *task) {
        assert(pthread_mutex_lock(&lock) == 0);
        queue.push(task);
        assert(pthread_cond_signal(&signal) == 0);
        assert(pthread_mutex_unlock(&lock) == 0);
    }

    void* run() {
        running = true;
        while(running) {
            Task *task = NULL;

            assert(pthread_mutex_lock(&lock) == 0);
            while(queue.count == 0) {
                struct timespec ts = {0,0};
                clock_gettime(CLOCK_REALTIME, &ts);
                ts.tv_sec += 1;
                pthread_cond_timedwait(&signal, &lock, &ts);
                
                if(!running) {
                    assert(pthread_mutex_unlock(&lock) == 0);
                    return NULL;
                }
            }
            if(queue.count) {
                task = queue.ordered_remove(0);
            }
            assert(pthread_mutex_unlock(&lock) == 0);

            if(task) {
                task->run();
                task->deinit();
            }
        }
        return NULL;
    }
};

void* task_queue_thread_run(void *p) {
    auto task_queue = cast(Task_Queue*, p);
    return task_queue->run();
}