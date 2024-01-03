enum Level {
    FATAL = 0,
    ERROR = 1,
    WARN  = 2,
    INFO  = 3,
    DEBUG = 4,
    TRACE = 5
};

extern Level log_level;

rstr format_level(Level level, bool color);

void log_init();
void log(Level level, rstr fmt, ...);