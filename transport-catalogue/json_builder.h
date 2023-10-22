#pragma once
#include <optional>
#include "json.h"

namespace json {
    class ItemContext;
    class Builder;
    class ArrayItemContext;
    class KeyDictItemContext;
    class ValueDictItemContext;

    class Builder {
        std::optional<Node> root_;
        std::vector<Node*> nodes_stack_;
        std::vector<std::string> keys;
        bool is_notfilled_key = false;
        bool fill_root = false;
    public:
        template<class T = ItemContext>
        T Value(Node value);
        ValueDictItemContext Key(const std::string& key);
        Node Build();
        KeyDictItemContext StartDict();
        Builder &EndDict();
        ArrayItemContext StartArray();
        Builder & EndArray();
    };


    class ItemContext {
    protected:
        Builder& builder;
    public:
        explicit ItemContext(Builder& builder1) : builder(builder1){}
        ValueDictItemContext Key(const std::string& key);
        Node Build();
        ItemContext Value(Node value);
        KeyDictItemContext StartDict();
        Builder & EndDict();
        ArrayItemContext StartArray();
        Builder & EndArray();
    };



    class KeyDictItemContext : public ItemContext {
    public:
        explicit KeyDictItemContext(Builder& builder1) : ItemContext(builder1){}
        ValueDictItemContext Value(Node value) = delete;
        Node Build() = delete;
        Builder & EndArray() = delete;
        KeyDictItemContext StartDict() = delete;
        ArrayItemContext StartArray() = delete;
    };

    class ValueDictItemContext : public ItemContext {
    public:
        explicit ValueDictItemContext(Builder& builder1) : ItemContext(builder1){}
        KeyDictItemContext Value(Node value);
        ValueDictItemContext Key(const std::string& key) = delete;
        Node Build() = delete;
        Builder & EndArray() = delete;
        Builder & EndDict() = delete;

    };

    class ArrayItemContext : public ItemContext {
    public:
        explicit ArrayItemContext(Builder& builder1) : ItemContext(builder1){}
        ArrayItemContext Value(Node value);
        ValueDictItemContext Key(const std::string& key) = delete;
        Node Build() = delete;
        Builder & EndDict() = delete;
    };
}

template <class T>
T json::Builder::Value(json::Node value) {
    Node* node = !nodes_stack_.empty() ? nodes_stack_.back() : nullptr;
    if (node) {
        if (node->IsArray()) {
            std::get<Array>(node->GetValue()).emplace_back(std::move(value));
        } else if (node->IsDict()) {
            if (!keys.empty()) {
                std::get<Dict>(node->GetValue()).insert({keys.back(), std::move(value)});
                keys.pop_back();
                is_notfilled_key = false;
            }
            else {
                throw std::logic_error("Keys are empty");
            }
        }
    }
    else if (!fill_root){
        root_ = value;
        fill_root = true;
    }
    else {
        root_ = std::nullopt;
        throw std::logic_error("All goes wrong");
    }
    return T{*this};
}