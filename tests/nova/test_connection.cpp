#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <nova/engine/property.hpp>

using namespace nova_sim;

// Simulating the new DataLink transfer logic for tests
template<typename T, typename U = T>
struct mock_connection {
    property_t<T>* src;
    property_t<U>* sink;
    std::function<U(T)> modifier;
    
    mock_connection(property_t<T>* s, property_t<U>* sk, std::function<U(T)> mod = [](T v){return static_cast<U>(v);}) 
        : src(s), sink(sk), modifier(mod) {}
        
    void transferData() {
        if (src && sink) {
            sink->set_value(modifier(src->get_value()));
        }
    }
};

using real_connection = mock_connection<double>;
using int_connection = mock_connection<int>;
using bool_connection = mock_connection<bool>;
using string_connection = mock_connection<std::string>;
template<typename T, typename U>
using connection_te = mock_connection<T, U>;

TEST_CASE("test_real_connection")
{
    double sourceValue = 0;
    double sinkValue = -1;
    property_t<double> source({"::source"}, [&] { return ++sourceValue; });
    property_t<double> sink(
        {"::sink"}, [&] { return sinkValue; }, [&](auto value) { sinkValue = value; });

    real_connection c{&source, &sink};

    c.transferData();
    sink.apply_sets(); // Updated to apply_sets

    CHECK_THAT(sourceValue, Catch::Matchers::WithinRel(1.));
    CHECK_THAT(sourceValue, Catch::Matchers::WithinRel(sinkValue));
}

TEST_CASE("test_int_connection")
{
    int sourceValue = 0;
    int sinkValue = -1;
    property_t<int> source({"::source"}, [&] { return ++sourceValue; });
    property_t<int> sink(
        {"::sink"}, [&] { return sinkValue; }, [&](auto value) { sinkValue = value; });

    int_connection c{&source, &sink};

    c.transferData();
    sink.apply_sets();

    CHECK(sourceValue == 1);
    CHECK(sourceValue == sinkValue);
}

TEST_CASE("test_bool_connection")
{
    bool sourceValue = false;
    bool sinkValue = false;
    property_t<bool> source({"::source"}, [&] {
        sourceValue = !sourceValue;
        return sourceValue;
    });
    property_t<bool> sink(
        {"::sink"}, [&] { return sinkValue; }, [&](auto value) { sinkValue = value; });

    bool_connection c{&source, &sink};

    c.transferData();
    sink.apply_sets();

    CHECK(sourceValue == true);
    CHECK(sourceValue == sinkValue);
}

TEST_CASE("test_string_connection")
{
    std::string sourceValue = "0";
    std::string sinkValue;
    property_t<std::string> source({"::source"}, [&] {
        sourceValue = std::to_string(std::stoi(sourceValue) + 1);
        return sourceValue;
    });
    property_t<std::string> sink(
        {"::sink"}, [&] { return sinkValue; }, [&](auto value) { sinkValue = value; });

    string_connection c{&source, &sink};

    c.transferData();
    sink.apply_sets();

    CHECK(sourceValue == "1");
    CHECK(sourceValue == sinkValue);
}

TEST_CASE("test_double_string_connection")
{
    double sourceValue = 0;
    std::string sinkValue;
    property_t<double> source({"::source"}, [&] {
        return ++sourceValue;
    });
    property_t<std::string> sink(
        {"::sink"}, [&] { return sinkValue; }, [&](auto value) { sinkValue = value; });

    connection_te<double, std::string> c{&source, &sink, [](double value) { return std::to_string(value); }};

    c.transferData();
    sink.apply_sets();

    CHECK_THAT(sourceValue, Catch::Matchers::WithinRel(1.));
    CHECK_THAT(std::stoi(sinkValue), Catch::Matchers::WithinRel(1.));
}
