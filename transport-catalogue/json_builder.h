#include <sstream>
#include <optional>
#include "json.h"
#include <memory>

namespace json {
    class Builder;
    class ArrayItemContext;
    class KeyDictItemContext;
    class ValueDictItemContext;
    class SimpleValueItemContext;



    class Builder {
        std::optional<Node> root_;
        std::vector<Node*> nodes_stack_;
        std::vector<std::string> keys;
        bool is_notfilled_key = false;
        bool fill_root = false;
    public:
        template<class T = SimpleValueItemContext>
        T Value(Node value);
        ValueDictItemContext Key(const std::string& key);
        Node Build();
        KeyDictItemContext StartDict();
        Builder &EndDict();
        ArrayItemContext StartArray();
        Builder & EndArray();
    };


    template <class T>
    class ItemContext {
    protected:
        Builder& builder;
    public:
        explicit ItemContext(Builder& builder1) : builder(builder1){}
        ValueDictItemContext Key(const std::string& key);
        Node Build();
        T Value(Node value);
        KeyDictItemContext StartDict();
        Builder & EndDict();
        ArrayItemContext StartArray();
        Builder & EndArray();
    };




    class SimpleValueItemContext : public ItemContext<SimpleValueItemContext> {
    public:
        explicit SimpleValueItemContext(Builder& builder1) : ItemContext<SimpleValueItemContext>(builder1){}
//        Node Build();
//        SimpleValueItemContext Value(Node value) = delete;
//        KeyDictItemContext StartDict() = delete;
//        Builder EndDict() = delete;
//        ArrayItemContext StartArray() = delete;
//        Builder EndArray() = delete;
    };


    class KeyDictItemContext : public ItemContext<ValueDictItemContext> {
    public:
        explicit KeyDictItemContext(Builder& builder1) : ItemContext<ValueDictItemContext>(builder1){}
        ValueDictItemContext Value(Node value) = delete;
        Node Build() = delete;
        Builder & EndArray() = delete;
        KeyDictItemContext StartDict() = delete;
        ArrayItemContext StartArray() = delete;
    };

    class ValueDictItemContext : public ItemContext<KeyDictItemContext> {
    public:
        explicit ValueDictItemContext(Builder& builder1) : ItemContext<KeyDictItemContext>(builder1){}
        ValueDictItemContext Key(const std::string& key) = delete;
        Node Build() = delete;
        Builder & EndArray() = delete;
        Builder & EndDict() = delete;

    };

    class ArrayItemContext : public ItemContext<ArrayItemContext> {
    public:
        explicit ArrayItemContext(Builder& builder1) : ItemContext<ArrayItemContext>(builder1){}
        ValueDictItemContext Key(const std::string& key) = delete;
        Node Build() = delete;
        Builder & EndDict() = delete;
//        Builder & EndArray() = delete;
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

template<class T>
T json::ItemContext<T>::Value(Node value) {
    return builder.template Value<T>(std::move(value));
}

template<class T>
json::ValueDictItemContext json::ItemContext<T>::Key(const std::string &key) {
    return builder.Key(key);
}

template<class T>
json::KeyDictItemContext json::ItemContext<T>::StartDict() {
    return builder.StartDict();
}

template<class T>
json::Builder & json::ItemContext<T>::EndDict() {
    return builder.EndDict();
}

template<class T>
json::ArrayItemContext json::ItemContext<T>::StartArray() {
    return builder.StartArray();
}

template<class T>
json::Builder & json::ItemContext<T>::EndArray() {
    return builder.EndArray();
}

template<class T>
json::Node json::ItemContext<T>::Build() {
    return builder.Build();
}