/**
 * sendfile
 * used in tcp
 * ssize_t tee(int fdin, int fdout, size_t len, unsigned int flags);
 * tee函数在两个管道文件描述符之间复制数据
 * delaying
 */

#include <fcntl.h> 