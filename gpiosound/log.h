#ifndef LOG_H__
#define LOG_H__

#ifdef NDEBUG
#  define debug(fmt, ...)
#else
#  define debug(fmt, ...) printf("DEBUG: " fmt, ##__VA_ARGS__)
#endif

#endif /* LOG_H__ */
