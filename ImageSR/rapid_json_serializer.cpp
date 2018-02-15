#include "stdafx.h"
#include "tree_serializer.h"
#include "utils_rapidjson.h"

using namespace rapidjson;
using namespace imgsr;

const char key_tree[] = "tree";
const char key_settings[] = "settings";

void RapidJsonSerializer::Serialize(const DTree & tree, std::ostream & os)
{
    Document doc;
    json::SerializeDTree(tree, doc.GetAllocator(), &doc);

    OStreamWrapper osw(os);
    PrettyWriter<OStreamWrapper> writer(osw);
    doc.Accept(writer);
}

void RapidJsonSerializer::Deserialize(std::istream & is, DTree * tree)
{
    IStreamWrapper reader(is);
    Document doc;
    doc.ParseStream(reader);

    json::DeserializeDTree(doc, tree);
}

void RapidJsonSerializer::Serialize(const HDTrees & hdtrees, std::ostream & os)
{
    Document doc;

    json::SerializeHDTrees(hdtrees, doc.GetAllocator(), &doc);

    OStreamWrapper osw(os);
    PrettyWriter<OStreamWrapper> writer(osw);
    doc.Accept(writer);
}

void RapidJsonSerializer::Deserialize(std::istream & is, HDTrees * hdtrees)
{
    IStreamWrapper isw(is);
    Document doc;
    doc.ParseStream(isw);

    json::DeserializeHDTrees(doc, hdtrees);
}
