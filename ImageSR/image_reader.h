#pragma once

#include "stdafx.h"

namespace imgsr
{
    class ImageReader
    {
    public:
        typedef std::function<void(Mat*)> HandleFunc;

        inline ImageReader(const vector<HandleFunc> handlers = vector<HandleFunc>())
            :handles(handlers) {}

        inline ImageReader(const HandleFunc & func)
        {
            handles.push_back(func);
        }

        virtual bool Empty() const = 0;

        virtual size_t Size() const = 0;

        inline Mat Get(int ind) const
        {
            Mat res = Read(ind);
            for (const auto & func : handles)
                func(&res);
            return res;
        }
    protected:
        virtual Mat Read(int ind) const = 0;

        vector<HandleFunc> handles;
    };

    class MemoryImageReader : public ImageReader
    {
    public:
        inline MemoryImageReader(const HandleFunc & func)
            :ImageReader(func) {}

        inline MemoryImageReader(const vector<HandleFunc> & funcs = vector<HandleFunc>())
            :ImageReader(funcs) {}

        void Set(const vector<string> & paths);
        void Set(const vector<Mat> & imgs);

        virtual bool   Empty() const override;

        virtual size_t Size()  const override;
    protected:
        virtual Mat    Read(int ind) const override;

        vector<Mat> buf;
    };

    class FileImageReader : public ImageReader
    {
    public:
        inline FileImageReader(const vector<HandleFunc> & funcs = vector<HandleFunc>())
            :ImageReader(funcs) {}

        inline FileImageReader(const HandleFunc & func)
            :ImageReader(func) {}

        void Set(const vector<string> & paths);

        virtual bool   Empty() const override;
        virtual size_t Size()  const override;
    protected:
        virtual Mat    Read(int ind) const override;

        vector<string> buf;
    };

    class HandlerImageReader : public ImageReader
    {
    public:
        HandlerImageReader(const vector<HandleFunc> & funcs = vector<HandleFunc>())
            :ImageReader(funcs) {}

        HandlerImageReader(const HandleFunc & func)
            :ImageReader(func) {}

        void Set(const Ptr<ImageReader> & reader);

        virtual bool   Empty() const override;

        virtual size_t Size()  const override;

        vector<Ptr<ImageReader>> readers;
    protected:
        virtual Mat    Read(int ind) const override;
    };
}