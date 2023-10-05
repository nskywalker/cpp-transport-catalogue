#include "json_reader.h"
#include <sstream>
#include "map_renderer.h"
#include "request_handler.h"

using namespace ctg::catalogue;

int main() {
    TransportCatalogue db;
    JsonReader reader(db, std::cin, std::cout);
    reader.ProcessingRequest();
    return 0;
}