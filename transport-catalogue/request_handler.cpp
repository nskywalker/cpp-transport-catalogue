#include <queue>
#include "request_handler.h"

using namespace json;
using namespace ctg::catalogue;
using namespace ctg::geo;
using namespace tor;
/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */
svg::Document RequestHandler::RenderMap() {
    const auto all_stops = GetAllStopsCoordsInBuses();
    if (!renderer_.GetSphereProjector()) {
        const auto& settings = renderer_.GetSettings();
        auto ptr = std::make_unique<renderer::SphereProjector>(all_stops.begin(), all_stops.end(),
                                                                settings.at(tor::width).AsDouble(),
                                                                settings.at(tor::height).AsDouble(),
                                                                settings.at(tor::padding).AsDouble());
        renderer_.SetSphereProjector(std::move(ptr));
    }
    svg::Document doc;
    const auto& routes = db_.GetAllRoutes();
    std::map<std::string_view, std::vector<ctg::catalogue::Stop*>> rut = {routes.begin(), routes.end()};
    for (const auto& [bus_name, route] : rut) {
        std::vector<ctg::geo::Coordinates> coords;
        coords.reserve(route.size());
        for (const auto& cur_stop : route) {
            coords.emplace_back(cur_stop->coords.value());
        }
        std::unique_ptr<svg::Object> ptr = std::make_unique<svg::Polyline>(renderer_.GetSvgObjects(coords));
        doc.AddPtr(std::move(ptr));
    }

    for (const auto& [bus_name, route] : rut) {
        if (route.empty()) {
            continue;
        }
        auto bus_underlayer = renderer_.GetBusUndelayerSvg(bus_name, *route[0]->coords);
        auto bus_svg = renderer_.GetBusNameSvg(bus_name, route[0]->coords.value());
        doc.AddPtr(std::make_unique<svg::Text>(bus_underlayer));
        doc.AddPtr(std::make_unique<svg::Text>(bus_svg));
        if (!db_.GetRoundTripRoute(bus_name)) {
            size_t pos = route.size() / 2;
            if (route[0]->name != route[pos]->name) {
                auto bus_underlayer_back = renderer_.GetBusUndelayerSvg(bus_name, route[pos]->coords.value());
                auto bus_svg_back = renderer_.GetBusNameSvg(bus_name, route[pos]->coords.value());
                doc.AddPtr(std::make_unique<svg::Text>(bus_underlayer_back));
                doc.AddPtr(std::make_unique<svg::Text>(bus_svg_back));
            }
        }
    }

    for (const auto& cur_stop : all_stops) {
        doc.AddPtr(std::make_unique<svg::Circle>(renderer_.GetStopCircle(cur_stop)));
    }

    for (const auto& [cur_stop, stop_struct] : GetSortedStops()) {
        if (db_.GetStopInBuses(cur_stop)) {
            doc.AddPtr(std::make_unique<svg::Text>(renderer_.GetStopUnderlayer(cur_stop, stop_struct->coords.value())));
            doc.AddPtr(std::make_unique<svg::Text>(renderer_.GetStopName(cur_stop, stop_struct->coords.value())));
        }
    }
    return doc;
}

RequestHandler::RequestHandler(ctg::catalogue::TransportCatalogue &db, renderer::MapRenderer &renderer,
                               const std::map<std::string, json::Node> &req_, std::ostream &out_)
        : db_(db), renderer_(renderer), requests(req_), out(out_) {

}

std::vector<ctg::geo::Coordinates> RequestHandler::GetAllStopsCoordsInBuses() const {
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

std::map<std::string_view, ctg::catalogue::Stop *> RequestHandler::GetSortedStops() const {
    return std::map<std::string_view, ctg::catalogue::Stop *>{db_.GetAllStops().begin(), db_.GetAllStops().end()};
}

void RequestHandler::ProcessingRequests() {
    ProcessingBaseRequests();
    ProcessingStatRequests();
}

void RequestHandler::ProcessingBaseRequests() {
    if (requests.find(base_requests) != requests.end()) {
        if(requests.at(base_requests).IsArray()) {
            FillCatalogue(requests.at(base_requests).AsArray());
        }
        else {
            throw json::ParsingError("zasada! base requests is not array");
        }
    }
    else {
        throw json::ParsingError("zasada! base requests doesn't exist");
    }
}

void RequestHandler::FillCatalogue(const json::Array &base_requests_array) {
    std::queue<Bus> queue_bus;
    for (const auto &node: base_requests_array) {
        if (node.IsMap()) {
            const auto &request = node.AsMap();
            if (request.at(type) == bus) {
                if (request.count(name) and request.count(stops) and request.count(is_roundtrip)) {
                    queue_bus.push({request.at(name).AsString(), request.at(stops).AsArray(),
                                    request.at(is_roundtrip).AsBool()});
                } else {
                    throw ParsingError("bus: wrong request");
                }
            } else if (request.at(type) == stop) {
                if (request.count(latitude) and request.count(longitude) and request.count(name)) {
                    auto lat = request.at(latitude).AsDouble();
                    auto lng = request.at(longitude).AsDouble();
                    db_.AddStop(request.at(name).AsString(), std::make_optional<Coordinates>(Coordinates{lat, lng}));
                    if (request.find(road_distances) != request.end()) {
                        for (const auto &cur_stop: request.at(road_distances).AsMap()) {
                            db_.AddStop(cur_stop.first, std::nullopt);
                            db_.SetDistStops(request.at(name).AsString(), cur_stop.first, cur_stop.second.AsDouble());
                        }
                    }
                } else {
                    throw ParsingError("stop: wrong request");
                }
            }
        } else {
            throw ParsingError("zasada! base request node is not map");
        }
    }

    while (!queue_bus.empty()) {
        const auto &cur_bus = queue_bus.front();
        db_.AddBus(cur_bus.name);
        for (const auto &cur_stop: cur_bus.stops) {
            db_.AddStopToLastBus(cur_stop.AsString());
        }
        db_.SetRoundTripLastRoute(cur_bus.is_roundtrip);
        if (!cur_bus.is_roundtrip) {
            for (int i = static_cast<int>(db_.FindLastRoute().size() - 2); i >= 0; --i) {
                db_.AddStopToLastBus(db_.FindLastRoute()[i]->name);
            }
        }
        queue_bus.pop();
    }
}

void RequestHandler::ProcessingStatRequests() {
    if (requests.find(stat_requests) != requests.end()) {
        if(requests.at(stat_requests).IsArray()) {
            FormingOutput(requests.at(stat_requests).AsArray());
        }
        else {
            throw ParsingError("zasada! stat requests is not array");
        }
    }
    else {
        throw ParsingError("zasada! stat requests doesn't exist");
    }
}

void RequestHandler::FormingOutput(const Array &stats) {
    json::Array answer;
    for (const auto &node: stats) {
        json::Dict req;
        if (node.IsMap()) {
            const auto &request = node.AsMap();
            if (request.at(type) == stop) {
                if (!db_.FindStop(request.at(name).AsString())) {
                    req[request_id] = request.at(id).AsInt();
                    req["error_message"] = std::string{"not found"};
                } else {
                    json::Array temp;
                    const auto *stop_inform_as_set = db_.GetStopInBuses(request.at(name).AsString());
                    if (stop_inform_as_set) {
                        for (auto it : *stop_inform_as_set) {
                            temp.emplace_back(std::string{it});
                        }
                    }
                    req["buses"] = temp;
                    req[request_id] = request.at(id).AsInt();
                }

            }
            else if (request.at(type) == bus) {
                const auto bus_inform = db_.GetBusInfo(request.at(name).AsString());
                if (bus_inform.has_value()) {
                    req[curvature] = bus_inform->curvature;
                    req[request_id] = request.at(id).AsInt();
                    req[route_length] = bus_inform->route_length;
                    req[stop_count] = static_cast<int>(bus_inform->number_of_stops);
                    req[unique_stop_count] = static_cast<int>(bus_inform->number_of_unique_stops);
                }
                else {
                    req[request_id] = (request.at(id).AsInt());
                    req["error_message"] = std::string{"not found"};
                }
            }
            else if (request.at(type) == Map) {
                auto res = RenderMap();
                std::stringstream ss;
                res.Render(ss);
                req[map] = ss.str();
                req[request_id] = request.at(id).AsInt();
            }
            else {
                throw ParsingError("stats: wrong type request");
            }
            answer.emplace_back(req);
        }
    }
    Print(answer);
//    if (!answer.empty()) {
//        out << visitNode(answer);
//    }
}

void RequestHandler::Print(const std::vector<json::Node> &answer) {
    if (answer.empty()) {
        return;
    }
    const std::string div = "    ";
    out << "[\n";
    for (const auto& node : answer) {
        out << div + "{\n";
        const auto& stat_answer = node.AsMap();
        for (auto it = stat_answer.begin(); it != stat_answer.end(); ++it) {
            out << div + div << visitNode(it->first) << ": " << std::visit(visitNode, it->second.GetValue());
            if (std::distance(it, stat_answer.end()) != 1) {
                out << ',';
            }
            out << '\n';
        }
        out << div + "}";
        if (&node != &answer.back()) {
            out << ',';
        }
        out << '\n';
    }
    out << ']';
}
