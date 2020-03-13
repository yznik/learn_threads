#include <iostream>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#define CHECK_ERROR(CODE, file, line)                                                    \
if (CODE != 0)                                                                          \
{                                                                                        \
    std::cerr << '[' << (file) << ':' << (line) << "] " << strerror(errno) << std::endl; \
    return -1;                                                                           \
}

#define CHECK(CODE) CHECK_ERROR(CODE, __FILE__, __LINE__)

void* threadForever(void*)
{
    while (true)
    {
        std::cout << "working..." << std::endl;
        usleep(100);
    }
    return nullptr;
}

int main()
{
    pthread_t thread;
    CHECK(pthread_create(&thread, nullptr, threadForever, nullptr))

    CHECK(pthread_join(thread, nullptr))
    return 0;
}