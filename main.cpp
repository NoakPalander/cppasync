#define FMT_HEADER_ONLY
#include <fmt/format.h> // fmt::print

#include <random>       // std::random_device, std::default_random_engine, std::uniform_x_distribution
#include <vector>       // std::vector
#include <type_traits>  // std::is_same_v
#include <algorithm>    // std::generate_n
#include <thread>       // std::thread, std::this_thread, std::launch
#include <chrono>       // std::chrono_literals, steadyclock, duration_cast
#include <future>       // std::future
#include <iostream>

// Returns a vector filled with random values determained by the generator
template<typename T>
std::vector<T> GenRandom(std::function<T(void)> const& generator, std::size_t quantity, std::size_t id) {
    using namespace std::chrono_literals;
    
    std::vector<T> results{};
    std::generate_n(std::back_inserter(results), quantity, [&generator, &id, &quantity, index = 1]() mutable { 
        fmt::print("[ID: {}] Polling random number {}/{}!\n", id, index++, quantity);
        std::cout.flush();

        std::this_thread::sleep_for(500ms);
        return generator();
    });

    return results;
}

/* Returns a random-generator suitable for the template type */
template<typename T>
std::function<T(void)> Generator(T min, T max) {
    static std::random_device rd{};
    static std::default_random_engine eng{rd()};
    
    if constexpr (std::is_same_v<T, int>) {
        static std::uniform_int_distribution<int> dist(min, max);
        return [&]() -> int { return dist(eng); };
    }
    else {
        static std::uniform_real_distribution<T> dist(min, max);
        return [&]() -> T { return dist(eng); };
    }    
}

int main(int argc, char** argv) {
    // If the launch contains "async" as flag
    // Run async
    if (argc > 1 && std::strcmp(argv[1], "async") == 0) {
        // Starts the clock
        auto start = std::chrono::steady_clock::now();
        
        // Launch both generators at the same time
        std::future<std::vector<int>>   f1 = std::async(std::launch::async, []{ return GenRandom<int>(Generator<int>(0, 10), 5, 1); });
        std::future<std::vector<float>> f2 = std::async(std::launch::async, []{ return GenRandom<float>(Generator<float>(0.5f, 1.5f), 5, 2); });

        auto const& v1 = f1.get(); // Unblock and getting the data
        auto const& v2 = f2.get(); // Unblock and getting the data

        // Stops the clock
        auto end = std::chrono::steady_clock::now();
        fmt::print("Duration: {}ms\n\n", std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());

        for (int const& i : v1)
            fmt::print("{} ", i);

        fmt::print("\n");
        for (float const& i : v2)
            fmt::print("{} ", i);
    }
    // Run in sequence
    else {
        // Starts the clock
        auto start = std::chrono::steady_clock::now();

        // Launches the generators in sequence
        auto v1 = GenRandom<int>(Generator<int>(0, 10), 5, 1);
        auto v2 = GenRandom<float>(Generator<float>(0.5f, 1.5f), 5, 1);

        // Stops the clock
        auto end = std::chrono::steady_clock::now();
        fmt::print("Duration: {}ms\n\n", std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());

        for (int const& i : v1)
            fmt::print("{} ", i);

        fmt::print("\n");
        for (float const& i : v2)
            fmt::print("{} ", i);
    }
}
