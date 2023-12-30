#pragma once

#include <filesystem>
#include <transport_catalogue.pb.h>
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

namespace serialization {
    using DataBase = ctg::catalogue::TransportCatalogue;
    using Path = std::filesystem::path;
    class SerializationData final {
        DataBase& db;
        proto_catalogue::DataBase proto_db;
        renderer::MapRenderer* renderer = nullptr;
        const json::Dict* render_settings = nullptr;
        const json::Dict* route_settings = nullptr;
        TransportRoute* router = nullptr;
        json::Dict settings;
        std::unordered_map<std::string_view, uint64_t> stopname_to_id;
        std::unordered_map<uint64_t, std::string_view> id_to_stopname;
    protected:
        void SerializeCatalogue();
        void SerializeRenderSettings();
        void SerializeTransportRoute();
        void DeserializeCatalogue();
        void DeserializeRenderSettings();
        void DeserializeTransportRoute();
        static proto_catalogue::ColorRGB GetProtobufColorRGB(const json::Node& node);
        static proto_catalogue::ColorRGBA GetProtobufColorRGBA(const json::Node& node);
        static json::Array GetJsonColorRGB(const proto_catalogue::ColorRGB& c_rgb);
        static json::Array GetJsonColorRGBA(const proto_catalogue::ColorRGBA &c_rgba);
    public:
        explicit SerializationData(DataBase &db_);
        void SerializeDataBaseInFile(const serialization::Path &path);
        void DeserializeDataBaseFromFile(const Path &path);
        void SetRenderSettings(const json::Dict& render_settings_);
        renderer::MapRenderer* GetMapRenderer();
        void SetRouteSettings(const json::Dict& route_settings_);

        TransportRoute * GetTransportRoute();
    };
}