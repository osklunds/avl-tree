
#include <catch2/catch_test_macros.hpp>

#include "main.hpp"

TEST_CASE("insert_and_contains") {
    avl_map<int, int> map{};

    REQUIRE(map.find(1) == std::nullopt);

    map.insert(1, 100);

    REQUIRE(map.find(1) == 100);

    map.insert(2, 200);

    REQUIRE(map.find(1) == 100);
    REQUIRE(map.find(2) == 200);

    
}

