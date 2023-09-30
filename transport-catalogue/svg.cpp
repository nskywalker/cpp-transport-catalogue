#include "svg.h"

#include <utility>

namespace svg {

    using namespace std::literals;

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

// ---------- Circle ------------------

    Circle& Circle::SetCenter(Point center)  {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius)  {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << R"(<circle cx=")" << center_.x << R"(" cy=")" << center_.y << R"(" )";
        out << R"(r=")" << radius_ << R"(")";
        RenderAttrs(out);
        out << "/>"sv;
    }

    Polyline &Polyline::AddPoint(Point point) {
        points.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext &context) const {
        auto& out = context.out;
        out << R"(<polyline points=")";
        for (auto it = points.begin(); it != points.end(); ++it) {
            out << it->x << ","sv << it->y;
            if (std::distance(it, points.end()) > 1) {
                out << " "sv;
            }
        }
        out << R"(")";
        RenderAttrs(out);
        out << R"(/>)";
    }

    Text &Text::SetPosition(Point pos) {
        pos_ = pos;
        return *this;
    }

    Text &Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    Text &Text::SetFontSize(uint32_t size) {
        size_ = size;
        return *this;
    }

    Text &Text::SetFontFamily(std::string font_family) {
        font_family_ = std::move(font_family);
        return *this;
    }

    Text &Text::SetFontWeight(std::string font_weight) {
        font_weight_ = std::move(font_weight);
        return *this;
    }

    Text &Text::SetData(std::string data) {
        data_ = std::move(data);
        return *this;
    }

    void Text::RenderObject(const RenderContext &context) const {
        auto& out = context.out;
        out << R"(<text)";
        RenderAttrs(out);
        out << R"( x=")" << pos_.x << R"(")" << R"( y=")" << pos_.y << R"(")";
        out << R"( dx=")" << offset_.x << R"(")" << R"( dy=")" << offset_.y << R"(")";
        out << R"( font-size=")" << size_;
        if (!font_family_.empty()) {
            out  << R"(" )" << R"(font-family=")" << font_family_;
        }
        if (!font_weight_.empty()) {
            out  << R"(" )" << R"(font-weight=")" << font_weight_;
        }
        out  << R"(">)" << data_ << R"(</text>)";
    }

    void Document::AddPtr(std::unique_ptr<Object> &&obj) {
        objects.push_back(std::move(obj));
    }

    void Document::Render(std::ostream &out) const {
        RenderContext r(out);
        out << R"(<?xml version="1.0" encoding="UTF-8" ?>)" << std::endl;
        out << R"(<svg xmlns="http://www.w3.org/2000/svg" version="1.1">)"<< std::endl;
        for (const auto& obj : objects) {
            out << "  ";
            obj->Render(r);
        }
        out << "</svg>"sv;
    }

    std::ostream &operator<<(std::ostream &out, StrokeLineCap lineCap)  {
        switch (lineCap) {
            case StrokeLineCap::SQUARE: {
                out << R"(square)";
                break;
            }
            case StrokeLineCap::BUTT: {
                out << R"(butt)";
                break;
            }
            case StrokeLineCap::ROUND: {
                out << R"(round)";
                break;
            }
        }
        return out;
    }

    std::ostream &operator<<(std::ostream &out, StrokeLineJoin lineJoin)  {
        switch (lineJoin) {
            case StrokeLineJoin::ARCS: {
                out << R"(arcs)";
                break;
            }
            case StrokeLineJoin::BEVEL: {
                out << R"(bevel)";
                break;
            }
            case StrokeLineJoin::ROUND: {
                out << R"(round)";
                break;
            }
            case StrokeLineJoin::MITER: {
                out << R"(miter)";
                break;
            }
            case StrokeLineJoin::MITER_CLIP: {
                out << R"(miter-clip)";
                break;
            }
        }
        return out;
    }

    std::ostream &operator<<(std::ostream &out, const Color &color)  {
        out << std::visit(ColorVisit{}, color);
        return out;
    }
}  // namespace svg