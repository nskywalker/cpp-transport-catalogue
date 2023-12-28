#include "serialization.h"
#include <fstream>
#include <unordered_map>


void serialization::SerializationData::SerializeDataBaseInFile(const serialization::Path &path) {
    std::ofstream out(path, std::ios::binary);
    if (!out) {
        throw json::ParsingError("Cannot open file");
    }

    SerializeCatalogue();
    SerializeRenderSettings();
    SerializeTransportRoute();

    proto_db.SerializeToOstream(&out);

}

void serialization::SerializationData::DeserializeDataBaseFromFile(const Path &path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        throw json::ParsingError("Cannot open file");
    }

    proto_db.ParseFromIstream(&in);

    DeserializeCatalogue();
    DeserializeRenderSettings();
    DeserializeTransportRoute();

}

serialization::SerializationData::SerializationData(DataBase &db_) : db(db_) {

}

void serialization::SerializationData::SetRenderSettings(const json::Dict &render_settings_) {
    render_settings = &render_settings_;
}

renderer::MapRenderer *serialization::SerializationData::GetMapRenderer() {
    if (!renderer) {
        renderer = new renderer::MapRenderer(settings, db);
    }
    return renderer;
}

void serialization::SerializationData::SerializeCatalogue() {

    proto_catalogue::TransportCatalogue proto_catalogue;
//    std::unordered_map<std::string_view, uint32_t> stopname_to_id;
    stopname_to_id.reserve(db.GetAllStops().size());
    id_to_stopname.reserve(db.GetAllStops().size());
    proto_catalogue.mutable_stop()->Reserve(db.GetAllStops().size());
    for (const auto& stop : db.GetAllStops()) {
        static uint64_t id = 0;
        stopname_to_id.emplace(stop.first, id);
        id_to_stopname.emplace(id++, stop.first);
        auto& proto_stop = *proto_catalogue.add_stop();
        proto_stop.set_name(stop.second->name);
        auto& proto_coords = *proto_stop.mutable_coords();
        proto_coords.set_lng(stop.second->coords->lng);
        proto_coords.set_lat(stop.second->coords->lat);
    }

    const auto& routes = db.GetAllRoutes();
    proto_catalogue.mutable_route()->Reserve(db.GetAllRoutes().size());
    for (const auto& [bus, route] : routes) {
        auto& proto_route = *proto_catalogue.add_route();
        proto_route.set_bus(std::string{bus});
        proto_route.set_is_roundtrip(db.GetRoundTripRoute(bus));
        for (const auto& stop : route) {
            proto_route.add_stop(stopname_to_id.at(stop->name));
        }
    }

    proto_catalogue.mutable_stops_distance()->Reserve(db.GetAllDist().size());
    for (const auto& [stops, dist] : db.GetAllDist()) {
        auto& proto_distance = *proto_catalogue.add_stops_distance();
        proto_distance.set_from(stopname_to_id.at(stops.first->name));
        proto_distance.set_to(stopname_to_id.at(stops.second->name));
        proto_distance.set_dist(dist);
    }

    *proto_db.mutable_catalogue() = std::move(proto_catalogue);
}

void serialization::SerializationData::SerializeRenderSettings() {
    if (!render_settings) {
        throw json::ParsingError("Empty render settings");
    }

    proto_catalogue::RenderSettings proto_render_settings;

    for (const auto& [name, node] : *render_settings) {
        proto_catalogue::RenderData data;
        if (name == tor::bus_label_offset or name == tor::stop_label_offset) {
            data.mutable_number_as_double()->Reserve(node.AsArray().size());
            for (const auto& number : node.AsArray()) {
                data.add_number_as_double(number.AsDouble());
            }
        }
        else if (name == tor::stop_label_font_size or name == tor::bus_label_font_size) {
            data.set_number_as_int(node.AsInt());
        }
        else if (name == tor::color_palette) {
            proto_catalogue::Color color;
            if (node.IsString()) {
                *color.mutable_color_as_string() = node.AsString();
                data.mutable_color()->Add(std::move(color));
            }
            else if (node.IsArray()) {
                if ((node.AsArray().size() == 3 or node.AsArray().size() == 4) and
                    node.AsArray()[0].IsInt()) {
                    if (node.AsArray().size() == 3) {
                        proto_catalogue::ColorRGB c_rgb;
                        c_rgb.mutable_color_as_int()->Reserve(node.AsArray().size());
                        for (const auto& v : node.AsArray()) {
                            c_rgb.mutable_color_as_int()->Add(v.AsInt());
                        }
                        *color.mutable_color_rgb() = std::move(c_rgb);
                    }
                    else {
                        proto_catalogue::ColorRGBA c_rgba;
                        c_rgba.mutable_color_as_int()->Reserve(3);
                        for (int i = 0; i < 3; ++i) {
                            c_rgba.mutable_color_as_int()->Add(node.AsArray()[i].AsInt());
                        }
                        c_rgba.set_opacity(node.AsArray()[3].AsDouble());
                        *color.mutable_color_rgba() = std::move(c_rgba);
                    }
                }
                else {
                    data.mutable_color()->Reserve(node.AsArray().size());
                    for (const auto &clr: node.AsArray()) {
                        color.Clear();
                        if (clr.IsString()) {
                            *color.mutable_color_as_string() = clr.AsString();
                        } else if (clr.IsArray()) {
                            if (clr.AsArray().size() == 3) {
                                proto_catalogue::ColorRGB c_rgb;
                                c_rgb.mutable_color_as_int()->Reserve(clr.AsArray().size());
                                for (const auto &v: clr.AsArray()) {
                                    c_rgb.mutable_color_as_int()->Add(v.AsInt());
                                }
                                *color.mutable_color_rgb() = std::move(c_rgb);
                            } else {
                                proto_catalogue::ColorRGBA c_rgba;
                                c_rgba.mutable_color_as_int()->Reserve(3);
                                for (int i = 0; i < 3; ++i) {
                                    c_rgba.mutable_color_as_int()->Add(clr.AsArray()[i].AsInt());
                                }
                                c_rgba.set_opacity(clr.AsArray()[3].AsDouble());
                                *color.mutable_color_rgba() = std::move(c_rgba);
                            }
                        }
                        data.mutable_color()->Add(std::move(color));
                    }
                }
            }
        }
        else if (name == tor::underlayer_color) {
            proto_catalogue::Color color;
            if (node.IsString()) {
                *color.mutable_color_as_string() = node.AsString();
            }
            else if (node.IsArray()) {
                if (node.AsArray().size() == 3) {
                    proto_catalogue::ColorRGB c_rgb;
                    c_rgb.mutable_color_as_int()->Reserve(3);
                    for (const auto& v : node.AsArray()) {
                        c_rgb.mutable_color_as_int()->Add(v.AsInt());
                    }
                    *color.mutable_color_rgb() = std::move(c_rgb);
                }
                else {
                    proto_catalogue::ColorRGBA c_rgba;
                    c_rgba.mutable_color_as_int()->Reserve(3);
                    for (int i = 0; i < 3; ++i) {
                        c_rgba.mutable_color_as_int()->Add(node.AsArray()[i].AsInt());
                    }
                    c_rgba.set_opacity(node.AsArray()[3].AsDouble());
                    *color.mutable_color_rgba() = std::move(c_rgba);
                }
            }
            data.mutable_color()->Add(std::move(color));
        }
        else {
            data.add_number_as_double(node.AsDouble());
        }
        proto_render_settings.mutable_render_settings()->insert({name, data});
    }



    *proto_db.mutable_render_settings() = std::move(proto_render_settings);
}

void serialization::SerializationData::DeserializeCatalogue() {
    const proto_catalogue::TransportCatalogue& proto_catalogue = proto_db.catalogue();
//    std::unordered_map<uint32_t, std::string_view> id_to_stopname;
    id_to_stopname.reserve(proto_catalogue.stop_size());
    stopname_to_id.reserve(proto_catalogue.stop_size());
    for (int i = 0; i < proto_catalogue.stop_size(); ++i) {
        const auto& proto_stop = proto_catalogue.stop(i);
        id_to_stopname.emplace(i, proto_stop.name());
        stopname_to_id.emplace(proto_stop.name(), i);
        db.AddStop(proto_stop.name(),
                   ctg::geo::Coordinates{proto_stop.coords().lat(), proto_stop.coords().lng()});
    }

    for (int i = 0; i < proto_catalogue.stops_distance_size(); ++i) {
        const auto& proto_distance = proto_catalogue.stops_distance(i);
        db.SetDistStops(id_to_stopname.at(proto_distance.from()), id_to_stopname.at(proto_distance.to()),
                        proto_distance.dist());
    }

    for (int i = 0; i < proto_catalogue.route_size(); ++i) {
        const auto& proto_route = proto_catalogue.route(i);
        db.AddBus(proto_route.bus());
        db.SetRoundTripLastRoute(proto_route.is_roundtrip());
        for (int j = 0; j < proto_route.stop_size(); ++j) {
            db.AddStopToLastBus(std::string{id_to_stopname.at(proto_route.stop(j))});
        }
    }
}

void serialization::SerializationData::DeserializeRenderSettings() {
    const proto_catalogue::RenderSettings& proto_settings = proto_db.render_settings();

    for (const auto& [name, param] : proto_settings.render_settings()) {
        json::Node data;
        if (name == tor::bus_label_offset or name == tor::stop_label_offset) {
            json::Array arr;
            arr.reserve(param.number_as_double_size());
            for (const auto& number : param.number_as_double()) {
                arr.emplace_back(number);
            }
            data = std::move(arr);
        }
        else if (name == tor::stop_label_font_size or name == tor::bus_label_font_size) {
            data = param.number_as_int();
        }
        else if (name == tor::color_palette) {
            json::Node value;
            if (param.color_size() == 1) {
                const auto& color = param.color(0);
                json::Array arr;
                if (color.has_color_rgb()) {
                    arr.reserve(3);
                    for (const auto& v : color.color_rgb().color_as_int()) {
                        arr.emplace_back(v);
                    }
                    value = std::move(arr);
                }
                else if (color.has_color_rgba()) {
                    arr.reserve(4);
                    for (const auto& v : color.color_rgba().color_as_int()) {
                        arr.emplace_back(v);
                    }
                    arr.emplace_back(color.color_rgba().opacity());
                    value = std::move(arr);
                }
                else {
                    value = color.color_as_string();
                }
            }
            else {
                json::Array arr;
                arr.reserve(param.color_size());
                for (const auto& v : param.color()) {
                    if (v.has_color_rgb()) {
                        json::Array arr2;
                        arr2.reserve(3);
                        for (const auto c : v.color_rgb().color_as_int()) {
                            arr2.emplace_back(c);
                        }
                        arr.emplace_back(std::move(arr2));
                    }
                    else if (v.has_color_rgba()) {
                        json::Array arr2;
                        arr2.reserve(4);
                        for (const auto c : v.color_rgba().color_as_int()) {
                            arr2.emplace_back(c);
                        }
                        arr2.emplace_back(v.color_rgba().opacity());
                        arr.emplace_back(std::move(arr2));
                    }
                    else {
                        arr.emplace_back(v.color_as_string());
                    }
                }
                value = std::move(arr);
            }
            data = std::move(value);
        }
        else if (name == tor::underlayer_color) {
            json::Node value;
            const auto& color = param.color(0);
            if (color.has_color_rgb()) {
                json::Array arr;
                arr.reserve(3);
                for (const auto v : color.color_rgb().color_as_int()) {
                    arr.emplace_back(v);
                }
                value = std::move(arr);
            }
            else if (color.has_color_rgba()) {
                json::Array arr;
                arr.reserve(4);
                for (const auto v : color.color_rgba().color_as_int()) {
                    arr.emplace_back(v);
                }
                arr.emplace_back(color.color_rgba().opacity());
                value = std::move(arr);
            }
            else {
                value = color.color_as_string();
            }
            data = std::move(value);
        }
        else {
            data = param.number_as_double(0);
        }
        settings.emplace(name, std::move(data));
    }
    renderer = !renderer ?  new renderer::MapRenderer(settings, db) : throw json::ParsingError("renderer isn't empty");
}

void serialization::SerializationData::SetRouteSettings(const json::Dict &route_settings_) {
    route_settings = &route_settings_;
}

void serialization::SerializationData::SerializeTransportRoute() {
    if (!route_settings) {
        throw json::ParsingError("Route settings is not set");
    }
    router = new TransportRoute(db, route_settings->at(tor::bus_wait_time).AsDouble(),
                                route_settings->at(tor::bus_velocity).AsDouble());
    const auto& graph = router->GetGraph();
    proto_catalogue::DirectedWeightedGraph proto_graph;
    proto_graph.mutable_edges_()->Reserve(graph.GetEdgeCount());
    for (uint64_t i = 0; i < graph.GetEdgeCount(); ++i) {
        const auto& edge = graph.GetEdge(i);
        auto& proto_edge = *proto_graph.add_edges_();
        proto_edge.set_from(edge.from);
        proto_edge.set_to(edge.to);

        const auto& edgeinfo = edge.weight;
        auto& proto_edgeinfo = *proto_edge.mutable_weight();
        proto_edgeinfo.set_stops_count(edgeinfo.stops_count);
        proto_edgeinfo.set_bus(std::string{edgeinfo.bus});
        proto_edgeinfo.set_time(edgeinfo.time);

    }

    proto_graph.mutable_incidence_lists_()->Reserve(graph.GetVertexCount());
    for (uint64_t i = 0; i < graph.GetVertexCount(); ++i) {
        const auto& list = graph.GetIncidentEdges(i);
        proto_catalogue::IncidenceList temp;
        temp.mutable_edge_id()->Reserve(list.end() - list.begin());
        for (auto val : list) {
            temp.add_edge_id(val);
        }
        *proto_graph.add_incidence_lists_() = std::move(temp);
    }

    proto_catalogue::Router proto_router;
    *proto_router.mutable_graph() = std::move(proto_graph);

    const auto& routes_internal_data = router->GetRoute().GetRoutesInternalData();
    proto_catalogue::RoutesInternalData proto_routes_internal_data;
    proto_routes_internal_data.mutable_routes_internal_data()->Reserve(routes_internal_data.size());
    for (const auto & route_internal_data : routes_internal_data) {
        auto& proto_route_internal_data = *proto_routes_internal_data.add_routes_internal_data();
        proto_route_internal_data.mutable_route_internal_data()->Reserve(route_internal_data.size());
        for (const auto & route_data : route_internal_data) {
            auto& proto_data = *proto_route_internal_data.add_route_internal_data();
            if (!route_data) {
                proto_data.set_not_exist(true);
            }
            else {
                auto& p_data =  *proto_data.mutable_route_internal_data();
                auto& weight = *p_data.mutable_weight();
                weight.set_time(route_data->weight.time);
                weight.set_bus(std::string{route_data->weight.bus});
                weight.set_stops_count(route_data->weight.stops_count);

                if (!route_data->prev_edge) {
                    p_data.set_not_exist(true);
                }
                else {
                    p_data.set_prev_edge(*route_data->prev_edge);
                }
            }
        }
    }
    *proto_router.mutable_routes_internal_data() = std::move(proto_routes_internal_data);

    proto_catalogue::TransportRoute proto_transport_router;
    proto_transport_router.set_speed(route_settings->at(tor::bus_velocity).AsDouble());
    proto_transport_router.set_bus_wait_time(route_settings->at(tor::bus_wait_time).AsDouble());
    *proto_transport_router.mutable_route() = std::move(proto_router);
    *proto_db.mutable_transport_route() = std::move(proto_transport_router);
    delete router;
}

void serialization::SerializationData::DeserializeTransportRoute() {
    const auto& proto_transport_router = proto_db.transport_route();
    const auto& proto_router = proto_transport_router.route();
    const auto& proto_graph = proto_router.graph();

    std::vector<graph::Edge<ctg::catalogue::EdgeInfo>> edges_;
    edges_.reserve(proto_graph.edges__size());
    for (const auto& proto_edge : proto_graph.edges_()) {
        auto& edge = edges_.emplace_back();
        edge.from = proto_edge.from();
        edge.to = proto_edge.to();
        const auto& proto_weight = proto_edge.weight();
        edge.weight.stops_count = proto_weight.stops_count();
        edge.weight.time = proto_weight.time();
        edge.weight.bus = proto_weight.bus();
    }

    std::vector<std::vector<uint64_t>> incidence_lists_;
    incidence_lists_.reserve(proto_graph.incidence_lists__size());
    for (const auto& proto_list : proto_graph.incidence_lists_()) {
        auto& list = incidence_lists_.emplace_back();
        list.reserve(proto_list.edge_id_size());
        for (const auto l : proto_list.edge_id()) {
            list.emplace_back(l);
        }
    }

    const auto& proto_routes_internal_data = proto_router.routes_internal_data();
    graph::Router<ctg::catalogue::EdgeInfo>::RoutesInternalData routes_internal_data;
    routes_internal_data.reserve(proto_routes_internal_data.routes_internal_data_size());
    for (const auto& proto_route_internal_data : proto_routes_internal_data.routes_internal_data()) {
        auto& data = routes_internal_data.emplace_back();
        data.reserve(proto_route_internal_data.route_internal_data_size());
        for (const auto& proto_i : proto_route_internal_data.route_internal_data()) {
            auto& d = data.emplace_back();
            switch (proto_i.optional_internal_data_case()) {
                case proto_catalogue::OptionalRouteInternalData::kRouteInternalData: {
                    d = graph::Router<ctg::catalogue::EdgeInfo>::RouteInternalData();
                    d->weight.bus = proto_i.route_internal_data().weight().bus();
                    d->weight.time = proto_i.route_internal_data().weight().time();
                    d->weight.stops_count = proto_i.route_internal_data().weight().stops_count();
                    switch (proto_i.route_internal_data().optional_prev_edge_case()) {
                        case proto_catalogue::RouteInternalData::kPrevEdge:
                            d->prev_edge = proto_i.route_internal_data().prev_edge();
                            break;
                        case proto_catalogue::RouteInternalData::kNotExist:
                            d->prev_edge = std::nullopt;
                            break;
                        case proto_catalogue::RouteInternalData::OPTIONAL_PREV_EDGE_NOT_SET:
                            throw json::ParsingError("INVALID PREV_EDGE");
                            break;
                    }
                    break;
                }
                case proto_catalogue::OptionalRouteInternalData::kNotExist:
                    d = std::nullopt;
                    break;
                case proto_catalogue::OptionalRouteInternalData::OPTIONAL_INTERNAL_DATA_NOT_SET:
                    throw json::ParsingError("INVALID INTERNAL DATA");
                    break;
            }
        }
    }

    graph = std::make_unique<graph::DirectedWeightedGraph<ctg::catalogue::EdgeInfo>>(std::move(edges_),
            std::move(incidence_lists_));
    router = new TransportRoute(db, proto_transport_router.bus_wait_time(), proto_transport_router.speed(),
                                              id_to_stopname, stopname_to_id, std::move(graph), std::move(routes_internal_data));



}

TransportRoute * serialization::SerializationData::GetTransportRoute() {
    return router;
}

