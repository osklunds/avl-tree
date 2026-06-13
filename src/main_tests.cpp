
#include <catch2/catch_test_macros.hpp>

#include "main.hpp"

#include <map>
#include <random>

#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

// Copied from https://stackoverflow.com/a/77336
// Used for showing a stacktrace when segfault
// Call as in a test case ---> signal(SIGSEGV, handler);
void handler(int sig) {
  void *array[10];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(array, 10);

  // print out all the frames to stderr
  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, STDERR_FILENO);
  exit(1);
}


TEST_CASE("insert_and_contains_fixed") {
    avl_map<int, int> map{};

    REQUIRE(map.find(10) == std::nullopt);

    map.insert(10, 1000);

    REQUIRE(map.find(10) == 1000);
    REQUIRE(map.find(20) == std::nullopt);
    REQUIRE(map.find(30) == std::nullopt);
    REQUIRE(map.find(5) == std::nullopt);
    REQUIRE(map.find(7) == std::nullopt);

    map.insert(20, 2000);

    REQUIRE(map.find(10) == 1000);
    REQUIRE(map.find(20) == 2000);
    REQUIRE(map.find(30) == std::nullopt);
    REQUIRE(map.find(5) == std::nullopt);
    REQUIRE(map.find(7) == std::nullopt);

    map.insert(30, 3000);

    REQUIRE(map.find(10) == 1000);
    REQUIRE(map.find(20) == 2000);
    REQUIRE(map.find(30) == 3000);
    REQUIRE(map.find(5) == std::nullopt);
    REQUIRE(map.find(7) == std::nullopt);

    map.insert(5, 500);

    REQUIRE(map.find(10) == 1000);
    REQUIRE(map.find(20) == 2000);
    REQUIRE(map.find(30) == 3000);
    REQUIRE(map.find(5) == 500);
    REQUIRE(map.find(7) == std::nullopt);

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

TEST_CASE("insert_random") {
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

TEST_CASE("insert_left_left") {
    avl_map<int, int> map{};

    map.insert(10, 1000);
    map.insert(9, 900);
    map.insert(8, 800);

    REQUIRE(map.find(10) == 1000);
    REQUIRE(map.find(9) == 900);
    REQUIRE(map.find(8) == 800);
}

TEST_CASE("insert_left_right") {
    avl_map<int, int> map{};

    map.insert(10, 1000);
    map.insert(8, 800);
    map.insert(9, 900);

    REQUIRE(map.find(10) == 1000);
    REQUIRE(map.find(8) == 800);
    REQUIRE(map.find(9) == 900);
}

TEST_CASE("insert_right_right") {
    avl_map<int, int> map{};

    map.insert(10, 1000);
    map.insert(11, 1100);
    map.insert(12, 1200);

    REQUIRE(map.find(10) == 1000);
    REQUIRE(map.find(11) == 1100);
    REQUIRE(map.find(12) == 1200);
}

TEST_CASE("insert_right_left") {
    avl_map<int, int> map{};

    map.insert(10, 1000);
    map.insert(12, 1200);
    map.insert(11, 1100);

    REQUIRE(map.find(10) == 1000);
    REQUIRE(map.find(11) == 1100);
    REQUIRE(map.find(12) == 1200);
}

TEST_CASE("remove_no_children") {
    avl_map<int, int> map{};

    map.insert(11, 1100);
    map.insert(10, 1000);
    map.insert(12, 1200);

    REQUIRE(map.find(10) == 1000);
    REQUIRE(map.find(11) == 1100);
    REQUIRE(map.find(12) == 1200);

    map.remove(10);

    REQUIRE(map.find(10) == std::nullopt);
    REQUIRE(map.find(11) == 1100);
    REQUIRE(map.find(12) == 1200);
}

TEST_CASE("remove_left_child") {
    avl_map<int, int> map{};

    map.insert(11, 1100);
    map.insert(10, 1000);
    map.insert(12, 1200);
    map.insert(9, 900);

    //     11
    //   10  12
    // 9 

    REQUIRE(map.find(9) == 900);
    REQUIRE(map.find(10) == 1000);
    REQUIRE(map.find(11) == 1100);
    REQUIRE(map.find(12) == 1200);

    map.remove(10);

    REQUIRE(map.find(9) == 900);
    REQUIRE(map.find(10) == std::nullopt);
    REQUIRE(map.find(11) == 1100);
    REQUIRE(map.find(12) == 1200);
}

TEST_CASE("remove_right_child") {
    avl_map<int, int> map{};

    map.insert(11, 1100);
    map.insert(10, 1000);
    map.insert(12, 1200);
    map.insert(13, 1300);

    //     11
    //   10  12
    //         13 

    REQUIRE(map.find(10) == 1000);
    REQUIRE(map.find(11) == 1100);
    REQUIRE(map.find(12) == 1200);
    REQUIRE(map.find(13) == 1300);

    map.remove(12);

    REQUIRE(map.find(10) == 1000);
    REQUIRE(map.find(11) == 1100);
    REQUIRE(map.find(12) == std::nullopt);
    REQUIRE(map.find(13) == 1300);
}

TEST_CASE("remove_two_children") {
    avl_map<int, int> map{};

    map.insert(11, 1100);
    map.insert(10, 1000);
    map.insert(12, 1200);

    //     11
    //   10  12

    REQUIRE(map.find(10) == 1000);
    REQUIRE(map.find(11) == 1100);
    REQUIRE(map.find(12) == 1200);

    map.remove(11);

    REQUIRE(map.find(10) == 1000);
    REQUIRE(map.find(11) == std::nullopt);
    REQUIRE(map.find(12) == 1200);
}

TEST_CASE("remove_rotate") {
    avl_map<int, int> map{};

    map.insert(11, 1100);
    map.insert(10, 1000);
    map.insert(12, 1200);
    map.insert(9, 900);

    //      11
    //    10  12
    //  9

    REQUIRE(map.find(9) == 900);
    REQUIRE(map.find(10) == 1000);
    REQUIRE(map.find(11) == 1100);
    REQUIRE(map.find(12) == 1200);

    map.remove(12);

    REQUIRE(map.find(9) == 900);
    REQUIRE(map.find(10) == 1000);
    REQUIRE(map.find(11) == 1100);
    REQUIRE(map.find(12) == std::nullopt);
}

TEST_CASE("remove_edge_cases") {
    avl_map<int, int> map{};

    map.insert(11, 1100);
    REQUIRE(map.find(11) == 1100);
    REQUIRE(map.find(12) == std::nullopt);

    // Remove not found
    map.remove(12);

    REQUIRE(map.find(11) == 1100);
    REQUIRE(map.find(12) == std::nullopt);

    // Remove last
    map.remove(11);

    REQUIRE(map.find(11) == std::nullopt);
    REQUIRE(map.find(12) == std::nullopt);

    // Remove when empty
    map.remove(13);

    REQUIRE(map.find(11) == std::nullopt);
    REQUIRE(map.find(12) == std::nullopt);
    REQUIRE(map.find(13) == std::nullopt);
}

TEST_CASE("remove_random") {
    signal(SIGSEGV, handler);

    avl_map<int, int> avl_map{};
    std::map<int, int> map{};

    std::random_device dev;
    std::mt19937 rng(dev());
    const int max = 1000;
    std::uniform_int_distribution<std::mt19937::result_type> dist(1,max);

    for (int i = 0; i < 1000; i++) {
        int key = dist(rng);
        int value = dist(rng);
        bool insert = dist(rng) % 2;

        if (insert) {
            avl_map.insert(key, value);
            map.erase(key);
            map.insert({key, value});
        } else {
            avl_map.remove(key);
            map.erase(key);
        }

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

TEST_CASE("min_max_fixed") {
    avl_map<int, int> map{};

    REQUIRE(map.get_min() == std::nullopt);
    REQUIRE(map.get_max() == std::nullopt);

    map.insert(10, 100);

    REQUIRE(map.get_min() == std::make_tuple(10,100));
    REQUIRE(map.get_max() == std::make_tuple(10,100));

    map.insert(11, 110);

    REQUIRE(map.get_min() == std::make_tuple(10,100));
    REQUIRE(map.get_max() == std::make_tuple(11,110));

    map.insert(12, 120);

    REQUIRE(map.get_min() == std::make_tuple(10,100));
    REQUIRE(map.get_max() == std::make_tuple(12,120));

    map.insert(9, 90);

    REQUIRE(map.get_min() == std::make_tuple(9,90));
    REQUIRE(map.get_max() == std::make_tuple(12,120));
}

TEST_CASE("min_max_random") {
    signal(SIGSEGV, handler);

    avl_map<int, int> avl_map{};
    std::map<int, int> map{};

    std::random_device dev;
    std::mt19937 rng(dev());
    const int max = 1000;
    std::uniform_int_distribution<std::mt19937::result_type> dist(1,max);

    for (int i = 0; i < 1000; i++) {
        int key = dist(rng);
        int value = dist(rng);
        bool insert = dist(rng) % 2;

        if (insert) {
            avl_map.insert(key, value);
            map.erase(key);
            map.insert({key, value});
        } else {
            avl_map.remove(key);
            map.erase(key);
        }

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

        if (map.size() == 0) {
            REQUIRE(avl_map.get_min() == std::nullopt);
            REQUIRE(avl_map.get_max() == std::nullopt);
        } else if (map.size() == 1) {
            auto elem = map.begin();
            auto elem2 = std::make_tuple(elem->first, elem->second);
            REQUIRE(avl_map.get_min() == elem2);
            REQUIRE(avl_map.get_max() == elem2);
        } else {
            auto min = map.begin();
            auto min2 = std::make_tuple(min->first, min->second);
            REQUIRE(avl_map.get_min() == min2);

            auto max = map.end();
            max--;
            auto max2 = std::make_tuple(max->first, max->second);
            REQUIRE(avl_map.get_max() == max2);
        }
    }
}

TEST_CASE("take_min_max_fixed") {
    avl_map<int, int> map{};

    map.insert(9, 90);
    map.insert(11, 110);
    map.insert(10, 100);
    map.insert(12, 120);

    REQUIRE(map.take_min() == std::make_tuple(9,90));
    REQUIRE(map.take_min() == std::make_tuple(10,100));
    REQUIRE(map.take_max() == std::make_tuple(12,120));

    REQUIRE(map.find(9) == std::nullopt);
    REQUIRE(map.find(10) == std::nullopt);
    REQUIRE(map.find(11) == 110);
    REQUIRE(map.find(12) == std::nullopt);

    REQUIRE(map.take_max() == std::make_tuple(11,110));

    REQUIRE(map.take_max() == std::nullopt);
    REQUIRE(map.take_min() == std::nullopt);

}
