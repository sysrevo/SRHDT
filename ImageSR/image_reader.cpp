#include "stdafx.h"
#include "image_reader.h"
#include "../Utils/utils.h"

using namespace imgsr;
using namespace utils;

ImageReader::ImageReader(const vector<Handler> & handlers_)
{
    handlers = math::Select(handlers_, [](Handler func) 
    {
        return func != nullptr;
    });
}

ImageReader::ImageReader(const Handler & func)
	: ImageReader(vector<Handler>({ func }))
{
}

Mat ImageReader::Get(int ind) const
{
    Mat res = Read(ind);
    for (const auto & func : handlers)
        if (func) func(&res);
    return res;
}

#define MAKE_CREATE_FUNC(class_type) \
	Ptr<class_type> class_type::Create(const vector<Handler> & funcs)\
	{\
		return make_shared<class_type>(funcs);\
	}\
	\
	Ptr<class_type> class_type::Create(Handler func)\
	{\
		return make_shared<class_type>(func);\
	}

#define MAKE_CONSTRUCTOR(class_type)\
	class_type::class_type(const Handler & func)\
		:ImageReader(func) {}\
	\
	class_type::class_type(const vector<Handler> & funcs)\
		: ImageReader(funcs) {}

MAKE_CREATE_FUNC(MemIR)
MAKE_CONSTRUCTOR(MemIR)

bool MemIR::Empty() const
{
    return images.empty();
}

size_t MemIR::Size() const
{
    return images.size();
}

Mat MemIR::Read(int ind) const
{
    assert(!Empty());
    assert(ind >= 0 && ind < images.size());
	return images[ind];
}

MAKE_CREATE_FUNC(FileIR)
MAKE_CONSTRUCTOR(FileIR)

bool FileIR::Empty() const
{
    return paths.empty();
}

size_t FileIR::Size() const
{
    return paths.size();
}

Mat FileIR::Read(int ind) const
{
    assert(!Empty());
    assert(ind >= 0 && ind < paths.size());
    Mat res = cv::imread(paths[ind]);
    return res;
}

MAKE_CREATE_FUNC(WrappedIR)
MAKE_CONSTRUCTOR(WrappedIR)

bool WrappedIR::Empty() const
{
	if (source) return source->Empty();
	return false;
}

size_t WrappedIR::Size() const
{
	if (source) return source->Size();
	return -1;
}

Mat WrappedIR::Read(int ind) const
{
	if (source)
	{
		Mat img = source->Get(ind);
		return img;
	}
	return Mat();
}
