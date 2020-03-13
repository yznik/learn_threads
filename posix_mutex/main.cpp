#include <iostream>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <array>
#include <sys/types.h>
#include <fstream>

#define CHECK_ERROR(CODE, RES, file, line)                                               \
if (CODE != 0)                                                                           \
{                                                                                        \
    std::cerr << '[' << (file) << ':' << (line) << "] " << strerror(errno) << std::endl; \
    return RES;                                                                        \
}

#define CHECK(CODE) CHECK_ERROR(CODE, -1, __FILE__, __LINE__)
#define CHECK_THREAD(CODE) CHECK_ERROR(CODE, nullptr, __FILE__, __LINE__)

void* lockMutex(void* ptr)
{
    pthread_mutex_t* mutex = reinterpret_cast<pthread_mutex_t*>(ptr);
    CHECK_THREAD(pthread_mutex_lock(mutex));
    std::cout << "mutex working..." << std::endl;
    sleep(5);
    CHECK_THREAD(pthread_mutex_unlock(mutex));
    return nullptr;
}

void* spinlockMutex(void* ptr)
{
    pthread_spinlock_t* spin = reinterpret_cast<pthread_spinlock_t*>(ptr);
    CHECK_THREAD(pthread_spin_lock(spin));
    std::cout << "spinlock working..." << std::endl;
    sleep(5);
    CHECK_THREAD(pthread_spin_unlock(spin));
    return nullptr;
}

void* readlockMutex(void* ptr)
{
    pthread_rwlock_t* rwlock = reinterpret_cast<pthread_rwlock_t*>(ptr);
    CHECK_THREAD(pthread_rwlock_rdlock(rwlock));
    std::cout << "read lock working..." << std::endl;
    sleep(5);
    CHECK_THREAD(pthread_rwlock_unlock(rwlock));
    return nullptr;
}

void* writelockMutex(void* ptr)
{
    pthread_rwlock_t* rwlock = reinterpret_cast<pthread_rwlock_t*>(ptr);
    CHECK_THREAD(pthread_rwlock_wrlock(rwlock));
    std::cout << "write lock working..." << std::endl;
    sleep(5);
    CHECK_THREAD(pthread_rwlock_unlock(rwlock));
    return nullptr;
}

constexpr const char* const PID_PATH = "/home/box/main.pid";

int main()
{
    {
        std::ofstream  f(PID_PATH);
        f << getpid() << std::endl;
    }
    pthread_mutex_t mutex;
    CHECK(pthread_mutex_init(&mutex, nullptr))
    CHECK(pthread_mutex_lock(&mutex))

    pthread_spinlock_t spin;
    CHECK(pthread_spin_init(&spin, PTHREAD_PROCESS_PRIVATE))
    CHECK(pthread_spin_lock(&spin))

    pthread_rwlock_t rwlockRead, rwlockWrite;
    CHECK(pthread_rwlock_init(&rwlockRead, nullptr))
    CHECK(pthread_rwlock_init(&rwlockWrite, nullptr))
    CHECK(pthread_rwlock_wrlock(&rwlockRead))
    CHECK(pthread_rwlock_wrlock(&rwlockWrite))

    std::array<pthread_t, 4> threads;
    std::array<void*, 4> args = {&mutex, (void*)&spin, &rwlockRead, &rwlockWrite};
    using func = void * (*) (void *); 
    std::array<func, 4> funcs = {lockMutex, spinlockMutex, readlockMutex, writelockMutex};
    for(size_t i = 0; i < threads.size(); ++i)
    {
        CHECK(pthread_create(&threads[i], nullptr, funcs[i], args[i]))    
    }

    std::cout << "main thread working" << std::endl;

    sleep(30);

    CHECK(pthread_mutex_unlock(&mutex))

    CHECK(pthread_spin_unlock(&spin))

    CHECK(pthread_rwlock_unlock(&rwlockRead))
    CHECK(pthread_rwlock_unlock(&rwlockWrite))

    for(auto thread : threads)
    {
        CHECK(pthread_join(thread, nullptr))
    }

    CHECK(pthread_rwlock_destroy(&rwlockRead))
    CHECK(pthread_rwlock_destroy(&rwlockWrite))

    CHECK(pthread_spin_destroy(&spin))

    CHECK(pthread_mutex_destroy(&mutex))

    return 0;
}