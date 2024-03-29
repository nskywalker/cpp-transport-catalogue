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
        : db_(db), input(in), out(out_), s_data(db) {

}

void JsonReader::ProcessingRequest() {
    requests = std::make_unique<Document>(Load(input));
    if (!requests->GetRoot().IsDict()) {
        throw ParsingError("zasada! Form of data must be map");
    }
    ReadBaseRequests();
    ReadStatRequests();
}


void JsonReader::ReadBaseRequests() {
    requests = std::make_unique<Document>(Load(input));
    if (!requests->GetRoot().IsDict()) {
        throw ParsingError("zasada! Form of data must be map");
    }
    const auto all_requests = requests->GetRoot().AsDict();
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

    if (all_requests.find(serialization_settings) != all_requests.end()) {
        SerializeDatabase(all_requests);
    }
    else {
        throw json::ParsingError(serialization_settings + " doesn't exist");
    }
}

void JsonReader::ReadStatRequests() {
    requests = std::make_unique<Document>(Load(input));
    if (!requests->GetRoot().IsDict()) {
        throw ParsingError("zasada! Form of data must be map");
    }
    const auto all_requests = requests->GetRoot().AsDict();
    if (all_requests.find(serialization_settings) != all_requests.end()) {
        DeserializeDatabase(all_requests.at(serialization_settings).AsDict());
    }
    else {
        throw json::ParsingError(serialization_settings + " doesn't exist");
    }

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
        if (node.IsDict()) {
            const auto &request = node.AsDict();
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
                        for (const auto &cur_stop: request.at(road_distances).AsDict()) {
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
    json::Array answer;
    for (const auto &node: stats) {
        json::Dict req;
        if (node.IsDict()) {
            const auto &request = node.AsDict();
            if (request.at(type) == stop) {
                if (!db_.FindStop(request.at(name).AsString())) {
                    req = Builder{}.StartDict().Key(request_id).Value(request.at(id))
                            .Key("error_message").Value("not found").EndDict().Build().AsDict();
                } else {
                    json::Array temp;
                    const auto *stop_inform_as_set = db_.GetStopInBuses(request.at(name).AsString());
                    if (stop_inform_as_set) {
                        for (auto it : *stop_inform_as_set) {
                            temp.emplace_back(std::string{it});
                        }
                    }
                    req = Builder{}.StartDict().Key("buses").Value(temp).Key(request_id).Value(request.at(id))
                            .EndDict().Build().AsDict();
                }

            }
            else if (request.at(type) == bus) {
                const auto bus_inform = db_.GetBusInfo(request.at(name).AsString());
                if (bus_inform.has_value()) {
                    req = json::Builder{}.StartDict().Key(curvature).Value(bus_inform->curvature)
                            .Key(request_id).Value(request.at(id))
                            .Key(route_length).Value(bus_inform->route_length)
                            .Key(stop_count).Value(static_cast<int>(bus_inform->number_of_stops))
                            .Key(unique_stop_count).Value(static_cast<int>(bus_inform->number_of_unique_stops))
                            .EndDict().Build().AsDict();
                }
                else {
                    req = Builder{}.StartDict().Key(request_id).Value(request.at(id))
                            .Key("error_message").Value("not found").EndDict().Build().AsDict();
                }
            }
            else if (request.at(type) == Map) {
                if (!rend) {
                    rend.reset(s_data.GetMapRenderer());
//                    rend = std::make_unique<renderer::MapRenderer>(
//                            requests->GetRoot().AsDict().at(render_settings).AsDict(), db_);
                }
                auto res = rend->RenderMap();
                std::stringstream ss;
                res.Render(ss);
                req = Builder{}.StartDict().Key(map).Value(ss.str()).Key(request_id).Value(request.at(id))
                        .EndDict().Build().AsDict();
            }
            else if (request.at(type) == Route) {
                if (!route) {
                    route.reset(s_data.GetTransportRoute());
                    /*const auto& settings = requests->GetRoot().AsDict().at(
                            routing_settings).AsDict();
                    route = std::make_unique<TransportRoute>(db_,
                                                             settings.at(tor::bus_wait_time).AsDouble(),
                                                             settings.at(tor::bus_velocity).AsDouble());*/
                }
                const auto short_route = route->GetShortRoute(request.at("from").AsString(), request.at("to").AsString());
                if (!short_route) {
                    req = Builder{}.StartDict().Key(request_id).Value(request.at(id))
                            .Key("error_message").Value("not found").EndDict().Build().AsDict();
                }
                else {
                    json::Array items;
                    items.reserve(short_route->drive_info.size() * 2);
                    for (const auto& edge : short_route->drive_info) {
                        items.emplace_back(json::Builder{}.StartDict().Key("stop_name").Value(std::string{edge.wait_stop})
                                                   .Key("time").Value(short_route->bus_wait_time).Key("type").Value("Wait")
                                                   .EndDict().Build().AsDict());
                        items.emplace_back(json::Builder{}.StartDict().Key("bus").Value(std::string{edge.bus})
                                                   .Key("span_count").Value(edge.stops_count)
                                                   .Key("time").Value(edge.time_driving).Key("type").Value(tor::bus).EndDict().Build().AsDict());
                    }
                    req = json::Builder{}.StartDict().Key("total_time").Value(short_route->total_time)
                            .Key(tor::request_id).Value(request.at(id)).Key("items").Value(items).EndDict().Build().AsDict();

                }
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
        const auto& stat_answer = node.AsDict();
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

void JsonReader::SerializeDatabase(const json::Dict &settings) {
    s_data.SetRenderSettings(settings.at(render_settings).AsDict());
    const auto& r_set = settings.at(tor::routing_settings).AsDict();
    s_data.SetRouteSettings(settings.at(routing_settings).AsDict());
    std::filesystem::path path = settings.at(serialization_settings).AsDict().at(file).AsString();
    s_data.SerializeDataBaseInFile(path);
}

void JsonReader::DeserializeDatabase(const Dict &settings) {
    std::filesystem::path path = settings.at(file).AsString();
    s_data.DeserializeDataBaseFromFile(path);
}
