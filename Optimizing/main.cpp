#include <iostream>
#include <random>
#include <thread>
#include <chrono>
#include <mutex>
#include <vector>

int in{};
int total{};
std::mutex mutex{};

struct vector2f
{
    float x, y;
};

vector2f GetRandomPoint()
{
    static std::random_device rd{};
    static std::default_random_engine eng{ rd() };
    static std::uniform_real_distribution<float> distr{ -1, 1 };
    return { distr(eng), distr(eng) };
}

void CountPoints(int number)
{
    int localIn{};
    for (int i{}; i < number; ++i)
    {
        const vector2f point{ GetRandomPoint() };
        if (point.x * point.x + point.y * point.y < 1.f)
        {
            ++localIn;
        }
    }
    std::lock_guard<std::mutex> lock(mutex);
    in += localIn;
    total += number;
}

int main()
{
    constexpr int points{ 10'000'000 };

    // WITHOUT THREADING //
    auto start{ std::chrono::high_resolution_clock::now() };
    CountPoints(points);
    auto end{ std::chrono::high_resolution_clock::now() };
    std::chrono::duration<double, std::milli> diff{ end - start };
    std::cout << "Time Run: " << diff.count() << "ms\n";


    // WITH THREADING //
    start = std::chrono::high_resolution_clock::now();

    const unsigned int nrThreads{ std::thread::hardware_concurrency() };
    std::vector<std::jthread> threads{ nrThreads };
    const int points_per_thread = points / nrThreads;

    for (auto& thread : threads)
    {
        thread = std::jthread(CountPoints, points_per_thread);
    }

    for (auto& thread : threads)
    {
        thread.join();
    }

    end = std::chrono::high_resolution_clock::now();
    diff = end - start;
    std::cout << "Time Run With Threads: " << diff.count() << "ms\n";

    std::cout << "In / Total: " << 4.f * in / total << "\n";
    return 0;
}
