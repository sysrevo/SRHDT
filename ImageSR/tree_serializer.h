#pragma once
#include "tree.h"

namespace imgsr
{
    class ITreeSerializer
    {
    public:
        virtual void WriteTree(const DTree & tree, const string & path) = 0;
        virtual void ReadTree(const string & file_path, DTree* tree) = 0;

        virtual void WriteTrees(const HDTrees & tree, const string & path) = 0;
        virtual void ReadTrees(const string & file_path, HDTrees* tree) = 0;
    };

    class RapidJsonSerializer : public ITreeSerializer
    {
    public:
        virtual void WriteTree(const DTree & tree, const string & path) override;
        virtual void ReadTree(const string & file_path, DTree * tree) override;

        virtual void WriteTrees(const HDTrees & tree, const string & path) override;
        virtual void ReadTrees(const string & file_path, HDTrees * tree) override;
    };
}