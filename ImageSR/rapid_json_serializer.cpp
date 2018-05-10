#include "stdafx.h"
#include "tree_serializer.h"
#include "utils_rapidjson.h"
#include "training_data.h"
#include <rapidjson/filereadstream.h>
#include <rapidjson/filewritestream.h>

typedef rapidjson::MemoryPoolAllocator<> AllocatorType;

using namespace rapidjson;
using namespace imgsr;

const char key_tree[] = "tree";
const char key_settings[] = "settings";

void WriteDocToFile(const Document & doc, const string & file_path)
{
    FILE* fp = nullptr;
    fopen_s(&fp, file_path.c_str(), "wb");
    if(fp)
    {
        vector<char>write_buffer(65536);
        FileWriteStream os(fp, write_buffer.data(), write_buffer.size());
        PrettyWriter<FileWriteStream> writer(os);
        doc.Accept(writer);
    }
    fclose(fp);
}

void ReadDocFromFile(const string & file_path, Document* doc)
{
    if (doc == nullptr) return;
    FILE* fp = nullptr;
    fopen_s(&fp, file_path.c_str(), "rb");
    if(fp)
    {
        vector<char> read_buffer(65536);
        FileReadStream is(fp, read_buffer.data(), read_buffer.size());

        doc->ParseStream(is);
    }
    fclose(fp);
}

void RapidJsonSerializer::WriteTree(const DTree & tree, const string & file_path)
{
    Document doc;
    json::SerializeDTree(tree, doc.GetAllocator(), &doc);
    WriteDocToFile(doc, file_path);
}

void RapidJsonSerializer::ReadTree(const string & file_path, DTree * tree)
{
    Document doc;
    ReadDocFromFile(file_path, &doc);
    json::DeserializeDTree(doc, tree);
}

void RapidJsonSerializer::WriteTrees(const HDTrees & hdtrees, const string & file_path)
{
    Document doc;
    json::SerializeHDTrees(hdtrees, doc.GetAllocator(), &doc);
    WriteDocToFile(doc, file_path);
}

void RapidJsonSerializer::ReadTrees(const string & file_path, HDTrees * hdtrees)
{
    Document doc;
    ReadDocFromFile(file_path, &doc);
    json::DeserializeHDTrees(doc, hdtrees);
}

void WriteMatrix(rapidjson::Value* val, const EMat& mat, AllocatorType& alloc)
{
    assert(val != nullptr);
    if (val == nullptr) return;

    val->SetArray();
    // serialize each row of training data
    for (int row = 0; row < mat.rows(); row += 1)
    {
        // serialize one row
        Value row_val;
        row_val.SetArray();
        for (int col = 0; col < mat.cols(); col += 1)
        {
            Value val_x;
            val_x.SetDouble(mat(row, col));
            row_val.PushBack(val_x, alloc);
        }
        val->PushBack(row_val, alloc);
    }
}

void RapidJsonSerializer::WriteTrainingData(const TrainingData & samples, const std::string& file)
{
    Document doc;
    doc.SetObject();

    Value x_data;
    WriteMatrix(&x_data, samples.MatX(), doc.GetAllocator());
    doc.AddMember("x", x_data, doc.GetAllocator());

    Value y_data;
    WriteMatrix(&y_data, samples.MatY(), doc.GetAllocator());
    doc.AddMember("y", y_data, doc.GetAllocator());

    WriteDocToFile(doc, file);
}

void RapidJsonSerializer::WriteMatrixData(const EMat & data, const std::string & file)
{
    Document doc;
    WriteMatrix(&doc, data, doc.GetAllocator());
    WriteDocToFile(doc, file);
}
