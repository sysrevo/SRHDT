#pragma once

#include "stdafx.h"
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/istreamwrapper.h>

#include "tree.h"

namespace imgsr
{
namespace json
{
typedef rapidjson::Value Value;
typedef rapidjson::Document Document;
typedef rapidjson::MemoryPoolAllocator<> AllocatorType;

// -------- Model -------------
void SerializeModel(const EMat & model, AllocatorType & alloc, Value* model_arr);
EMat DeserializeModel(const Value & val, int patch_size);

// -------- BinaryTest -------------
void SerializeBinaryTest(const BinaryTest & test, AllocatorType & alloc, Value* test_obj);
BinaryTest DeserializeBinaryTest(const Value & val);

// -------- Node -------------
void SerializeNode(DTNode * node, AllocatorType & alloc, Value* node_obj);
void DeserializeNode(const Value & val, int patch_size, DTNode* node);

// -------- Settings -------------
void SerializeSettings(const Settings & sets, AllocatorType & alloc, Value* sets_obj);
Settings DeserializeSettings(const Value & val);

// -------- DTree -------------
void SerializeDTree(const DTree & tree, AllocatorType & alloc, Value* tree_obj);
void DeserializeDTree(const Value & tree_obj, DTree * tree_out_ptr);

// -------- HDTrees -------------
void SerializeHDTrees(const HDTrees & tree, AllocatorType & alloc, Value* hdt_obj);
void DeserializeHDTrees(const Value & hdt_obj, HDTrees* tree_out);
}
}