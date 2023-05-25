
#pragma once


#include <pthread.h>
#include <stdatomic.h>
#include "ThreadMemory.h"
#include "Utils.h"


/**
 * Manages a pool of threads used for parallelization of rasterization tasks.
 */
class Threads final {
public:
    Threads();
   ~Threads();
public:
    static int GetHardwareThreadCount();
public:
    template <typename F>
    void ParallelFor(const int count, const F loopBody);

    void *MallocMain(const int size);

    template <typename T>
    T *MallocMain();

    template <typename T, typename ...Args>
    T *NewMain(Args&&... args);

    void ResetFrameMemory();
private:
    void RunThreads();
private:

    struct Function {
        virtual ~Function() {
        }

        virtual void Execute(const int index, ThreadMemory &memory) = 0;
    };

    template <typename T>
    struct Fun : public Function {
        constexpr Fun(const T lambda)
        :   Lambda(lambda)
        {
        }

        void Execute(const int index, ThreadMemory &memory) {
            Lambda(index, memory);
        }

        T Lambda;
    };

    struct TaskList final {
        atomic_int Cursor = 0;
        int Count = 0;
        Function *Fn = nullptr;

        pthread_cond_t CV = PTHREAD_COND_INITIALIZER;
        pthread_mutex_t Mutex = PTHREAD_MUTEX_INITIALIZER;
        int RequiredWorkerCount = 0;

        pthread_cond_t FinalizationCV = PTHREAD_COND_INITIALIZER;
        pthread_mutex_t FinalizationMutex = PTHREAD_MUTEX_INITIALIZER;
        int FinalizedWorkers = 0;
    };

    struct ThreadData final {
        ThreadData(TaskList *tasks)
        :   Tasks(tasks)
        {
        }

        ThreadMemory Memory;
        TaskList *Tasks = nullptr;
        pthread_t Thread = 0;
    };

    TaskList *mTaskData = nullptr;
    ThreadData **mThreadData = nullptr;
    int mThreadCount = 0;
    ThreadMemory mMainMemory;

private:
    void Run(const int count, Function *loopBody);
private:
    static void *Worker(void *p);
private:
    DISABLE_COPY_AND_ASSIGN(Threads);
};


template <typename F>
FORCE_INLINE void Threads::ParallelFor(const int count, const F loopBody) {
    RunThreads();

    const int run = Max(Min(64, count / (mThreadCount * 32)), 1);

    if (run == 1) {
        Fun p([&loopBody](const int index, ThreadMemory &memory) {
            loopBody(index, memory);

            memory.ResetTaskMemory();
        });

        Run(count, &p);
    } else {
        const int iterationCount = (count / run) + Min(count % run, 1);

        Fun p([run, count, &loopBody](const int index, ThreadMemory &memory) {
            const int idx = run * index;
            const int maxidx = Min(count, idx + run);

            for (int i = idx; i < maxidx; i++) {
                loopBody(i, memory);

                memory.ResetTaskMemory();
            }
        });

        Run(iterationCount, &p);
    }
}


FORCE_INLINE void *Threads::MallocMain(const int size) {
    return mMainMemory.FrameMalloc(size);
}


template <typename T>
FORCE_INLINE T *Threads::MallocMain() {
    return mMainMemory.FrameMalloc<T>();
}


template <typename T, typename ...Args>
FORCE_INLINE T *Threads::NewMain(Args&&... args) {
    return new (MallocMain<T>()) T(std::forward<Args>(args)...);
}
