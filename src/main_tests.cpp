
#include <catch2/catch_test_macros.hpp>

#include "main.hpp"

TEST_CASE("insert_and_contains_fixed") {
    avl_map<int, int> map{};

    REQUIRE(map.find(10) == std::nullopt);

    map.insert(10, 1000);

    REQUIRE(map.find(10) == 1000);

    map.insert(20, 2000);

    REQUIRE(map.find(10) == 1000);
    REQUIRE(map.find(20) == 2000);

    map.insert(30, 3000);

    REQUIRE(map.find(10) == 1000);
    REQUIRE(map.find(20) == 2000);
    REQUIRE(map.find(30) == 3000);

    map.insert(5, 500);

    REQUIRE(map.find(10) == 1000);
    REQUIRE(map.find(20) == 2000);
    REQUIRE(map.find(30) == 3000);
    REQUIRE(map.find(5) == 500);

    map.insert(7, 700);

    REQUIRE(map.find(10) == 1000);
    REQUIRE(map.find(20) == 2000);
    REQUIRE(map.find(30) == 3000);
    REQUIRE(map.find(5) == 500);
    REQUIRE(map.find(7) == 700);
}

