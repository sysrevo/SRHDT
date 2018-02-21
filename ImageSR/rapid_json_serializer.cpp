#include "stdafx.h"
#include "tree_serializer.h"
#include "utils_rapidjson.h"
#include <rapidjson/filereadstream.h>
#include <rapidjson/filewritestream.h>

using namespace rapidjson;
using namespace imgsr;

const char key_tree[] = "tree";
const char key_settings[] = "settings";

template<class OStreamType>
void SaveDoc(const Document & doc, OStreamType & os)
{
    PrettyWriter<OStreamType> writer(os);
    doc.Accept(writer);
}

void WriteDocToFile(const Document & doc, const string & file_path)
{
    FILE* fp = fopen(file_path.c_str(), "wb");
    {
        vector<char>write_buffer(65536);
        FileWriteStream os(fp, write_buffer.data(), write_buffer.size());

        SaveDoc(doc, os);
    }
    fclose(fp);
}

void ReadDocFromFile(const string & file_path, Document* doc)
{
    if (doc == nullptr) return;
    FILE* fp = fopen(file_path.c_str(), "rb");
    {
        vector<char> read_buffer(65536);
        FileReadStream is(fp, read_buffer.data(), read_buffer.size());

        doc->ParseStream(is);
    }
    fclose(fp);
}

void RapidJsonSerializer::Serialize(const DTree & tree, const string & file_path)
{
    Document doc;
    json::SerializeDTree(tree, doc.GetAllocator(), &doc);
    WriteDocToFile(doc, file_path);
}

void RapidJsonSerializer::Deserialize(const string & file_path, DTree * tree)
{
    Document doc;
    ReadDocFromFile(file_path, &doc);
    json::DeserializeDTree(doc, tree);
}

void RapidJsonSerializer::Serialize(const HDTrees & hdtrees, const string & file_path)
{
    Document doc;
    json::SerializeHDTrees(hdtrees, doc.GetAllocator(), &doc);
    WriteDocToFile(doc, file_path);
}

void RapidJsonSerializer::Deserialize(const string & file_path, HDTrees * hdtrees)
{
    Document doc;
    ReadDocFromFile(file_path, &doc);
    json::DeserializeHDTrees(doc, hdtrees);
}

void RapidJsonSerializer::DeserializeString(const string & buf, DTree * tree)
{
    Document doc;
    doc.Parse(buf.c_str());
    json::DeserializeDTree(doc, tree);
}

void RapidJsonSerializer::DeserializeString(const string & buf, HDTrees * trees)
{
    Document doc;
    doc.Parse(buf.c_str());
    json::DeserializeHDTrees(doc, trees);
}
