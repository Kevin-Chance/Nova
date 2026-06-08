#include "boot_service_handler.hpp"
#include "client_handler.hpp"
#include "util/uuid.hpp"

#include "ecos/lib_info.hpp"

#include "simple_socket/TCPSocket.hpp"
#include "simple_socket/UnixDomainSocket.hpp"
#include "simple_socket/util/byte_conversion.hpp"
#include "simple_socket/util/port_query.hpp"
#include <flatbuffers/flexbuffers.h>
#include <ecos/logger/logger.hpp>

#include <cli11/CLI11.h>
#include <iostream>
#include <random>
#include <utility>
#include <filesystem>

using namespace nova_sim;
using namespace nova_sim::proxy;

namespace fs = std::filesystem;

namespace
{

const int port_range_min = 7000;
const int port_range_max = 9999;

const int SUCCESS = 0;
const int COMMANDLINE_ERROR = 1;
const int UNHANDLED_ERROR = 2;


void wait_for_input()
{
    std::cout << '\n'
              << "Press any key to quit...\n";
    // clang-format off
    while (std::cin.get() != '\n');
    //clang-format on
    std::cout << "Done." << std::endl;
}

int run_application(const std::string& fmu, const std::string& instanceName, bool local)
{

    if (!local) {
        const auto port = simple_socket::getAvailablePort(port_range_min, port_range_max);
        if (!port) {
            nova_sim::log::err("Unable to locate free port number..");
            return UNHANDLED_ERROR;
        }

        try {
            simple_socket::TCPServer server(*port);
            nova_sim::log::info("Serving proxy '{}' on port {}", instanceName, *port);
            // communication with parent process
            std::cout << "[proxyfmu] bind=" << std::to_string(*port) << std::endl;

            auto con = server.accept();
            nova_sim::log::info("TCP Client connected");
            client_handler(std::move(con), fmu, instanceName);
        } catch (const std::exception& ex) {
            nova_sim::log::err("[run_application] Exception occurred: {}", ex.what());
            return UNHANDLED_ERROR;
        }
    } else {

        auto fileHandle = instanceName + "_" + generate_uuid();
#ifndef _WIN32
        fileHandle.insert(0, "/tmp/");
#endif

        try {
            simple_socket::UnixDomainServer server(fileHandle);
            nova_sim::log::info("Serving proxy '{}' using file '{}'", instanceName, fileHandle);

            // communication with parent process
            std::cout << "[proxyfmu] bind=" << fileHandle << std::endl;

            auto con = server.accept();
            nova_sim::log::info("Unix Domain Client connected");
            client_handler(std::move(con), fmu, instanceName);
        } catch (const std::exception& ex) {
            nova_sim::log::err("[run_application] Exception occurred: {}", ex.what());
            return UNHANDLED_ERROR;
        }
    }

    return SUCCESS;
}

int run_boot_application(const int port)
{

    nova_sim::log::info("Boot application serving on port {}", port);

    boot_service_handler handler;
    simple_socket::TCPServer server(port);

    std::thread server_thread([&] {
        try {
            while (true) {
                const auto conn = server.accept();

                try {
                    std::vector<uint8_t> buffer(4);
                    if (!conn->readExact(buffer)) {
                        log::err("Error reading size");
                        break;
                    }

                    const auto msgSize = simple_socket::decode_uint32(buffer);
                    buffer.resize(msgSize);
                    if (!conn->readExact(buffer)) {
                        log::err("Error reading payload");
                        break;
                    }

                    const auto root = flexbuffers::GetRoot(buffer.data(), msgSize).AsVector();

                    const std::string fmuName = root[0].AsString().str();
                    const std::string instanceName = root[1].AsString().str();
                    const auto blobRef = root[2].AsBlob();
                    const std::vector<uint8_t> data = std::vector(blobRef.data(), blobRef.data() + blobRef.size());

                    nova_sim::log::info("Booting: {}::{}, file size={}", fmuName, instanceName, data.size());

                    const int16_t instance_port = handler.loadFromBinaryData(fmuName, instanceName, data);
                    flexbuffers::Builder fbb;
                    fbb.UInt(instance_port);
                    fbb.Finish();
                    conn->write(fbb.GetBuffer());

                    } catch (const std::exception& ex) {
                        nova_sim::log::err("Exception occurred: {}", ex.what());
                    }
                }
            } catch (const std::exception&) {}

    });

    wait_for_input();

    server.close();
    server_thread.join();

    return SUCCESS;
}

int printHelp(const CLI::App& desc)
{
    std::cout << desc.help() << std::endl;
    return SUCCESS;
}

std::string versionString()
{
    const auto v = nova_sim::library_version();
    std::stringstream ss;
    ss << "v" << v.major << "." << v.minor << "." << v.patch;
    return ss.str();
}

} // namespace

int main(int argc, char** argv)
{

    CLI::App app{"proxyfmu"};

    app.set_version_flag("-v,--version", versionString());
    auto fmu_opt = app.add_option("--fmu", "Location of the fmu to load.");
    auto name_opt = app.add_option("--instanceName", "Name of the slave instance.");
    auto local_opt = app.add_option("--local", "Running locally?");

    CLI::App* sub = app.add_subcommand("boot");
    sub->add_option("--port", "Specify the network port to be used.")->required();

    if (argc == 1) {
        return printHelp(app);
    }

    try {

        CLI11_PARSE(app, argc, argv);

        if (*sub) {

            nova_sim::log::set_logging_level(nova_sim::log::level::debug);

            const auto port = sub->get_option("--port")->as<int>();
            const auto status = run_boot_application(port);
            return status;

        }

        const auto local = local_opt->as<bool>();
        const auto instanceName = name_opt->as<std::string>();

        // NovaLogger doesn't support file logging yet, we'll skip creating the file logger
        // and just use the console logger with debug level.
        nova_sim::log::set_logging_level(nova_sim::log::level::debug);

        const auto fmu = fmu_opt->as<std::string>();
        const auto fmuPath = fs::path(fmu);
        if (!fs::exists(fmuPath)) {
            nova_sim::log::err("No such file: '{}'", fs::absolute(fmuPath).string());
            return COMMANDLINE_ERROR;
        }

        nova_sim::log::info("Got commandline arguments: --fmu '{}', --instanceName '{}', --local {}", fmu, instanceName, local);

        const auto status = run_application(fmu, instanceName, local);

        return status;

    } catch (const std::exception& e) {
        std::cerr << "Unhandled Exception reached the top of main: " << e.what() << std::endl;
        return UNHANDLED_ERROR;
    }
}
