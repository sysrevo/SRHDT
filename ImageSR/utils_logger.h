#pragma once
#include "stdafx.h"

#define USE_LOG

namespace imgsr
{

    class MyLogger
    {
    public:
        static MyLogger debug;
        bool enable = true;

        template<typename T>
        inline MyLogger & operator<<(const T& item)
        {
#ifdef USE_LOG
            if (!enable) return *this;
            stream << item;
#endif
            return *this;
        }

        inline MyLogger & operator<<(std::ostream & (*pf)(std::ostream&))
        {
#ifdef USE_LOG
            if (!enable) return *this;
            stream << pf;
#endif
            return *this;
        }

        inline operator std::ostream & ()
        {
            return stream;
        }

        inline std::ostream & GetStream() { return stream; }
    private:
        std::ostream & stream;
        MyLogger(std::ostream & s) : stream(s) {}
    };
}
#undef USE_LOG