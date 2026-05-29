//
// Created by William Zhao on 4/15/26.
//

#include "JobManager.h"

void JobManager::Worker::Begin()
{
    m_thread = std::thread(&Worker::Loop);
}

void JobManager::Worker::End()
{
    if (m_thread.joinable())
    {
        m_thread.join();
    }
}

void JobManager::Worker::Loop()
{
    JobManager *pManager = JobManager::Get();
    while (true)
    {
        Job *job = pManager->GetJob();
        if (job == nullptr)
        {
            break;
        }
        job->DoIt();
        delete job; {
            std::unique_lock<std::mutex> lock(pManager->m_jobMutex);
            pManager->m_activeJobs--;
        }
        pManager->m_jobsComplete.notify_one();
    }
}

// JobManager implementation
JobManager::JobManager() : m_activeJobs(0), m_shutdown(false)
{
}

JobManager::~JobManager() = default;

JobManager *JobManager::Get()
{
    static JobManager s_jobManager;
    return &s_jobManager;
}

void JobManager::Begin()
{
    m_shutdown = false;
    for (auto &worker: m_workers)
    {
        worker.Begin();
    }
}

void JobManager::End()
{
    WaitForJobs();
    m_shutdown = true;

    // Wake up all workers so they can check the shutdown flag
    m_jobAvailable.notify_all();

    // Wait for all workers to finish
    for (auto &worker: m_workers)
    {
        worker.End();
    }

    // Clean up any remaining jobs in the queue (safety check)
    {
        std::unique_lock<std::mutex> lock(m_jobMutex);
        while (!m_jobs.empty())
        {
            Job *job = m_jobs.front();
            m_jobs.pop();
            delete job;
        }
    }
}

void JobManager::AddJob(Job *pJob)
{ {
        std::unique_lock<std::mutex> lock(m_jobMutex);
        m_jobs.push(pJob);
        m_activeJobs++;
    }
    m_jobAvailable.notify_one();
}

void JobManager::WaitForJobs()
{
    std::unique_lock<std::mutex> lock(m_jobMutex);
    m_jobsComplete.wait(lock, [this] { return m_activeJobs == 0 && m_jobs.empty(); });
}

JobManager::Job *JobManager::GetJob()
{
    std::unique_lock<std::mutex> lock(m_jobMutex);

    m_jobAvailable.wait(lock, [this] { return !m_jobs.empty() || m_shutdown; });

    if (m_jobs.empty())
    {
        return nullptr;
    }

    Job *job = m_jobs.front();
    m_jobs.pop();
    return job;
}

