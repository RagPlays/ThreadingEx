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
    thread_local std::random_device rd{};
    thread_local std::default_random_engine eng{ rd() };
    thread_local std::uniform_real_distribution<float> distr{ -1, 1 };
    return { distr(eng), distr(eng) };
}

void CountPoints(int number)
{
    int localIn{};
    for (int idx{}; idx < number; ++idx)
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

void PrintTime(const std::string& version, int milisec, float result)
{
    std::cout << version << " version: " << milisec << " ms\n";
    std::cout << "result: " << result << "\n\n";
}

int main()
{
    // variables //
    constexpr unsigned int samples{ 100'000'000 };
    const unsigned int nrThreads{ 4 };
    const unsigned int points_per_thread{ samples / nrThreads };

    std::chrono::high_resolution_clock::time_point start{};
    std::chrono::high_resolution_clock::time_point end{};
    std::chrono::duration<double, std::milli> diff{};

    // Normal Version //
    start = std::chrono::high_resolution_clock::now();
    CountPoints(samples);
    end = std::chrono::high_resolution_clock::now();
    diff = end - start;

    PrintTime("Original", static_cast<int>(diff.count()), 4.f * in / total);
    in = 0;
    total = 0;

    // Threaded Version //
    std::vector<std::jthread> threads{ nrThreads };

    start = std::chrono::high_resolution_clock::now();
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
    PrintTime("Threaded", static_cast<int>(diff.count()), 4.f * in / total);
    in = 0;
    total = 0;

    return 0;
}