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
JsonReader::JsonReader(ctg::catalogue::TransportCatalogue &db_, std::istream &in)
        : db(db_), input(in) {

}

void JsonReader::ProcessingRequest() {
    requests = std::make_unique<Document>(Load(input));
    if (!requests->GetRoot().IsMap()) {
        throw ParsingError("zasada! Form of data must be map");
    }
}


const Dict & JsonReader::GetRenderSettings() const {
    return requests->GetRoot().AsMap().at(render_settings).AsMap();
}

const std::map<std::string, json::Node> &JsonReader::GetAllRequests() const {
    if (!requests) {
        throw json::ParsingError("request is empty");
    }
    return requests->GetRoot().AsMap();
}
