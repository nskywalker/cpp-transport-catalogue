#pragma once

#include "geo.h"
#include "svg.h"
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>
#include "json.h"
#include "domain.h"
#include "transport_catalogue.h"

namespace renderer {

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршрутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */
    inline const double EPSILON = 1e-6;

    bool IsZero(double value);

    class SphereProjector {
    public:
        // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
        template<typename PointInputIt>
        SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                        double max_width, double max_height, double padding);

        // Проецирует широту и долготу в координаты внутри SVG-изображения
        svg::Point operator()(ctg::geo::Coordinates coords) const;

    private:
        double padding_;
        double min_lon_ = 0;
        double max_lat_ = 0;
        double zoom_coeff_ = 0;
    };

    template<typename PointInputIt>
    SphereProjector::SphereProjector(PointInputIt points_begin, PointInputIt points_end, double max_width,
                                     double max_height, double padding) : padding_(padding) {
        // Если точки поверхности сферы не заданы, вычислять нечего
        if (points_begin == points_end) {
            return;
        }

        // Находим точки с минимальной и максимальной долготой
        const auto [left_it, right_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        // Находим точки с минимальной и максимальной широтой
        const auto [bottom_it, top_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        // Вычисляем коэффициент масштабирования вдоль координаты x
        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        // Вычисляем коэффициент масштабирования вдоль координаты y
        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            // Коэффициенты масштабирования по ширине и высоте ненулевые,
            // берём минимальный из них
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        } else if (width_zoom) {
            // Коэффициент масштабирования по ширине ненулевой, используем его
            zoom_coeff_ = *width_zoom;
        } else if (height_zoom) {
            // Коэффициент масштабирования по высоте ненулевой, используем его
            zoom_coeff_ = *height_zoom;
        }
    }

    class MapRenderer {
        const json::Dict &settings_;
        std::unique_ptr<SphereProjector> projector;
        const ctg::catalogue::TransportCatalogue& db_;
    protected:
        svg::Text GetBusCommonProperties(std::string_view bus_name, const ctg::geo::Coordinates &cur_stop) const;
        svg::Text GetCommonPropertiesStopName(std::string_view stop_name, const ctg::geo::Coordinates &cur_stop) const;
        std::vector<ctg::geo::Coordinates> GetAllStopsCoordsInBuses() const;
        std::map<std::string_view, ctg::catalogue::Stop*> GetSortedStops() const;
        svg::Polyline GetSvgObjects(const std::vector<ctg::geo::Coordinates> &route) const;
        svg::Text GetBusUndelayerSvg(std::string_view bus_name, const ctg::geo::Coordinates &cur_stop) const;
        svg::Text GetBusNameSvg(std::string_view bus_name, const ctg::geo::Coordinates &cur_stop) const;
        svg::Circle GetStopCircle(const ctg::geo::Coordinates &cur_stop) const;
        svg::Text GetStopName(std::string_view stop_name, const ctg::geo::Coordinates &cur_stop) const;
        svg::Text GetStopUnderlayer(std::string_view stop_name, const ctg::geo::Coordinates &cur_stop) const;
        void FormingRoutePolylines(svg::Document& doc, const std::map<std::string_view, std::vector<ctg::catalogue::Stop*>>& rut) const;
        void FormingRoutesName(svg::Document& doc, const std::map<std::string_view, std::vector<ctg::catalogue::Stop*>>& rut) const;
        inline void FormingStopCircles(svg::Document& doc, const std::vector<ctg::geo::Coordinates>& all_stops) const;
        void FormingStopsName(svg::Document& doc) const;
    public:
        explicit MapRenderer(const json::Dict &settings, const ctg::catalogue::TransportCatalogue &db);
        svg::Document RenderMap();
    };

}