#pragma once
#include "../Utils/common.h"

namespace imgsr
{
    class ImageReader
    {
    public:
        typedef std::function<void(Mat*)> HandleFunc;

        ImageReader(const vector<HandleFunc> & handlers = vector<HandleFunc>());

        inline ImageReader(const HandleFunc & func)
            : ImageReader(vector<HandleFunc>({ func })) {}

        virtual bool Empty() const = 0;
        virtual size_t Size() const = 0;

        Mat Get(int ind) const;
    protected:
        virtual Mat Read(int ind) const = 0;

        vector<HandleFunc> handlers;
    };

    class MemoryImageReader : public ImageReader
    {
    public:
        inline static Ptr<MemoryImageReader> Create(const vector<HandleFunc> & funcs = vector<HandleFunc>())
        {
            return make_shared<MemoryImageReader>(funcs);
        }

        inline static Ptr<MemoryImageReader> Create(HandleFunc func)
        {
            return make_shared<MemoryImageReader>(func);
        }

        inline MemoryImageReader(const HandleFunc & func)
            :ImageReader(func) {}

        inline MemoryImageReader(const vector<HandleFunc> & funcs = vector<HandleFunc>())
            :ImageReader(funcs) {}

        void Set(const vector<string> & paths);
        void Set(const vector<Mat> & imgs);
        virtual bool Empty() const override;
        virtual size_t Size()  const override;
    protected:
        virtual Mat Read(int ind) const override;
        vector<Mat> buf;
    };

    class FileImageReader : public ImageReader
    {
    public:
        inline static Ptr<FileImageReader> Create(const vector<HandleFunc> & funcs = vector<HandleFunc>())
        {
            return make_shared<FileImageReader>(funcs);
        }

        inline static Ptr<FileImageReader> Create(HandleFunc func)
        {
            return make_shared<FileImageReader>(func);
        }

        inline FileImageReader(const vector<HandleFunc> & funcs = vector<HandleFunc>())
            :ImageReader(funcs) {}

        inline FileImageReader(const HandleFunc & func)
            :ImageReader(func) {}

        void Set(const vector<string> & paths);
        void Set(const string & paths);
        virtual bool Empty() const override;
        virtual size_t Size()  const override;
    protected:
        virtual Mat Read(int ind) const override;

        vector<string> buf;
    };

    class HandlerImageReader : public ImageReader
    {
    public:
        inline static Ptr<HandlerImageReader> Create(const vector<HandleFunc> & funcs = vector<HandleFunc>())
        {
            return make_shared<HandlerImageReader>(funcs);
        }

        inline static Ptr<HandlerImageReader> Create(HandleFunc func)
        {
            return make_shared<HandlerImageReader>(func);
        }

        HandlerImageReader(const vector<HandleFunc> & funcs = vector<HandleFunc>())
            :ImageReader(funcs) {}

        HandlerImageReader(const HandleFunc & func)
            :ImageReader(func) {}

        void SetInput(const vector<Ptr<ImageReader>> & readers);
        void SetInput(const Ptr<ImageReader> & reader) { SetInput(vector<Ptr<ImageReader>>({ reader })); }

        virtual bool Empty() const override;
        virtual size_t Size()  const override;

        vector<Ptr<ImageReader>> readers;
    protected:
        virtual Mat Read(int ind) const override;
    };
}