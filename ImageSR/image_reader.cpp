#include "stdafx.h"
#include "image_reader.h"
#include "utils.h"

using namespace imgsr;

ImageReader::ImageReader(const vector<HandleFunc> & handlers_)
{
    handlers = utils::math::Select(handlers_, [](HandleFunc func) 
    {
        return func != nullptr;
    });
}

Mat ImageReader::Get(int ind) const
{
    Mat res = Read(ind);
    for (const auto & func : handlers)
        if (func) func(&res);
    return res;
}

void MemoryImageReader::Set(const vector<string> & paths)
{
    buf.reserve(paths.size());
    buf.clear();
    for (const string & p : paths)
    {
        buf.push_back(cv::imread(p));
    }
}

void MemoryImageReader::Set(const vector<Mat> & imgs)
{
    buf = imgs;
}

bool MemoryImageReader::Empty() const
{
    return buf.empty();
}

size_t MemoryImageReader::Size() const
{
    return buf.size();
}

Mat MemoryImageReader::Read(int ind) const
{
    assert(!Empty());
    assert(ind >= 0 && ind < buf.size());
    return buf[ind];
}

void FileImageReader::Set(const vector<string> & paths)
{
    buf = paths;
}

void FileImageReader::Set(const string & path)
{
    Set({ path });
}

bool FileImageReader::Empty() const
{
    return buf.empty();
}

size_t FileImageReader::Size() const
{
    return buf.size();
}

Mat FileImageReader::Read(int ind) const
{
    assert(!Empty());
    assert(ind >= 0 && ind < buf.size());
    Mat res = cv::imread(buf[ind]);
    return res;
}

bool HandlerImageReader::Empty() const
{
    for (const auto & r : readers)
        if (!r->Empty()) return false;
    return true;
}

size_t HandlerImageReader::Size() const
{
    size_t num = 0;
    for (const auto & r : readers)
        num += r->Size();
    return num;
}

Mat HandlerImageReader::Read(int ind) const
{
    Mat img;
    for (const auto & r : readers)
    {
        int size = (int)r->Size();
        if (ind < size)
        {
            img = r->Get(ind);
            break;
        }
        ind -= size;
    }
    return img;
}

void HandlerImageReader::SetInput(
    const vector<Ptr<ImageReader>> & readers_)
{
    readers = utils::math::Select(readers_, [](const Ptr<ImageReader> & reader)
    {
        return reader != nullptr;
    });
}