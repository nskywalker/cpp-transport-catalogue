#include "map_renderer.h"

using namespace tor;

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршрутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */
bool renderer::IsZero(double value)  {
    return std::abs(value) < EPSILON;
}

svg::Point renderer::SphereProjector::operator()(ctg::geo::Coordinates coords) const {
    return {
            (coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_
    };
}

svg::Polyline
renderer::MapRenderer::GetSvgObjects(const std::vector<ctg::geo::Coordinates> &route) const {
    static size_t i = 0;
    svg::Polyline line;
    if (i == settings_.at(color_palette).AsArray().size()) {
        i = 0;
    }
    const auto& cur_color = settings_.at(color_palette).AsArray().at(i++);
    if (cur_color.IsString()) {
        line.SetStrokeColor(cur_color.AsString());
    }
    else if (cur_color.IsArray()) {
        auto red = static_cast<uint8_t>(cur_color.AsArray()[0].AsInt());
        auto green = static_cast<uint8_t>(cur_color.AsArray()[1].AsInt());
        auto blue = static_cast<uint8_t>(cur_color.AsArray()[2].AsInt());
        if (cur_color.AsArray().size() == 4) {
            double opacity = cur_color.AsArray()[3].AsDouble();
            line.SetStrokeColor(svg::Rgba(red, green, blue, opacity));
        }
        else {
            line.SetStrokeColor(svg::Rgb(red, green, blue));
        }
    }
    line.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND).SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetFillColor("none").SetStrokeWidth(settings_.at(line_width).AsDouble());
    for (const auto& stp : route) {
        line.AddPoint(projector->operator()(stp));
    }
    return line;
}

renderer::MapRenderer::MapRenderer(const json::Dict &settings, const ctg::catalogue::TransportCatalogue &db)
        : settings_(settings), db_(db){

}

svg::Text
renderer::MapRenderer::GetBusUndelayerSvg(std::string_view bus_name, const ctg::geo::Coordinates &cur_stop) const {
    svg::Text text = GetBusCommonProperties(bus_name, cur_stop);
    const auto& cur_color = settings_.at(underlayer_color);
    if (cur_color.IsString()) {
        text.SetFillColor(cur_color.AsString()).SetStrokeColor(cur_color.AsString());
    }
    else if (cur_color.IsArray()) {
        auto clr = cur_color.AsArray();
        auto red = static_cast<uint8_t>(clr[0].AsInt());
        auto green = static_cast<uint8_t>(clr[1].AsInt());
        auto blue = static_cast<uint8_t>(clr[2].AsInt());
        if (clr.size() == 4) {
            double opacity = clr[3].AsDouble();
            svg::Rgba color(red, green, blue, opacity);
            text.SetStrokeColor(color).SetFillColor(color);
        }
        else {
            svg::Rgb color(red, green, blue);
            text.SetStrokeColor(color).SetFillColor(color);
        }
    }

    return text.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
    .SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeWidth(settings_.at(underlayer_width).AsDouble());
}

svg::Text renderer::MapRenderer::GetBusNameSvg(std::string_view bus_name, const ctg::geo::Coordinates &cur_stop) const {
    static std::string prev_name = std::string{bus_name};
    svg::Text text = GetBusCommonProperties(bus_name, cur_stop);
    static size_t i = 0;
    if (prev_name != bus_name) {
        ++i;
        prev_name = std::string{bus_name};
    }
    if (i == settings_.at(color_palette).AsArray().size()) {
        i = 0;
    }
    const auto& cur_color = settings_.at(color_palette).AsArray().at(i);
    if (cur_color.IsString()) {
        text.SetFillColor(cur_color.AsString());
    }
    else if (cur_color.IsArray()) {
        auto red = static_cast<uint8_t>(cur_color.AsArray()[0].AsInt());
        auto green = static_cast<uint8_t>(cur_color.AsArray()[1].AsInt());
        auto blue = static_cast<uint8_t>(cur_color.AsArray()[2].AsInt());
        if (cur_color.AsArray().size() == 4) {
            double opacity = cur_color.AsArray()[3].AsDouble();
            text.SetFillColor(svg::Rgba(red, green, blue, opacity));
        }
        else {
            text.SetFillColor(svg::Rgb(red, green, blue));
        }
    }
    return text;
}

svg::Text
renderer::MapRenderer::GetBusCommonProperties(std::string_view bus_name, const ctg::geo::Coordinates &cur_stop) const {
    svg::Text text;
    text.SetFontSize(settings_.at(bus_label_font_size).AsInt())
            .SetFontFamily("Verdana").SetFontWeight("bold").SetData(std::string{bus_name})
            .SetPosition(projector->operator()(cur_stop))
            .SetOffset(svg::Point(settings_.at(bus_label_offset).AsArray()[0].AsDouble(), settings_.at(bus_label_offset).AsArray()[1].AsDouble()));
    return text;
}

svg::Circle renderer::MapRenderer::GetStopCircle(const ctg::geo::Coordinates &cur_stop) const {
    svg::Circle circle;
    circle.SetCenter(projector->operator()(cur_stop))
            .SetRadius(settings_.at(stop_radius).AsDouble()).SetFillColor("white");
    return circle;
}

svg::Text renderer::MapRenderer::GetCommonPropertiesStopName(std::string_view stop_name,
                                                             const ctg::geo::Coordinates &cur_stop) const {
    svg::Text text;
    text.SetFontSize(settings_.at(stop_label_font_size).AsInt())
            .SetFontFamily("Verdana").SetData(std::string{stop_name})
            .SetPosition(projector->operator()(cur_stop))
            .SetOffset(svg::Point(settings_.at(stop_label_offset).AsArray()[0].AsDouble(), settings_.at(stop_label_offset).AsArray()[1].AsDouble()));
    return text;;
}

svg::Text renderer::MapRenderer::GetStopName(std::string_view stop_name, const ctg::geo::Coordinates &cur_stop) const {
    return GetCommonPropertiesStopName(stop_name, cur_stop).SetFillColor("black");
}

svg::Text
renderer::MapRenderer::GetStopUnderlayer(std::string_view stop_name, const ctg::geo::Coordinates &cur_stop) const {
    svg::Text text = GetCommonPropertiesStopName(stop_name, cur_stop);
    const auto& cur_color = settings_.at(underlayer_color);
    if (cur_color.IsString()) {
        text.SetFillColor(cur_color.AsString()).SetStrokeColor(cur_color.AsString());
    }
    else if (cur_color.IsArray()) {
        auto clr = cur_color.AsArray();
        auto red = static_cast<uint8_t>(clr[0].AsInt());
        auto green = static_cast<uint8_t>(clr[1].AsInt());
        auto blue = static_cast<uint8_t>(clr[2].AsInt());
        if (clr.size() == 4) {
            double opacity = clr[3].AsDouble();
            svg::Rgba color(red, green, blue, opacity);
            text.SetStrokeColor(color).SetFillColor(color);
        }
        else {
            svg::Rgb color(red, green, blue);
            text.SetStrokeColor(color).SetFillColor(color);
        }
    }
    return text.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeWidth(settings_.at(underlayer_width).AsDouble());
}

svg::Document renderer::MapRenderer::RenderMap() {
    const auto all_stops = GetAllStopsCoordsInBuses();
    if (!projector) {
        projector = std::make_unique<renderer::SphereProjector>(all_stops.begin(), all_stops.end(),
                                                               settings_.at(tor::width).AsDouble(),
                                                               settings_.at(tor::height).AsDouble(),
                                                               settings_.at(tor::padding).AsDouble());
    }
    svg::Document doc;
    const auto& routes = db_.GetAllRoutes();
    std::map<std::string_view, std::vector<ctg::catalogue::Stop*>> rut = {routes.begin(), routes.end()};
    FormingRoutePolylines(doc, rut);
    FormingRoutesName(doc, rut);
    FormingStopCircles(doc, all_stops);
    FormingStopsName(doc);
    return doc;
}

std::vector<ctg::geo::Coordinates> renderer::MapRenderer::GetAllStopsCoordsInBuses() const {
    const auto unsorted_stops = GetSortedStops();
    std::vector<ctg::geo::Coordinates> result;
    result.reserve(unsorted_stops.size());
    std::for_each(unsorted_stops.begin(), unsorted_stops.end(), [&](const auto& cur_stop){
        if (db_.GetStopInBuses(cur_stop.first)) {
            result.emplace_back(*cur_stop.second->coords);
        }
    });
    return result;
}

std::map<std::string_view, ctg::catalogue::Stop *> renderer::MapRenderer::GetSortedStops() const {
    return std::map<std::string_view, ctg::catalogue::Stop *>{db_.GetAllStops().begin(), db_.GetAllStops().end()};
}

void renderer::MapRenderer::FormingRoutePolylines(svg::Document &doc,
                                                  const std::map<std::string_view, std::vector<ctg::catalogue::Stop *>> &rut) const {
    for (const auto& [bus_name, route] : rut) {
        std::vector<ctg::geo::Coordinates> coords;
        coords.reserve(route.size());
        for (const auto& cur_stop : route) {
            coords.emplace_back(cur_stop->coords.value());
        }
        std::unique_ptr<svg::Object> ptr = std::make_unique<svg::Polyline>(GetSvgObjects(coords));
        doc.AddPtr(std::move(ptr));
    }
}

void renderer::MapRenderer::FormingRoutesName(svg::Document &doc,
                                              const std::map<std::string_view, std::vector<ctg::catalogue::Stop *>> &rut) const {
    for (const auto& [bus_name, route] : rut) {
        if (route.empty()) {
            continue;
        }
        auto bus_underlayer = GetBusUndelayerSvg(bus_name, *route[0]->coords);
        auto bus_svg = GetBusNameSvg(bus_name, route[0]->coords.value());
        doc.AddPtr(std::make_unique<svg::Text>(bus_underlayer));
        doc.AddPtr(std::make_unique<svg::Text>(bus_svg));
        if (!db_.GetRoundTripRoute(bus_name)) {
            size_t pos = route.size() / 2;
            if (route[0]->name != route[pos]->name) {
                auto bus_underlayer_back = GetBusUndelayerSvg(bus_name, route[pos]->coords.value());
                auto bus_svg_back = GetBusNameSvg(bus_name, route[pos]->coords.value());
                doc.AddPtr(std::make_unique<svg::Text>(bus_underlayer_back));
                doc.AddPtr(std::make_unique<svg::Text>(bus_svg_back));
            }
        }
    }
}

void renderer::MapRenderer::FormingStopCircles(svg::Document &doc, const std::vector<ctg::geo::Coordinates> &all_stops) const {
    for (const auto& cur_stop : all_stops) {
        doc.AddPtr(std::make_unique<svg::Circle>(GetStopCircle(cur_stop)));
    }
}

void renderer::MapRenderer::FormingStopsName(svg::Document &doc) const {
    for (const auto& [cur_stop, stop_struct] : GetSortedStops()) {
        if (db_.GetStopInBuses(cur_stop)) {
            doc.AddPtr(std::make_unique<svg::Text>(GetStopUnderlayer(cur_stop, stop_struct->coords.value())));
            doc.AddPtr(std::make_unique<svg::Text>(GetStopName(cur_stop, stop_struct->coords.value())));
        }
    }
}
