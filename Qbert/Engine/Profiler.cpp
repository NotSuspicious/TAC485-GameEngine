//
// Created by William Zhao on 2/4/26.
//

#include "Profiler.h"

Profiler::ScopedTimer::ScopedTimer(Profiler::Timer *timer) : m_timer(timer)
{
    m_timer->Start();
}

Profiler::ScopedTimer::~ScopedTimer()
{
    m_timer->Stop();
}

Profiler::Timer *Profiler::GetTimer(const std::string &name)
{
    if (m_timers.contains(name))
    {
        return m_timers[name];
    } else
    {
        m_timers[name] = new Timer(name);
        return m_timers[name];
    }
}

void Profiler::ResetAll()
{
    for (auto timer: m_timers)
    {
        timer.second->Reset();
    }
}

Profiler *Profiler::Get()
{
    static Profiler s_profiler;
    return &s_profiler;
}

Profiler::Profiler() : m_needsComma(false), m_nextThreadId(0)
{
    m_jsonFile.open("profile.json");
    m_jsonFile << "[";
}

uint32_t Profiler::GetThreadId()
{
    std::thread::id currentThread = std::this_thread::get_id();

    if (m_threadIdMap.contains(currentThread))
    {
        return m_threadIdMap[currentThread];
    }

    uint32_t newId = m_nextThreadId++;
    m_threadIdMap[currentThread] = newId;
    return newId;
}

void Profiler::BeginTimer(const std::string &name, uint64_t startTime)
{
    std::lock_guard<std::mutex> lock(m_jsonLock);

    uint32_t tid = GetThreadId();

    if (m_needsComma)
    {
        m_jsonFile << ",";
    }

    m_jsonFile << "\n{\"name\":\"" << name
            << "\",\"ph\":\"B\",\"ts\":" << startTime
            << ",\"pid\":1,\"tid\":" << tid << "}";

    m_needsComma = true;
}

void Profiler::EndTimer(uint64_t endTime)
{
    std::lock_guard<std::mutex> lock(m_jsonLock);

    uint32_t tid = GetThreadId();

    m_jsonFile << ",\n{\"ph\":\"E\",\"ts\":" << endTime
            << ",\"pid\":1,\"tid\":" << tid << "}";
}

Profiler::~Profiler()
{
    // Close the JSON file
    m_jsonFile << "\n]";
    m_jsonFile.close();

    // Write the CSV file
    std::ofstream output("profile.txt");
    output << "Timer Name,Avg (ms),Max (ms)" << std::endl;

    for (auto timer: m_timers)
    {
        output << timer.second->m_name << ","
                << std::fixed << std::setprecision(3) << timer.second->GetAvg_ms() << ","
                << std::fixed << std::setprecision(3) << timer.second->GetMax_ms()
                << std::endl;
    }
    output.close();
}

