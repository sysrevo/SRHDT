#pragma once

#include "tree.h"

namespace imgsr
{
    class ITreeSerializer
    {
    public:
        virtual void Serialize(const DTree & tree, std::ostream & os) = 0;
        virtual void Serialize(const HDTrees & tree, std::ostream & os) = 0;
        virtual void Deserialize(std::istream & is, DTree* tree) = 0;
        virtual void Deserialize(std::istream & is, HDTrees* tree) = 0;
    };

    class RapidJsonSerializer : public ITreeSerializer
    {
    public:
        virtual void Serialize(const DTree & tree, std::ostream & os) override;
        virtual void Serialize(const HDTrees & tree, std::ostream & os) override;
        virtual void Deserialize(std::istream & is, DTree * tree) override;
        virtual void Deserialize(std::istream & is, HDTrees * tree) override;
        void Deserialize(const string & buf, DTree * tree);
        void Deserialize(const string & buf, HDTrees * tree);
    };
}