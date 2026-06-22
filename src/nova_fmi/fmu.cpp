#include "fmu.hpp"
#include "fmi1/fmi1_fmu.hpp"
#include "fmi2/fmi2_fmu.hpp"
#include "fmi3/fmi3_fmu.hpp"
#include "util/temp_dir.hpp"
#include "util/unzipper.hpp"
#include "nova/components/logger/logger.hpp"
#include "nova_fmi_library.hpp"

#include "nova/components/util/nova_xml.hpp"
#include <iostream>

namespace nova_fmi {

std::unique_ptr<fmu> loadFmu(const std::filesystem::path& fmuPath, bool fmiLogging)
{
    if (!std::filesystem::exists(fmuPath)) {
        std::cerr << "[loadFmu] Path does not exist: " << fmuPath << std::endl;
        return nullptr;
    }
    auto absFmuPath = std::filesystem::absolute(fmuPath);

    const std::string fmuName = absFmuPath.stem().string();
    auto temp = std::make_unique<nova_sim::temp_dir>(fmuName);

    if (!nova_sim::unzip(absFmuPath, temp->path())) {
        std::cerr << "[loadFmu] Unzip failed" << std::endl;
        return nullptr;
    }

    nova_sim::xml::XmlDocument doc;
    if (!doc.load_file((temp->path() / "modelDescription.xml").string().c_str())) {
        std::cerr << "[loadFmu] Failed to load XML" << std::endl;
        return nullptr;
    }

    auto root = doc.child("fmiModelDescription");
    std::string version = root.attribute("fmiVersion").as_string();
    std::string guid = root.attribute("guid").as_string();
    if (guid.empty()) guid = root.attribute("instantiationToken").as_string();
    
    std::string modelIdentifier;
    if (version.find("1.0") == 0) {
        modelIdentifier = root.child("Implementation").child("CoSimulation_StandAlone").attribute("modelIdentifier").as_string();
    } else {
        modelIdentifier = root.child("CoSimulation").attribute("modelIdentifier").as_string();
    }
    if (modelIdentifier.empty()) modelIdentifier = root.attribute("modelIdentifier").as_string();

    if (modelIdentifier.empty()) {
        std::cerr << "[loadFmu] modelIdentifier is empty" << std::endl;
        return nullptr;
    }

    model_variables variables;
    auto vars_node = root.child("ModelVariables");
    
    if (version.find("3.0") == 0) {
        // FMI 3.0 Variables
        for (auto var : vars_node.children()) {
            std::string nodeName = var.name();
            if (nodeName == "Float64" || nodeName == "Float32" || nodeName == "Int32" || nodeName == "Int64" || 
                nodeName == "Boolean" || nodeName == "String" || nodeName == "Binary" || nodeName == "Clock") {
                scalar_variable sv;
                sv.name = var.attribute("name").as_string();
                sv.vr = var.attribute("valueReference").as_uint();
                sv.causality = var.attribute("causality").as_string();
                sv.variability = var.attribute("variability").as_string();
                sv.initial = var.attribute("initial").as_string();
                sv.description = var.attribute("description").as_string();

                if (nodeName.find("Float") != std::string::npos) {
                    real_attributes attrs;
                    if (var.attribute("start")) attrs.start = var.attribute("start").as_double();
                    sv.typeAttributes = attrs;
                } else if (nodeName.find("Int") != std::string::npos || nodeName == "UInt64") {
                    integer_attributes attrs;
                    if (var.attribute("start")) attrs.start = var.attribute("start").as_int();
                    sv.typeAttributes = attrs;
                } else if (nodeName == "Boolean") {
                    boolean_attributes attrs;
                    if (var.attribute("start")) attrs.start = var.attribute("start").as_bool();
                    sv.typeAttributes = attrs;
                } else if (nodeName == "String") {
                    string_attributes attrs;
                    if (var.attribute("start")) attrs.start = var.attribute("start").as_string();
                    sv.typeAttributes = attrs;
                }
                variables.push_back(std::move(sv));
            }
        }
    } else {
        // FMI 1.0 and 2.0 Variables
        for (auto var : vars_node.children("ScalarVariable")) {
            scalar_variable sv;
            sv.name = var.attribute("name").as_string();
            sv.vr = var.attribute("valueReference").as_uint();
            sv.causality = var.attribute("causality").as_string();
            sv.variability = var.attribute("variability").as_string();
            sv.initial = var.attribute("initial").as_string();
            sv.description = var.attribute("description").as_string();

            if (auto type = var.child("Real")) {
                real_attributes attrs;
                if (type.attribute("start")) attrs.start = type.attribute("start").as_double();
                sv.typeAttributes = attrs;
            } else if (auto type = var.child("Integer")) {
                integer_attributes attrs;
                if (type.attribute("start")) attrs.start = type.attribute("start").as_int();
                sv.typeAttributes = attrs;
            } else if (auto type = var.child("Boolean")) {
                boolean_attributes attrs;
                if (type.attribute("start")) attrs.start = type.attribute("start").as_bool();
                sv.typeAttributes = attrs;
            } else if (auto type = var.child("String")) {
                string_attributes attrs;
                if (type.attribute("start")) attrs.start = type.attribute("start").as_string();
                sv.typeAttributes = attrs;
            }
            variables.push_back(std::move(sv));
        }
    }

    std::string libPath;
    auto bins = temp->path() / "binaries";
#ifdef _WIN32
  #if defined(_WIN64)
    std::vector<std::string> platforms = {"win64", "x86_64-windows"};
  #else
    std::vector<std::string> platforms = {"win32", "x86-windows"};
  #endif
    std::string ext = ".dll";
#else
  #if defined(__x86_64__) || defined(__aarch64__)
    std::vector<std::string> platforms = {"linux64", "x86_64-linux"};
  #else
    std::vector<std::string> platforms = {"linux32", "x86-linux"};
  #endif
    std::string ext = ".so";
#endif

    if (std::filesystem::exists(bins)) {
        for (const auto& plat : platforms) {
            auto platDir = bins / plat;
            if (std::filesystem::exists(platDir)) {
                for (const auto& entry : std::filesystem::directory_iterator(platDir)) {
                    if (entry.is_regular_file() && entry.path().extension().string() == ext) {
                        libPath = entry.path().string();
                        if (entry.path().filename().string().find(modelIdentifier) != std::string::npos) {
                            break;
                        }
                    }
                }
            }
            if (!libPath.empty()) break;
        }
    }

    if (libPath.empty()) {
        std::cerr << "[loadFmu] DLL not found for architecture." << std::endl;
        return nullptr;
    }

    std::shared_ptr<NovaFmiLibrary> lib;
    try { 
        lib = std::make_shared<NovaFmiLibrary>(libPath); 
    } catch (const std::exception& e) { 
        std::cerr << "[loadFmu] Library load failed: " << e.what() << std::endl;
        return nullptr; 
    }
    
    model_description md;
    md.guid = guid;
    md.modelName = root.attribute("modelName").as_string();
    md.description = root.attribute("description").as_string();
    md.modelIdentifier = modelIdentifier;
    md.modelVariables = std::move(variables);
    md.canGetAndSetState = true;

    if (version.find("2.0") == 0) return std::make_unique<fmi2_fmu>(lib, std::move(temp), std::move(md), fmiLogging);
    if (version.find("1.0") == 0) return std::make_unique<fmi1_fmu>(lib, std::move(temp), std::move(md), fmiLogging);
    if (version.find("3.0") == 0) return std::make_unique<fmi3_fmu>(lib, std::move(temp), std::move(md), fmiLogging);
    
    std::cerr << "[loadFmu] Unknown version: " << version << std::endl;
    return nullptr; 
}

} // namespace nova_fmi
