
#include <unistd.h>
#include "Threads.h"


Threads::Threads()
{
}


Threads::~Threads()
{
}


int Threads::GetHardwareThreadCount()
{
    return Max(static_cast<int>(sysconf(_SC_NPROCESSORS_ONLN)), 1);
}


void Threads::Run(const int count, Function *loopBody)
{
    ASSERT(loopBody != nullptr);

    if (count < 1) {
        return;
    }

    if (count == 1) {
        loopBody->Execute(0, mMainMemory);
        return;
    }

    mTaskData->Cursor = 0;
    mTaskData->Count = count;
    mTaskData->Fn = loopBody;

    const int threadCount = Min(mThreadCount, count);

    mTaskData->RequiredWorkerCount = threadCount;
    mTaskData->FinalizedWorkers = 0;

    pthread_mutex_lock(&mTaskData->FinalizationMutex);

    // Wake all threads waiting on this condition variable.
    pthread_cond_broadcast(&mTaskData->CV);

    while (mTaskData->FinalizedWorkers < threadCount) {
        pthread_cond_wait(&mTaskData->FinalizationCV,
            &mTaskData->FinalizationMutex);
    }

    pthread_mutex_unlock(&mTaskData->FinalizationMutex);

    // Cleanup.
    mTaskData->Cursor = 0;
    mTaskData->Count = 0;

    mTaskData->Fn = nullptr;

    mTaskData->RequiredWorkerCount = 0;
    mTaskData->FinalizedWorkers = 0;
}


void Threads::ResetFrameMemory()
{
    for (int i = 0; i < mThreadCount; i++) {
        mThreadData[i]->Memory.ResetFrameMemory();
    }

    mMainMemory.ResetFrameMemory();
}


void Threads::RunThreads()
{
    if (mThreadData != nullptr) {
        return;
    }

    mTaskData = new TaskList();

    const int cpuCount = Min(GetHardwareThreadCount(), 128);

    mThreadCount = cpuCount;

    mThreadData = new ThreadData * [cpuCount];

    for (int i = 0; i < cpuCount; i++) {
        mThreadData[i] = new ThreadData(mTaskData);
    }

    for (int i = 0; i < cpuCount; i++) {
        ThreadData *d = mThreadData[i];

        pthread_create(&d->Thread, nullptr, Worker, d);
    }
}


void *Threads::Worker(void *p)
{
    ASSERT(p != nullptr);

#ifndef __EMSCRIPTEN__
    pthread_set_qos_class_self_np(QOS_CLASS_USER_INTERACTIVE, 0);
#endif // __EMSCRIPTEN__

    ThreadData *d = reinterpret_cast<ThreadData *>(p);

    // Loop forever waiting for next dispatch of tasks.
    for (;;) {
        TaskList *items = d->Tasks;

        pthread_mutex_lock(&items->Mutex);

        while (items->RequiredWorkerCount < 1) {
            // Wait until required worker count becomes greater than zero.
            pthread_cond_wait(&items->CV, &items->Mutex);
        }

        items->RequiredWorkerCount--;

        pthread_mutex_unlock(&items->Mutex);

        const int count = items->Count;

        for (;;) {
            const int index = items->Cursor++;

            if (index >= count) {
                break;
            }

            items->Fn->Execute(index, d->Memory);
        }

        pthread_mutex_lock(&items->FinalizationMutex);

        items->FinalizedWorkers++;

        pthread_mutex_unlock(&items->FinalizationMutex);

        pthread_cond_signal(&items->FinalizationCV);
    }
}
