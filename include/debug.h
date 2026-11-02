#ifndef DEBUG
#define DEBUG

#ifdef MYDEBUG

#define ASSERT(right_instr) do { \
    if (!right_instr) { \
        fprintf(stderr, "\nAssertion failed: (%s), file <%s>, line: %d\n\n", #right_instr, __FILE__, __LINE__); \
        printf("Current function: %s\n", __PRETTY_FUNCTION__);\
        printf(RED "Link to the line:\n" RESET);          \
        printf("File: " __FILE__ ":%d:1:\n\n", __LINE__); \
        printf(RED "--------------------------------------------------" RESET); \
        abort(); \
    } \
} while (0)

#define PRINT_DEBUG(format, ...) printf(format, ##__VA_ARGS__)
#else
#define ASSERT(right_instr) (void)0
#define PRINT_DEBUG(format, ...) (void)0
#endif

#endif