#include <queue>
#include "json_reader.h"

using namespace json;
using namespace ctg::catalogue;
using namespace ctg::geo;
using namespace tor;

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */
JsonReader::JsonReader(ctg::catalogue::TransportCatalogue &db, std::istream &in, std::ostream& out_)
        : db_(db), input(in), out(out_) {

}

void JsonReader::ProcessingRequest() {
    requests = std::make_unique<Document>(Load(input));
    if (!requests->GetRoot().IsMap()) {
        throw ParsingError("zasada! Form of data must be map");
    }
    ReadBaseRequests();
    ReadStatRequests();
}


void JsonReader::ReadBaseRequests() {
    const auto all_requests = requests->GetRoot().AsMap();
    if (all_requests.find(base_requests) != all_requests.end()) {
        if(all_requests.at(base_requests).IsArray()) {
            FillCatalogue(all_requests.at(base_requests).AsArray());
        }
        else {
            throw json::ParsingError("zasada! base requests is not array");
        }
    }
    else {
        throw json::ParsingError("zasada! base requests doesn't exist");
    }
}

void JsonReader::ReadStatRequests() {
    const auto all_requests = requests->GetRoot().AsMap();
    if (all_requests.find(stat_requests) != all_requests.end()) {
        if(all_requests.at(stat_requests).IsArray()) {
            FormingOutput(all_requests.at(stat_requests).AsArray());
        }
        else {
            throw ParsingError("zasada! stat requests is not array");
        }
    }
    else {
        throw ParsingError("zasada! stat requests doesn't exist");
    }
}

void JsonReader::FillCatalogue(const Array &base_requests_array) {
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

void JsonReader::FormingOutput(const Array &stats) {
    rend = std::make_unique<renderer::MapRenderer>(requests->GetRoot().AsMap().at(render_settings).AsMap(), db_);
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
                auto res = rend->RenderMap();
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
}

void JsonReader::Print(const std::vector<json::Node> &answer) {
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
