//
// Created by William Zhao on 4/15/26.
//

#ifndef GAME_JOBMANAGER_H
#define GAME_JOBMANAGER_H

#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>

class JobManager
{
public:
    class Job
    {
    public:
        virtual ~Job() = default;

        virtual void DoIt() = 0;
    };

    class Worker
    {
        friend JobManager;

    public:
        Worker() = default;

        ~Worker() = default;

        void Begin();

        void End();

    private:
        static void Loop();

        std::thread m_thread;
    };

public:
    static JobManager *Get();

    void Begin();

    void End();

    void AddJob(Job *pJob);

    void WaitForJobs();

    Job *GetJob();

private:
    JobManager();

    ~JobManager();

private:
    static const int NUM_WORKERS = 4;
    Worker m_workers[NUM_WORKERS];
    std::queue<Job *> m_jobs;
    std::mutex m_jobMutex;
    std::condition_variable m_jobAvailable;
    std::condition_variable m_jobsComplete;
    std::atomic<int> m_activeJobs{0};
    std::atomic<bool> m_shutdown{false};
};

#endif //GAME_JOBMANAGER_H
