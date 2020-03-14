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
    return RES;                                                                          \
}

#define CHECK(CODE) CHECK_ERROR(CODE, -1, __FILE__, __LINE__)
#define CHECK_THREAD(CODE) CHECK_ERROR(CODE, nullptr, __FILE__, __LINE__)

struct CondVar
{
    pthread_mutex_t* mutex;
    pthread_cond_t* conditional;
};

void* lockConditional(void* ptr)
{
    CondVar* cv = reinterpret_cast<CondVar*>(ptr);
    std::cout << "cond await..." << std::endl;
    CHECK_THREAD(pthread_cond_wait(cv->conditional, cv->mutex))
    std::cout << "after cond await..." << std::endl;
    sleep(5);
    return nullptr;
}

void* lockBarrier(void* ptr)
{
    pthread_barrier_t* barrier = reinterpret_cast<pthread_barrier_t*>(ptr);
    std::cout << "barrier await..." << std::endl;
    auto res = pthread_barrier_wait(barrier);
    if (res != PTHREAD_BARRIER_SERIAL_THREAD && res != 0)
    {
        std::cerr << '[' << __FILE__ << ':' << __LINE__ << "] " << strerror(errno) << std::endl;
        return nullptr;
    }
    std::cout << "after barrier await..." << std::endl;
    sleep(5);
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

    pthread_cond_t conditional;
    CHECK(pthread_cond_init(&conditional, nullptr))

    pthread_barrier_t barrier;
    CHECK(pthread_barrier_init(&barrier, nullptr, 2))

    std::array<pthread_t, 2> threads;

    CondVar cv{&mutex, &conditional};
    CHECK(pthread_create(&threads[0], nullptr, lockConditional, &cv))
    CHECK(pthread_create(&threads[1], nullptr, lockBarrier, &barrier))

    std::cout << "main thread working" << std::endl;

    sleep(30);

    auto res = pthread_barrier_wait(&barrier);
    if (res != PTHREAD_BARRIER_SERIAL_THREAD && res != 0)
    {
        std::cerr << '[' << __FILE__ << ':' << __LINE__ << "] " << strerror(errno) << std::endl;
        return -1;
    }

    CHECK(pthread_cond_signal(&conditional));

    for(auto thread : threads)
    {
        CHECK(pthread_join(thread, nullptr))
    }

    CHECK(pthread_barrier_destroy(&barrier))

    CHECK(pthread_cond_destroy(&conditional))

    CHECK(pthread_mutex_destroy(&mutex))

    return 0;
}