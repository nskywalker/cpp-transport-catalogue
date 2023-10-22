#include "json_builder.h"


json::ArrayItemContext json::Builder::StartArray() {
    if (fill_root) {
        throw std::logic_error("Fill root");
    }
    if (!root_) {
        root_ = Array{};
        nodes_stack_.emplace_back(&root_.value());
    }
    else {
        nodes_stack_.push_back(new Node(Array{}));
    }
    is_notfilled_key = false;
    return ArrayItemContext{*this};
}

json::Builder & json::Builder::EndArray() {
    if (nodes_stack_.empty() or !nodes_stack_.back()->IsArray()) {
        throw std::logic_error("Not Array or empty");
    }
    if (nodes_stack_.size() > 1) {
        auto prev = nodes_stack_.end() - 2;
        if ((*prev)->IsArray()) {
            std::get<Array>((*prev)->GetValue()).emplace_back(*nodes_stack_.back());
        }
        else if ((*prev)->IsDict()) {
            if (!keys.empty()) {
                std::get<Dict>((*prev)->GetValue()).insert({keys.back(), *nodes_stack_.back()});
                keys.pop_back();
                is_notfilled_key = false;
            }
            else {
                throw std::logic_error("Empty keys");
            }
        }
        nodes_stack_.pop_back();
    }
    else {
        nodes_stack_.clear();
    }
    return *this;
}



json::Node json::Builder::Build() {
    if (root_ and nodes_stack_.empty()) {
        return *root_;
    }
    throw std::logic_error("Not build");
}

json::ValueDictItemContext json::Builder::Key(const std::string &key) {
    if (is_notfilled_key) {
        throw std::logic_error("notfilled key");
    }
//    Node* node = nullptr;
    Node* node = !nodes_stack_.empty() ? nodes_stack_.back() : throw std::logic_error("Stack are empty");
//    if (!nodes_stack_.empty()) {
//        node = nodes_stack_.back();
//    }
//    else {
//        throw std::logic_error("Stack are empty");
//    }
    if (node->IsDict()) {
        keys.push_back(key);
    }
    else {
        throw std::logic_error("Key without Dict");
    }
    is_notfilled_key = true;
    return ValueDictItemContext{*this};
}

json::KeyDictItemContext json::Builder::StartDict() {
    if (fill_root) {
        throw std::logic_error("Fill root");
    }
    if (!root_) {
        root_ = Dict {};
        nodes_stack_.emplace_back(&root_.value());
    }
    else {
        auto node = nodes_stack_.back();
        if (node->IsDict() and keys.empty()) {
            throw std::logic_error("empty dict");
        }
        nodes_stack_.push_back(new Node(Dict{}));
    }
    is_notfilled_key = false;
    return KeyDictItemContext{*this};
}

json::Builder & json::Builder::EndDict() {
    if (nodes_stack_.empty() or !nodes_stack_.back()->IsDict()) {
        throw std::logic_error("Not Dict or empty");
    }
    if (nodes_stack_.size() > 1) {
        auto prev = nodes_stack_.end() - 2;
        if ((*prev)->IsArray()) {
            std::get<Array>((*prev)->GetValue()).emplace_back(*nodes_stack_.back());
        }
        else if ((*prev)->IsDict()) {
            if (!keys.empty()) {
                std::get<Dict>((*prev)->GetValue()).insert({keys.back(), *nodes_stack_.back()});
                keys.pop_back();
            }
            else {
                throw std::logic_error("err");
            }
        }
        nodes_stack_.pop_back();
    }
    else {
        nodes_stack_.clear();
    }
    return *this;
}


json::ValueDictItemContext json::ItemContext::Key(const std::string &key) {
    return builder.Key(key);
}

json::Node json::ItemContext::Build() {
    return builder.Build();
}

json::ItemContext json::ItemContext::Value(Node value) {
    return builder.Value<ItemContext>(std::move(value));
}

json::KeyDictItemContext json::ItemContext::StartDict() {
    return builder.StartDict();
}

json::Builder &json::ItemContext::EndDict() {
    return builder.EndDict();
}

json::ArrayItemContext json::ItemContext::StartArray() {
    return builder.StartArray();
}

json::Builder &json::ItemContext::EndArray() {
    return builder.EndArray();
}

json::KeyDictItemContext json::ValueDictItemContext::Value(json::Node node) {
    return builder.Value<KeyDictItemContext>(std::move(node));
}

json::ArrayItemContext json::ArrayItemContext::Value(json::Node value) {
    return builder.Value<ArrayItemContext>(std::move(value));
}
