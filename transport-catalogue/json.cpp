#include "json.h"

using namespace std;

namespace json {

    namespace {

        Node LoadNode(istream& input);

        Node LoadArray(istream& input) {
            Array result;

            char t;
            input >> t;
            if (input.eof()) {
                throw json::ParsingError("zasada! array is empty");
            }
            input.putback(t);

            for (char c; input >> c && c != ']';) {
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }

            return Node(move(result));
        }

        Node LoadInt(istream& input) {
            /*
            std::ostringstream out;
            for (char c; input >> c;) {
                out << c;
            }
            std::string str(out.str());
            if (str.find('.') != std::string::npos or str.find('e') != std::string::npos) {
                return Node(std::stod(str));
            }
            return Node(std::stoi(str));
             */
            using namespace std::literals;

            std::string parsed_num;

            // Считывает в parsed_num очередной символ из input
            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            // Считывает одну или более цифр в parsed_num из input
            auto read_digits = [&input, read_char] {
                if (!std::isdigit(input.peek())) {
                    throw ParsingError("A digit is expected"s);
                }
                while (std::isdigit(input.peek())) {
                    read_char();
                }
            };

            if (input.peek() == '-') {
                read_char();
            }
            // Парсим целую часть числа
            if (input.peek() == '0') {
                read_char();
                // После 0 в JSON не могут идти другие цифры
            } else {
                read_digits();
            }

            bool is_int = true;
            // Парсим дробную часть числа
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

            // Парсим экспоненциальную часть числа
            if (int ch = input.peek(); ch == 'e' || ch == 'E') {
                read_char();
                if (ch = input.peek(); ch == '+' || ch == '-') {
                    read_char();
                }
                read_digits();
                is_int = false;
            }

            try {
                if (is_int) {
                    // Сначала пробуем преобразовать строку в int
                    try {
                        return std::stoi(parsed_num);
                    } catch (...) {
                        // В случае неудачи, например, при переполнении,
                        // код ниже попробует преобразовать строку в double
                    }
                }
                return std::stod(parsed_num);
            } catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        Node LoadNull(istream& input) {
            std::string str;
            str.reserve(4);
            for (auto it = std::istreambuf_iterator<char>(input); it != std::istreambuf_iterator<char>(); ++it) {
                if (*it == '\t' or *it == ' ' or *it == '\n' or *it == '\r' or *it == '"' or *it ==  '\\') {
                    continue;
                }
                if (*it == ',' or *it == ':' or *it == '}'){
                    break;
                }
                str.push_back(*it);
            }
            if (str == "null") {
                return Node();
            }
            throw json::ParsingError("zasada! wrong word for null");
        }

        Node LoadBull(istream& input) {
            std::string str;
            str.reserve(5);
            for (auto it = std::istreambuf_iterator<char>(input); it != std::istreambuf_iterator<char>(); ++it) {
                if (*it == '\t' or *it == ' ' or *it == '\n' or *it == '\r' or *it == '"' or *it ==  '\\') {
                    continue;
                }
                if (*it == ',' or *it == ':' or *it == '}'){
                    break;
                }
                str.push_back(*it);
            }
            if (str == "true") {
                return Node(true);
            }
            else if (str == "false") {
                return Node(false);
            }
            else {
                throw json::ParsingError("zasada! bool is not true or false");
            }
        }

        Node LoadString(istream& input) {
//    string line;
//    getline(input, line, '"');
            using namespace std::literals;

            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while (true) {
                if (it == end) {
                    // Поток закончился до того, как встретили закрывающую кавычку?
                    throw ParsingError("String parsing error");
                }
                const char ch = *it;
                if (ch == '"') {
                    // Встретили закрывающую кавычку
                    ++it;
                    break;
                } else if (ch == '\\') {
                    // Встретили начало escape-последовательности
                    ++it;
                    if (it == end) {
                        // Поток завершился сразу после символа обратной косой черты
                        throw ParsingError("String parsing error");
                    }
                    const char escaped_char = *(it);
                    // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
                    switch (escaped_char) {
                        case 'n':
                            s.push_back('\n');
                            break;
                        case 't':
                            s.push_back('\t');
                            break;
                        case 'r':
                            s.push_back('\r');
                            break;
                        case '"':
                            s.push_back('"');
                            break;
                        case '\\':
                            s.push_back('\\');
                            break;
                        default:
                            // Встретили неизвестную escape-последовательность
                            throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                    }
                } else if (ch == '\n' || ch == '\r') {
                    // Строковый литерал внутри- JSON не может прерываться символами \r или \n
                    throw ParsingError("Unexpected end of line"s);
                } else {
                    // Просто считываем очередной символ и помещаем его в результирующую строку
                    s.push_back(ch);
                }
                ++it;
            }

//    return s;
//}
            return Node(s);
        }

        Node LoadDict(istream& input) {
            Dict result;

            char t;
            input >> t;
            if (input.eof()) {
                throw json::ParsingError("zasada! dict is empty");
            }
            input.putback(t);

            for (char c; input >> c && c != '}';) {
                if (c == ',') {
                    input >> c;
                }
                string key = LoadString(input).AsString();
                input >> c;
                result.insert({move(key), LoadNode(input)});
            }

            return Node(move(result));
        }

        Node LoadNode(istream& input) {
            char c;
            input >> c;

            if (c == '[') {
                return LoadArray(input);
            }
            else if (c == 'n') {
                input.putback(c);
                return LoadNull(input);
            }
            else if (c == '{') {
                return LoadDict(input);
            } else if (c == '"') {
                return LoadString(input);
            } else if (c == 't' or c == 'f') {
                input.putback(c);
                return LoadBull(input);
            }
            else {
                input.putback(c);
                return LoadInt(input);
            }
        }

    }  // namespace

    Node::Node(Array array)
            : value(std::move(array)) {
    }

    Node::Node(Dict map) : value(std::move(map)) {
    }

    Node::Node(int val)
            : value(val) {
    }

    Node::Node(string value)
            : value(std::move(value)) {
    }

    const Array& Node::AsArray() const {
        if (IsArray()) {
            return std::get<Array>(value);
        }
        throw std::logic_error("zasada: not Array");
    }

    const Dict& Node::AsMap() const {
        if (IsMap()) {
            return std::get<Dict>(value);
        }
        throw std::logic_error("zasada: not map");
    }

    int Node::AsInt() const {
        if (IsInt()) {
            return std::get<int>(value);
        }
        throw std::logic_error("zasada: not int");
    }

    const string& Node::AsString() const {
        if (IsString()) {
            return std::get<std::string>(value);
        }
        throw std::logic_error("zasada: not string");
    }

    bool Node::IsInt() const {
        return holds_alternative<int>(value);
    }

    bool Node::IsDouble() const {
        return holds_alternative<double>(value) or holds_alternative<int>(value);
    }

    bool Node::IsPureDouble() const {
        return holds_alternative<double>(value);
    }

    bool Node::IsBool() const {
        return holds_alternative<bool>(value);
    }

    bool Node::IsString() const {
        return holds_alternative<std::string>(value);
    }

    bool Node::IsNull() const {
        return holds_alternative<std::nullptr_t>(value);
    }

    bool Node::IsArray() const {
        return holds_alternative<Array>(value);
    }

    bool Node::IsMap() const {
        return holds_alternative<Dict>(value);
    }

    bool Node::AsBool() const {
        if (IsBool()) {
            return std::get<bool>(value);
        }
        throw std::logic_error("zasada: not bool");
    }

    double Node::AsDouble() const {
        if (IsPureDouble()) {
            return std::get<double>(value);
        }
        if (IsInt()) {
            return static_cast<double>(std::get<int>(value));
        }
        throw std::logic_error("zasada: not double");
    }

    Node::Node(bool b) : value(b) {

    }

    Node::Node(double val) : value(val) {

    }

    bool Node::operator==(const Node &other) const {
        return value == other.value;
    }

    bool Node::operator!=(const Node &other) const {
        return value != other.value;
    }

    Document::Document(Node root)
            : root_(std::move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    bool Document::operator==(const Document &doc) {
        return root_ == doc.root_;
    }

    bool Document::operator!=(const Document &doc) {
        return root_ != doc.root_;
    }

    Document Load(istream& input) {
        return Document{LoadNode(input)};
    }

    void Print(const Document& doc, std::ostream& output) {
        output << std::visit(VisitNode{}, doc.GetRoot().GetValue());
    }

    std::string VisitNode::operator()(const Array &array) const {
        std::ostringstream out;
        out << '[';
        for (auto val = array.begin(); val != array.end();++val) {
            out << std::visit(*this, val->GetValue());
            if (val != array.end() - 1) {
                out << ',';
            }
        }
        out << ']';
        return out.str();
    }

    std::string VisitNode::operator()(const Dict &dict) const {
        std::ostringstream out;
        out << '{';
        for (auto it = dict.begin(); it != dict.end(); ++it) {
            out << operator()(it->first) << ':';
            out << std::visit(*this, it->second.GetValue());
            if (std::distance(it, dict.end()) != 1) {
                out << ',';
            }
        }
        out << '}';
        return out.str();
    }

    std::string VisitNode::operator()(bool b) const  {
        std::ostringstream out;
        out << std::boolalpha << b;
        return out.str();
    }

    std::string VisitNode::operator()(int i) const  {
        return std::to_string(i);
    }

    std::string VisitNode::operator()(double d) const  {
        std::ostringstream str;
        str << d;
        return str.str();
    }

    std::string VisitNode::operator()(const string &str) const {
        std::ostringstream out_str;
        for (char c : str) {
            switch(c) {
                case '\n': {
                    out_str << "\\n";
                    break;
                }
                case '\r': {
                    out_str << "\\r";
                    break;
                }
                case '\t': {
                    out_str << "\t";
                    break;
                }
                case '\"' : {
                    out_str << "\\\"";
                    break;
                }
                case '\\' : {
                    out_str << "\\\\";
                    break;
                }
                default: {
                    out_str << c;
                    break;
                }
            }
        }
        return "\"" + out_str.str() + "\"";
    }
}  // namespace json