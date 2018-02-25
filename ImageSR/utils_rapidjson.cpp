#include "stdafx.h"
#include "utils_rapidjson.h"

using rapidjson::Value;
using rapidjson::Document;

using rapidjson::StringBuffer;
using rapidjson::PrettyWriter;

using namespace imgsr;

namespace jskey
{
    /*const char is_leaf[] = "is_leaf";
    const char model[] = "c";
    const char p1[] = "p1";
    const char p2[] = "p2";
    const char r[] = "r";
    const char left[] = "left";
    const char right[] = "right";

    const char binary_test[] = "binary_test";

    const char settings[] = "settings";

    const char patch_size[] = "patch_size";
    const char overlap[] = "overlap";
    const char canny_edge_threshold[] = "canny_edge_threshold";
    const char lamda[] = "regulation_parameter";
    const char min_num_patches[] = "min_num_patches";
    const char num_test[] = "num_test";
    const char root[] = "root";
    const char k[] = "k";
    const char fuse_model[] = "fuse_model";*/
}

#define MAKE_KEY(key_name) const char key_##key_name[] = #key_name
// -------- tree ---------
MAKE_KEY(root);
MAKE_KEY(n_node);
MAKE_KEY(n_leaf);
MAKE_KEY(settings);
MAKE_KEY(trees);
// -------- binary test -------
MAKE_KEY(p1);
MAKE_KEY(p2);
MAKE_KEY(r);
// --------- node ----------
MAKE_KEY(left);
MAKE_KEY(right);
MAKE_KEY(c);
MAKE_KEY(is_leaf);
MAKE_KEY(binary_test);
// --------- settings ------------
MAKE_KEY(patch_size);
MAKE_KEY(overlap);
MAKE_KEY(canny_edge_threshold);
MAKE_KEY(lamda);
MAKE_KEY(min_n_patches);
MAKE_KEY(n_test);
MAKE_KEY(k);
MAKE_KEY(fuse_model);
MAKE_KEY(layers);

#undef MAKE_KEY

// ------------------------- Model -----------------
void json::SerializeModel(const EMat & model, AllocatorType & allocator, Value* model_arr)
{
    model_arr->Clear();
    model_arr->SetArray();
    for (int col = 0; col < model.cols(); ++col)
    {
        for (int row = 0; row < model.rows(); ++row)
        {
            Real val = model(row, col);
            model_arr->PushBack(val, allocator);
        }
    }
}

EMat json::DeserializeModel(const Value & val, int patch_size)
{
    int len_vec_patch = patch_size * patch_size;
    EMat res = EMat::Zero(len_vec_patch, len_vec_patch);
    auto it_arr = val.Begin();
    for (int col = 0; col < len_vec_patch; ++col)
    {
        for (int row = 0; row < len_vec_patch; ++row)
        {
            res(row, col) = (Real)it_arr->GetDouble();
            ++it_arr;
        }
    }
    return res;
}

// ------------------------- Settings ---------------
void json::SerializeSettings(const Settings & settings, AllocatorType & allocator, Value* sets_obj)
{
    sets_obj->Clear();
    sets_obj->SetObject();
    sets_obj->AddMember(key_patch_size, settings.patch_size, allocator);
    sets_obj->AddMember(key_canny_edge_threshold, settings.canny_edge_threshold, allocator);
    sets_obj->AddMember(key_min_n_patches, settings.min_n_patches, allocator);
    sets_obj->AddMember(key_lamda, settings.lamda, allocator);
    sets_obj->AddMember(key_n_test, settings.n_test, allocator);
    sets_obj->AddMember(key_k, settings.k, allocator);
    sets_obj->AddMember(key_fuse_model, int(settings.fuse_option), allocator);
    sets_obj->AddMember(key_overlap, settings.overlap, allocator);
    sets_obj->AddMember(key_layers, settings.layers, allocator);
}

Settings json::DeserializeSettings(const Value & val)
{
    Settings settings;
    settings.patch_size = val[key_patch_size].GetInt();
    settings.min_n_patches = val[key_min_n_patches].GetInt();
    settings.n_test = val[key_n_test].GetInt();
    settings.canny_edge_threshold = val[key_canny_edge_threshold].GetDouble();
    settings.k = val[key_k].GetDouble();
    settings.lamda = val[key_lamda].GetDouble();
    settings.fuse_option = Settings::FuseModelOption(val[key_fuse_model].GetInt());
    settings.overlap = val[key_overlap].GetInt();
    settings.layers = val[key_layers].GetInt();
    return settings;
}

// ----------------------- Binary test ------------
BinaryTest json::DeserializeBinaryTest(const Value & val)
{
    BinaryTest test;
    test.p1 = val[key_p1].GetInt();
    test.p2 = val[key_p2].GetInt();
    test.r = val[key_r].GetDouble();
    return test;
}

void json::SerializeBinaryTest(const BinaryTest & test, AllocatorType & allocator, Value* test_obj)
{
    test_obj->SetObject();
    test_obj->AddMember(key_p1, test.p1, allocator);
    test_obj->AddMember(key_p2, test.p2, allocator);
    test_obj->AddMember(key_r, test.r, allocator);
}

// ------------------------ Node -----------------
void json::DeserializeNode(const Value & val, int patch_size, DTNode* node)
{
    node->is_leaf = val[key_is_leaf].GetBool();
    if (node->is_leaf)
    {
        node->c = DeserializeModel(val[key_c], patch_size);
    }
    else
    {
        node->test = DeserializeBinaryTest(val[key_binary_test]);
        node->CreateLeft();
        node->CreateRight();
        DeserializeNode(val[key_left], patch_size, node->GetLeft());
        DeserializeNode(val[key_right], patch_size, node->GetRight());
    }
}

void json::SerializeNode(DTNode* node, AllocatorType & allocator, Value* node_obj)
{
    node_obj->SetObject();
    node_obj->AddMember(key_is_leaf, node->is_leaf, allocator);
    if (node->is_leaf)
    {
        // store the regression model for leaf node
        Value model_val;
        SerializeModel(node->c, allocator, &model_val);
        node_obj->AddMember(key_c, model_val, allocator);
    }
    else
    {
        // store the binary test
        Value left_node_val;
        Value right_node_val;
        Value binary_test_val;
        SerializeNode(node->GetLeft(), allocator, &left_node_val);
        SerializeNode(node->GetRight(), allocator, &right_node_val);
        SerializeBinaryTest(node->test, allocator, &binary_test_val);

        node_obj->AddMember(key_binary_test, binary_test_val.Move(), allocator);
        node_obj->AddMember(key_left, left_node_val.Move(), allocator);
        node_obj->AddMember(key_right, right_node_val.Move(), allocator);
    }
}

// ------------------------ DTree ---------------------
void json::SerializeDTree(const DTree & tree, AllocatorType & allocator, Value* tree_obj)
{
    tree_obj->SetObject();

    Value root_obj;
    SerializeNode(tree.GetRoot(), allocator, &root_obj);

    //Value sets_obj;
    //SerializeSettings(tree.settings, allocator, &sets_obj);

    Value n_leaf(tree.GetNumLeafNodes());
    Value n_node(tree.GetNumNodes());

    //tree_obj->AddMember(key_settings, sets_obj, allocator);
    tree_obj->AddMember(key_n_leaf, n_leaf, allocator);
    tree_obj->AddMember(key_n_node, n_node, allocator);
    tree_obj->AddMember(key_root, root_obj, allocator);
}

void json::DeserializeDTree(const Value & tree_obj, DTree* tree_out_ptr)
{
    assert(tree_out_ptr != nullptr);
    DTree & tree = *tree_out_ptr;

    DeserializeNode(tree_obj[key_root], tree.settings.patch_size, tree.GetRoot());

    assert(tree.GetNumLeafNodes() == tree_obj[key_n_leaf].GetInt());
    assert(tree.GetNumNodes() == tree_obj[key_n_node].GetInt());
}

// ------------------------ HDTrees -------------------------
void json::SerializeHDTrees(const HDTrees & hdtrees, AllocatorType & alloc, Value * hdt_obj)
{
    hdt_obj->SetObject();

    //Value sets_obj;
    //SerializeSettings(hdtrees.settings, alloc, &sets_obj);

    Value trees_obj;
    trees_obj.SetArray();
    for (const auto & tree : hdtrees.trees)
    {
        Value tree_obj;
        SerializeDTree(tree, alloc, &tree_obj);

        trees_obj.PushBack(tree_obj, alloc);
    }

    //hdt_obj->AddMember(key_settings, sets_obj, alloc);
    hdt_obj->AddMember(key_trees, trees_obj, alloc);
}

void json::DeserializeHDTrees(const Value & hdt_obj, HDTrees * tree_out)
{
    if (!tree_out) return;

    HDTrees & hdt = *tree_out;

    //hdt.settings = json::DeserializeSettings(hdt_obj[key_settings]);
    hdt.trees.clear();

    const auto & arr = hdt_obj[key_trees].GetArray();
    int size = arr.Size();

    hdt.trees.reserve(size);
    for (int i = 0; i < size; ++i)
        hdt.trees.push_back(DTree(hdt.settings));

    for (int i = 0; i < size; ++i)
    {
        json::DeserializeDTree(arr[i], &hdt.trees[i]);
    }
}
