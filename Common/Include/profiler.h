#ifndef PROFILER_H_
#define PROFILER_H_

#define PROFILE 0

#if PROFILE
#pragma message( "   --> Profiler enabled!" )

#include <windows.h>  // defines QueryPerformanceFrequency
#include <QDebug>

// This code is originally from
// http://stackoverflow.com/questions/61278/quick-and-dirty-way-to-profile-your-code/61279#61279
// and has been modified to be Qt-friendly.

// Output is visible in the debugger when the application ends or by
// using DebugView by Sysinternals (http://technet.microsoft.com/en-us/sysinternals/bb896647.aspx)
// when running stand-alone.

namespace scope_timer {
    class time_collector {
        __int64 total;
        LARGE_INTEGER start;
        size_t times;
        const char* name;

        double cpu_frequency() { // cache the CPU frequency, which doesn't change.
            static double ret = 0; // store as double so division later on is floating point and not truncating
            if (ret == 0) {
                LARGE_INTEGER freq;
                QueryPerformanceFrequency(&freq);
                ret = static_cast<double>(freq.QuadPart);
            }
            return ret;
        }

    public:
        time_collector(const char* n) : times(0), name(n), total(0), start(LARGE_INTEGER()) { }
        ~time_collector() {
            double seconds = total / cpu_frequency();
            double average = seconds / times;
            qDebug() << "scope_timer> " << name << " called: "
                     << times << " times total time: " << seconds << " seconds  "
                     << " (avg " << average << ")";
        }

        void add_time(__int64 ticks) {
            total += ticks;
            ++times;
        }
    };

    class one_time {
        LARGE_INTEGER start;
        time_collector& collector;
    public:
        one_time(time_collector& tc) : collector(tc) {
            QueryPerformanceCounter(&start);
        }
        ~one_time() {
            LARGE_INTEGER end;
            QueryPerformanceCounter(&end);
            collector.add_time(end.QuadPart - start.QuadPart);
        }
    };
}

// Usage TIME_THIS_SCOPE(XX); where XX is a C variable name (can begin with a number)
#define TIME_THIS_SCOPE(name) \
    static scope_timer::time_collector st_time_collector_##name(#name); \
    scope_timer::one_time st_one_time_##name(st_time_collector_##name)

#else

   #define TIME_THIS_SCOPE(name)

#endif

#endif // PROFILER_H_
