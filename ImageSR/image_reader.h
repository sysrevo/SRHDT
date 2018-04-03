#pragma once
#include "common.h"

#define MAKE_CREATE_FUNC(class_type) \
	static Ptr<class_type> Create(const vector<Handler> & funcs = vector<Handler>());\
	static Ptr<class_type> Create(Handler func);

#define MAKE_CONSTRUCTOR(class_type)\
	class_type(const Handler & func);\
	class_type(const vector<Handler> & funcs = vector<Handler>());

namespace imgsr
{
    class ImageReader
    {
    public:
        typedef std::function<void(Mat*)> Handler;

        ImageReader(const vector<Handler> & handlers = vector<Handler>());
	    ImageReader(const Handler & func);

        virtual bool Empty() const = 0;
        virtual size_t Size() const = 0;

        Mat Get(int ind) const;
	    inline Mat operator[](int ind) const { return Get(ind); }

        vector<Handler> handlers;
    protected:
        virtual Mat Read(int ind) const = 0;
    };


    class MemIR : public ImageReader
    {
    public:
	    MAKE_CREATE_FUNC(MemIR)
	    MAKE_CONSTRUCTOR(MemIR)

        virtual bool Empty() const override;
        virtual size_t Size()  const override;

	    vector<Mat> images;
    protected:
        virtual Mat Read(int ind) const override;
    };

    class FileIR : public ImageReader
    {
    public:
	    MAKE_CREATE_FUNC(FileIR)
	    MAKE_CONSTRUCTOR(FileIR)

        virtual bool Empty() const override;
        virtual size_t Size()  const override;

        vector<string> paths;
    protected:
        virtual Mat Read(int ind) const override;
    };

    class WrappedIR : public ImageReader
    {
    public:
	    MAKE_CREATE_FUNC(WrappedIR)
	    MAKE_CONSTRUCTOR(WrappedIR)
	
	    virtual bool Empty() const override;
	    virtual size_t Size() const override;

        Ptr<ImageReader> source = nullptr;
    protected:
	    virtual Mat Read(int ind) const override;
    };
}

#undef MAKE_CREATE_FUNC
#undef MAKE_CONSTRUCTOR