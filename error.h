/**
 *  define some exceptions 
 */
#ifndef __ERROR_H__
#define __ERROR_H__
typedef unsigned char byte;

#define ERR_EXIT(m)         \
    do                      \
    {                       \
        perror(m);          \
        exit(EXIT_FAILURE); \
    } while (0)

#endif