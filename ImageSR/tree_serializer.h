#pragma once
#include "tree.h"

namespace imgsr
{
    class ITreeSerializer
    {
    public:
        virtual void Serialize(const DTree & tree, const string & path) = 0;
        virtual void Serialize(const HDTrees & tree, const string & path) = 0;
        virtual void Deserialize(const string & file_path, DTree* tree) = 0;
        virtual void Deserialize(const string & file_path, HDTrees* tree) = 0;
    };

    class RapidJsonSerializer : public ITreeSerializer
    {
    public:
        virtual void Serialize(const DTree & tree, const string & path) override;
        virtual void Serialize(const HDTrees & tree, const string & path) override;
        virtual void Deserialize(const string & file_path, DTree * tree) override;
        virtual void Deserialize(const string & file_path, HDTrees * tree) override;
        void DeserializeString(const string & buf, DTree * tree);
        void DeserializeString(const string & buf, HDTrees * tree);
    };
}