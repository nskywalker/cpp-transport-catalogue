#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>
#include <sstream>

namespace json {

    class Node;
// Сохраните объявления Dict и Array без изменения
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    struct VisitNode {
        std::string operator()(std::nullptr_t) const {
            return "null";
        }
        std::string operator()(const Array& array) const;
        std::string operator()(const Dict& dict) const;
        std::string operator()(bool b) const;
        std::string operator()(int i)const;
        std::string operator()(double d) const;
        std::string operator()(const std::string& str) const;
    };

    class Node {
    public:
        /* Реализуйте Node, используя std::variant */
        using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;
        Node() = default;
        Node(Array array);
        Node(Dict map);
        Node(int val);
        Node(std::string value);
        Node(std::nullptr_t) : Node(){}
        Node(bool b);
        Node(double val);
        const Value& GetValue() const { return value; }

        const Array& AsArray() const;
        const Dict& AsMap() const;
        int AsInt() const;
        const std::string& AsString() const;
        bool AsBool() const;
        double AsDouble() const;

        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsBool() const;
        bool IsString() const;
        bool IsNull() const;
        bool IsArray() const;
        bool IsMap() const;

        bool operator==(const Node& other) const;
        bool operator!=(const Node& other) const;

    private:
        Value value;
    };

    class Document {
    public:
        explicit Document(Node root);
        bool operator==(const Document& doc);
        bool operator!=(const Document& doc);
        const Node& GetRoot() const;

    private:
        Node root_;
    };

    Document Load(std::istream& input);

    void Print(const Document& doc, std::ostream& output);

}  // namespace json