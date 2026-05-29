#pragma once
#include <chrono>
#include <unordered_map>
#include <map>
#include <string>
#include <fstream>
#include <iomanip>
#include <thread>
#include <mutex>

#define PROFILE_SCOPE(name) \
Profiler::ScopedTimer name##_scope(Profiler::Get()->GetTimer(std::string(#name)));

class Profiler
{
public:
    class ScopedTimer; // Forward declaration

    class Timer
    {
        friend Profiler;
        friend ScopedTimer;

    public:
        void Start()
        {
            m_timerStart = std::chrono::high_resolution_clock::now();
            Profiler::Get()->BeginTimer(m_name, m_timerStart.time_since_epoch().count() / 1000);
        }

        void Stop()
        {
            auto end = std::chrono::high_resolution_clock::now();
            m_currentFrameTime += std::chrono::duration<double, std::milli>(end - m_timerStart).count();
            Profiler::Get()->EndTimer(end.time_since_epoch().count() / 1000);
        }

        void Reset()
        {
            m_totalTime += m_currentFrameTime;
            m_totalFrames++;
            if (m_currentFrameTime > m_maxFrameTime)
            {
                m_maxFrameTime = m_currentFrameTime;
            }
            m_currentFrameTime = 0;
        }

        [[nodiscard]] const std::string &GetName() const { return m_name; }
        [[nodiscard]] double GetTime_ms() const { return m_currentFrameTime; }
        [[nodiscard]] double GetMax_ms() const { return m_maxFrameTime; }
        [[nodiscard]] double GetAvg_ms() const { return m_totalFrames > 0 ? m_totalTime / m_totalFrames : 0.0; }

    private:
        Timer(const std::string &name) : m_name(name), m_currentFrameTime(0.0), m_maxFrameTime(0.0), m_totalTime(0.0),
                                         m_totalFrames(0)
        {
        }

        ~Timer()
        {
        }

        std::string m_name;
        double m_currentFrameTime;
        double m_maxFrameTime;
        double m_totalTime;
        int m_totalFrames;
        std::chrono::high_resolution_clock::time_point m_timerStart;
    };

    class ScopedTimer
    {
        friend Profiler;

    public:
        ScopedTimer(Timer *timer);

        ~ScopedTimer();

    private:
        Timer *m_timer;
    };

public:
    static Profiler *Get();

    Timer *GetTimer(const std::string &name);

    void ResetAll();

    void BeginTimer(const std::string &name, uint64_t startTime);

    void EndTimer(uint64_t endTime);

    uint32_t GetThreadId();

private:
    Profiler();

    ~Profiler();

private:
    std::unordered_map<std::string, Timer *> m_timers;
    std::map<std::thread::id, uint32_t> m_threadIdMap;
    std::ofstream m_jsonFile;
    std::mutex m_jsonLock;
    bool m_needsComma;
    uint32_t m_nextThreadId;
};


