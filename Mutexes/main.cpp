#include <iostream>
#include <thread>
#include <mutex> //mutual exclusion
#include <vector>
#include <chrono>

class Wallet final
{
public:

    Wallet() = default;
    ~Wallet() = default;

    float getMoney()
    {
        return m_EuroCents / 100.0f;
    }

    void addMoney(int euros)
    {
        const std::lock_guard<std::mutex> addmoneyLock{ m_myMutex };

        for (int i{}; i < euros; ++i)
        {
            addEuro();
        }
    }

    void moveMoney(int euros, Wallet& to)
    {
        std::scoped_lock lock(m_myMutex, to.m_myMutex);

        for (int i{}; i < euros; ++i)
        {
            to.removeEuro();
            addEuro();
        }
    }

private:

    void addEuro()
    {
        m_EuroCents = m_EuroCents + 100;
    }

    void removeEuro()
    {
        m_EuroCents = m_EuroCents - 100;
    }

private:

    int m_EuroCents{};
    std::mutex m_myMutex{};

};

int testMultiThreadedWallet()
{
    constexpr int times{ 5 };
    Wallet walletA{};
    Wallet walletB{};
    std::vector<std::jthread> threads;

    // Add money
    for (int idx{}; idx < times; ++idx)
    {
        threads.emplace_back(std::jthread(&Wallet::addMoney, &walletA, 2500 / times));
        threads.emplace_back(std::jthread(&Wallet::addMoney, &walletB, 2500 / times));
    }

    // Wait for all threads
    for (auto& thread : threads)
    {
        thread.join();
    }
    // clear all threads to set new ones
    threads.clear();

    // Move money
    for (int idx{}; idx < times; ++idx)
    {
        threads.emplace_back(std::jthread(&Wallet::moveMoney, &walletA, 1500 / times, std::ref(walletB)));
        threads.emplace_back(std::jthread(&Wallet::moveMoney, &walletB, 1000 / times, std::ref(walletA)));
    }

    // Wait for all threads
    for (auto& thread : threads)
    {
        thread.join();
    }

    return static_cast<int>(walletA.getMoney() + walletB.getMoney());
}

int main()
{
    constexpr int sampleSize{ 1000 };


    auto start{ std::chrono::high_resolution_clock::now() };

    for (int i{}; i < sampleSize; ++i)
    {
        if (int value{ testMultiThreadedWallet() }; value != 5000)
        {
            std::cout
                << "//ERROR// count: "
                << i
                << ", money: "
                << value << "\n";
        }
    }

    auto end{ std::chrono::high_resolution_clock::now() };
    std::chrono::duration<double> diff{ end - start };

    std::cout << "Programm finished in " << diff.count() << " Sec.\n";

    return 0;
}