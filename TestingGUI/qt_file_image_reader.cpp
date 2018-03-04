#include "qt_file_image_reader.h"
#include <qdiriterator.h>

bool QFileImageReader::Empty() const
{
    return paths.empty();
}

size_t QFileImageReader::Size() const
{
    return paths.size();
}

cv::Mat QFileImageReader::Read(int ind) const
{
    return cv::imread(paths[ind]);
}

std::string ToLocalString(const QString& str)
{
    return std::string(str.toLocal8Bit().constData());
}


imgsr::Ptr<QFileImageReader> QFileImageReader::Create(const QString & dir, int max_size)
{
    auto res = std::make_shared<QFileImageReader>();
    QDirIterator it(dir, QStringList() << "*.png" << "*.bmp" << "*.jpg", QDir::Files, QDirIterator::Subdirectories);



    while (it.hasNext())
    {
        if (max_size > 0 && res->Size() >= max_size) break;
        res->paths.push_back(ToLocalString(it.next()));
    }

    return res;
}
