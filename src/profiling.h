#pragma once
#include <string>
#include <iostream>
#include <sstream>
#include <chrono>

namespace Profiling
{
    using clk = std::chrono::high_resolution_clock;
    using time_point = std::chrono::time_point<clk>;
    using std::chrono::duration;
    using std::chrono::duration_cast;

    class Timer
    {
    public:
        explicit Timer(const std::string &name) : m_start{clk::now()}, m_name{name} {}

        Timer(const Timer &other) = default;
        Timer &operator=(const Timer &other) = default;

        Timer(Timer &&other) = default;
        Timer &operator=(Timer &&other) = default;

        ~Timer() 
        { 
            print(std::cerr);
        }

        void print(std::ostream &os)
        {
            m_finish = clk::now();
            auto elapsed = m_finish - m_start;
            auto elapsed_s = duration_cast<duration<double>>(elapsed).count();
            auto elapsed_ms = elapsed_s * 1000;

            std::stringstream ss;
            ss << m_name << " " << elapsed_ms << " ms\n";
            os << ss.str();
        }

    private:
        time_point m_start;
        time_point m_finish;
        std::string m_name;
    };

};
