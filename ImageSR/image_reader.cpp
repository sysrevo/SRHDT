#include "stdafx.h"
#include "image_reader.h"

using namespace imgsr;

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
    if (!img.empty())
    {
        for (const auto & func : handles)
            func(&img);
    }
    return img;
}

void HandlerImageReader::Set(const Ptr<ImageReader> & reader)
{
    readers.clear();
    readers.push_back(reader);
}