#include "json_reader.h"
#include <sstream>
#include "map_renderer.h"
#include "request_handler.h"

using namespace ctg::catalogue;

int main() {
    TransportCatalogue db;
    JsonReader reader(db, std::cin);
    reader.ProcessingRequest();
    renderer::MapRenderer rend(reader.GetRenderSettings());
    RequestHandler handler(db, rend, reader.GetAllRequests(), std::cout);
    handler.ProcessingRequests();
    return 0;
}