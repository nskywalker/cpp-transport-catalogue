#include <fstream>
#include <iostream>
#include <string_view>
#include "json_reader.h"
#include "map_renderer.h"
#include "request_handler.h"
#include <filesystem>

using namespace std::literals;
using namespace ctg::catalogue;


void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

void Serialize() {
    std::ifstream in("make_base.txt");
    TransportCatalogue db;
    JsonReader reader(db, in, std::cout);
    reader.ReadBaseRequests();
//    exit(0);
}

void MyTest() {
    std::ifstream in("stat_req.txt");
    std::filesystem::path p("output.txt");
    std::ofstream out(p);
    TransportCatalogue db;
    JsonReader reader(db, in, out);
    reader.ReadStatRequests();
    out.close();
    exit(0);
}

int main(int argc, char* argv[]) {
//    Serialize();
//    MyTest();
    if (argc != 2) {
        PrintUsage();
        return 1;
    }
    TransportCatalogue db;
    JsonReader reader(db, std::cin, std::cout);
    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {
        // make base here
        reader.ReadBaseRequests();

    } else if (mode == "process_requests"sv) {
        // process requests here
        reader.ReadStatRequests();

    } else {
        PrintUsage();
        return 1;
    }
}

/*
using namespace ctg::catalogue;

int main() {
    TransportCatalogue db;
    JsonReader reader(db, std::cin, std::cout);
    reader.ProcessingRequest();
    return 0;
}
*/