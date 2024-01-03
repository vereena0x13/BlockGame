FILE *log_file = NULL;
Level log_level = TRACE;
Ticket_Mutex log_lock;

void log_init() {
    mkdir_if_not_exists("logs");
    
    copy_file("logs/latest.1.log", "logs/latest.2.log");
    copy_file("logs/latest.0.log", "logs/latest.1.log");

    log_file = fopen("logs/latest.0.log", "w");
    if(!log_file) {
        log(ERROR, "Failed to open log file 'logs/latest.0.log'");
    }
}

void log_deinit() {
    fclose(log_file);
}

rstr format_level(Level level, bool color) {
    if(color) {
        switch(level) {
            case FATAL:
                return "\u001b[31;1mFATAL\u001b[0m";
            case ERROR:
                return "\u001b[31;1mERROR\u001b[0m";
            case WARN:
                return "\u001b[33;1mWARN\u001b[0m";
            case INFO:
                return "\u001b[32;1mINFO\u001b[0m";
            case DEBUG:
                return "\u001b[34;1mDEBUG\u001b[0m";
            case TRACE:
                return "\u001b[35;1mTRACE\u001b[0m";
            default:
                assert(false);
        }
    } else {
        switch(level) {
            case FATAL:
                return "FATAL";
            case ERROR:
                return "ERROR";
            case WARN:
                return "WARN";
            case INFO:
                return "INFO";
            case DEBUG:
                return "DEBUG";
            case TRACE:
                return "TRACE";
            default:
                assert(false);
        }
    }
}

void log(Level level, rstr fmt, ...) {
    if(level > log_level) return;

    log_lock.lock();

    va_list args;
	va_start(args, fmt);
    u64 n = vsnprintf(NULL, 0, fmt, args);
    va_end(args);
    va_start(args, fmt);
    auto s = cast(cstr, xalloc(n + 1));
    vsprintf(s, fmt, args);
    va_end(args);

    // TODO: check if color is supported for format_level

    cstr s1 = NULL;
    if(log_file) {
        time_t t = time(NULL);
        struct tm tm = *localtime(&t);
        
        n = snprintf(
            NULL, 0,
            "%02d-%02d-%04d %02d:%02d:%02d [%s] %s\n",
            tm.tm_mon + 1, tm.tm_mday, tm.tm_year + 1900,
            tm.tm_hour, tm.tm_min, tm.tm_sec,
            format_level(level, false), 
            s
        );
        s1 = cast(cstr, xalloc(n + 1));
        sprintf(
            s1,
            "%02d-%02d-%04d %02d:%02d:%02d [%s] %s\n",
            tm.tm_mon + 1, tm.tm_mday, tm.tm_year + 1900,
            tm.tm_hour, tm.tm_min, tm.tm_sec,
            format_level(level, false), 
            s
        );

        fwrite(s1, 1, strlen(s1), log_file);
        fflush(log_file);
    }

    auto s2 = tsprintf("[%s] %s\n", format_level(level, true), s);
    printf("%s", s2);

    xfree(s);
    if(s1) xfree(s1);

    log_lock.unlock();
}