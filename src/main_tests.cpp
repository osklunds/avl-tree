
#include <catch2/catch_test_macros.hpp>

#include "main.hpp"

#include <map>
#include <random>

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

    map.insert(7, 701);

    REQUIRE(map.find(10) == 1000);
    REQUIRE(map.find(20) == 2000);
    REQUIRE(map.find(30) == 3000);
    REQUIRE(map.find(5) == 500);
    REQUIRE(map.find(7) == 701);
}

TEST_CASE("insert_and_contains_random") {
    avl_map<int, int> avl_map{};
    std::map<int, int> map{};

    std::random_device dev;
    std::mt19937 rng(dev());
    const int max = 1000;
    std::uniform_int_distribution<std::mt19937::result_type> dist(1,max);

    for (int i = 0; i < 1000; i++) {
        int key = dist(rng);
        int value = dist(rng);

        avl_map.insert(key, value);
        map.erase(key);
        map.insert({key, value});

        for (int j = 0; j < max; j++) {
            auto exp_value = map.find(j);
            auto value = avl_map.find(j);
            if (exp_value == map.end()) {
                REQUIRE(value == std::nullopt);
            } else {
                int v = std::get<1>(*exp_value);
                REQUIRE(value == std::optional<int>(v));
            }
        }
    }
}
