#pragma once
#include <string>
#include <optional>
#include "geo.h"
#include "json.h"

/*
 * В этом файле вы можете разместить классы/структуры, которые являются частью предметной области (domain)
 * вашего приложения и не зависят от транспортного справочника. Например Автобусные маршруты и Остановки.
 *
 * Их можно было бы разместить и в transport_catalogue.h, однако вынесение их в отдельный
 * заголовочный файл может оказаться полезным, когда дело дойдёт до визуализации карты маршрутов:
 * визуализатор карты (map_renderer) можно будет сделать независящим от транспортного справочника.
 *
 * Если структура вашего приложения не позволяет так сделать, просто оставьте этот файл пустым.
 *
 */

namespace ctg::catalogue {
    struct Stop {
        std::string name;
        std::optional<ctg::geo::Coordinates> coords;
    };

    struct BusInfo {
        size_t number_of_stops;
        size_t number_of_unique_stops;
        double route_length;
        double curvature;
    };

    struct Bus {
        const std::string& name;
        const json::Array& stops;
        bool is_roundtrip;
    };

    struct HashStopPair {
        size_t operator()(const std::pair<Stop *, Stop *> &stop_pair) const;

    private:
        std::hash<const void *> hasher;
    };

    struct EdgeInfo {
        int stops_count;
        double time;
        std::string_view bus;
    };

    bool operator<(const EdgeInfo& lhs, const EdgeInfo& rhs);

    bool operator>(const EdgeInfo& lhs, const EdgeInfo& rhs);

    EdgeInfo operator+(const EdgeInfo& lhs, const EdgeInfo& rhs);

    struct RouteInfo {
        json::Array items;
        double time;
    };
}

namespace tor {
    const std::string base_requests = "base_requests";
    const std::string stat_requests = "stat_requests";
    const std::string type = "type";
    const std::string stop = "Stop";
    const std::string stops = "stops";
    const std::string name = "name";
    const std::string is_roundtrip = "is_roundtrip";
    const std::string latitude = "latitude";
    const std::string longitude = "longitude";
    const std::string road_distances = "road_distances";
    const std::string bus = "Bus";
    const std::string curvature = "curvature";
    const std::string request_id = "request_id";
    const std::string id = "id";
    const std::string route_length = "route_length";
    const std::string stop_count = "stop_count";
    const std::string unique_stop_count = "unique_stop_count";
    const std::string render_settings = "render_settings";
    const std::string width = "width";
    const std::string height = "height";
    const std::string padding = "padding";
    const std::string line_width = "line_width";
    const std::string stop_radius = "stop_radius";
    const std::string bus_label_font_size = "bus_label_font_size";
    const std::string bus_label_offset = "bus_label_offset";
    const std::string stop_label_font_size = "stop_label_font_size";
    const std::string stop_label_offset = "stop_label_offset";
    const std::string underlayer_color = "underlayer_color";
    const std::string underlayer_width = "underlayer_width";
    const std::string color_palette = "color_palette";
    const std::string Map = "Map";
    const std::string map = "map";
    const std::string routing_settings = "routing_settings";
    const std::string Route = "Route";
    const std::string bus_wait_time = "bus_wait_time";
    const std::string bus_velocity = "bus_velocity";
} //tor