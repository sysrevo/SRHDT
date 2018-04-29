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

Mat ImageReader::Get(size_t ind) const
{
    Mat res = Read(ind);
    for (const auto & func : handlers)
        if (func) func(&res);
    return res;
}

#define MAKE_CREATE_FUNC(class_type) \
	Ptr<class_type> class_type::Create(const vector<Handler> & handlers)\
	{\
		return make_shared<class_type>(handlers);\
	}\
	\
	Ptr<class_type> class_type::Create(Handler handlers)\
	{\
		return make_shared<class_type>(handlers);\
	}

#define MAKE_CONSTRUCTOR(class_type)\
	class_type::class_type(const Handler & handler)\
		:ImageReader(handler) {}\
	\
	class_type::class_type(const vector<Handler> & handlers)\
		: ImageReader(handlers) {}

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

vector<Ptr<ImageReader>> MemIR::Split(int num) const
{
    vector<vector<Mat>> data = utils::math::SplitEqually(this->images, num);

    vector<Ptr<ImageReader>> res(data.size());
    for (int i = 0; i < data.size(); i += 1)
    {
        auto part = MemIR::Create(handlers);
        part->images = std::move(data[i]);
        res[i] = part;
    }
    return res;
}

Mat MemIR::Read(size_t ind) const
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

vector<Ptr<ImageReader>> FileIR::Split(int num) const
{
    vector<vector<string>> data = utils::math::SplitEqually(this->paths, num);

    vector<Ptr<ImageReader>> res(data.size());
    for (int i = 0; i < data.size(); i += 1)
    {
        auto part = FileIR::Create(handlers);
        part->paths = std::move(data[i]);
        res[i] = part;
    }
    return res;
}

Mat FileIR::Read(size_t ind) const
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

vector<Ptr<ImageReader>> WrappedIR::Split(int num) const
{
    vector<Ptr<ImageReader>> data = this->source->Split(num);

    vector<Ptr<ImageReader>> res(data.size());
    for (int i = 0; i < data.size(); i += 1)
    {
        auto part = WrappedIR::Create(handlers);
        part->source = data[i];
        res[i] = part;
    }
    return res;
}

Mat WrappedIR::Read(size_t ind) const
{
	if (source)
	{
		Mat img = source->Get(ind);
		return img;
	}
	return Mat();
}
