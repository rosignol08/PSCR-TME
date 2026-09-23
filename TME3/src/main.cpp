#include <iostream>
#include <fstream>
#include <regex>
#include <chrono>
#include <string>
#include <algorithm>
#include <vector>
#include <unordered_map>
#include <ios>
#include <thread>
#include <mutex>
#include <atomic>
#include "HashMap.h"
#include "FileUtils.h"

using namespace std;

int main(int argc, char **argv)
{
    using namespace std::chrono;

    // Allow filename as optional first argument, default to project-root/WarAndPeace.txt
    // Optional second argument is mode (e.g. "freqstd" or "freq").
    // Optional third argument is num_threads (default 4).
    string filename = "../WarAndPeace.txt";
    string mode = "freqstdf";
    int num_threads = 4;
    if (argc > 1)
        filename = argv[1];
    if (argc > 2)
        mode = argv[2];
    if (argc > 3)
        num_threads = std::stoi(argv[3]);

    // Check if file is readable
    ifstream check(filename, std::ios::binary);
    if (!check.is_open())
    {
        cerr << "Could not open '" << filename << "'. Please provide a readable text file as the first argument." << endl;
        cerr << "Usage: " << (argc > 0 ? argv[0] : "TME3") << " [path/to/textfile] [mode] [num threads]" << endl;
        return 2;
    }
    check.seekg(0, std::ios::end);
    std::streamoff file_size = check.tellg();
    check.close();

    cout << "Preparing to parse " << filename << " (mode=" << mode << " N=" << num_threads << "), containing " << file_size << " bytes" << endl;

    auto start = steady_clock::now();

    std::vector<std::pair<std::string, int>> pairs;

    if (mode == "freqstd")
    {
        ifstream input(filename, std::ios::binary);
        size_t total_words = 0;
        size_t unique_words = 0;
        std::unordered_map<std::string, int> um;
        std::string word;
        while (input >> word)
        {
            word = pr::cleanWord(word);
            if (!word.empty())
            {
                total_words++;
                ++um[word];
            }
        }
        unique_words = um.size();
        pairs.reserve(unique_words);
        for (const auto &p : um)
            pairs.emplace_back(p);
        pr::printResults(total_words, unique_words, pairs, mode + ".freq");
    }
    else if (mode == "freqstdf")
    {
        size_t total_words = 0;
        size_t unique_words = 0;
        std::unordered_map<std::string, int> um;
        pr::processRange(filename, 0, file_size, [&](const std::string &word)
                         {
                        total_words++;
                        um[word]++; });
        unique_words = um.size();
        pairs.reserve(unique_words);
        for (const auto &p : um)
            pairs.emplace_back(p);
        pr::printResults(total_words, unique_words, pairs, mode + ".freq");
    }
    else if (mode == "partition")
    {
        size_t total_words = 0;
        size_t unique_words = 0;
        std::unordered_map<std::string, int> um;

        auto partitions = pr::partition(filename, file_size, num_threads);
        for (int i = 0; i < num_threads; ++i)
        {
            pr::processRange(filename, partitions[i], partitions[i + 1], [&](const std::string &word)
                             {
                                total_words++;
                                um[word]++; });
        }

        unique_words = um.size();
        pairs.reserve(unique_words);
        for (const auto &p : um)
            pairs.emplace_back(p);
        pr::printResults(total_words, unique_words, pairs, mode + ".freq");
    }
    else if (mode == "mt_naive")
    {
        size_t total_words = 0;
        size_t unique_words = 0;
        std::vector<std::unordered_map<std::string, int>> local_maps(num_threads);
        std::vector<size_t> local_counts(num_threads, 0);
        auto partitions = pr::partition(filename, file_size, num_threads);
        std::vector<std::thread> threads;

        for (int i = 0; i < num_threads; ++i)
        {
            threads.emplace_back([&, i]()
                                 { pr::processRange(filename, partitions[i], partitions[i + 1], [&](const std::string &word)
                                                    {
                                                                local_counts[i]++;
                                                                local_maps[i][word]++; }); });
        }
        for (auto &t : threads)
            t.join();

        // Merge results
        std::unordered_map<std::string, int> um;
        for (int i = 0; i < num_threads; ++i)
        {
            total_words += local_counts[i];
            for (const auto &p : local_maps[i])
            {
                um[p.first] += p.second;
            }
        }
        unique_words = um.size();
        pairs.reserve(unique_words);
        for (const auto &p : um)
            pairs.emplace_back(p);
        pr::printResults(total_words, unique_words, pairs, mode + ".freq");
    }
    else if (mode == "mt_hnaive")
    {
        size_t total_words = 0;
        size_t unique_words = 0;
        HashMap<std::string, int> hm;//hmap pas protege

        auto partitions = pr::partition(filename, file_size, num_threads);
        std::vector<std::thread> threads;

        for (int i = 0; i < num_threads; ++i)
        {
            threads.emplace_back([&, i]() {
                pr::processRange(filename, partitions[i], partitions[i + 1], [&](const std::string &word) {
                    total_words++;
                    hm.incrementFrequency(word);//pr partager les info entre les threads
                });
            });
        }

        for (auto &t : threads)
        {
            t.join();
        }
        pairs = hm.toKeyValuePairs();
        unique_words = pairs.size();
        pr::printResults(total_words, unique_words, pairs, mode + ".freq");
    }
    else if (mode == "mt_atomic")
    {
        std::atomic<size_t> total_words(0);
        size_t unique_words = 0;
        std::vector<std::unordered_map<std::string, int>> local_maps(num_threads);
        std::vector<size_t> local_counts(num_threads, 0);
        auto partitions = pr::partition(filename, file_size, num_threads);
        std::vector<std::thread> threads;

        for (int i = 0; i < num_threads; ++i){
            threads.emplace_back([&, i](){
                pr::processRange(filename, partitions[i], partitions[i + 1], [&](const std::string &word){
                    local_counts[i]++;
                    local_maps[i][word]++;
                    total_words.fetch_add(1, std::memory_order_relaxed);
                    });
                });
            }
        for (auto &t : threads)
            t.join();

        //fusion ici
        std::unordered_map<std::string, int> um;
        for (int i = 0; i < num_threads; ++i)
        {
            for (const auto &p : local_maps[i])
            {
                um[p.first] += p.second;
            }
        }
        unique_words = um.size();
        pairs.reserve(unique_words);
        for (const auto &p : um)
            pairs.emplace_back(p);
        pr::printResults(total_words.load(), unique_words, pairs, mode + ".freq");
    }
    else if (mode == "mt_mutex")
    {
        size_t total_words = 0;
        size_t unique_words = 0;
        std::unordered_map<std::string, int> um;
        std::mutex mutex;

        auto partitions = pr::partition(filename, file_size, num_threads);
        std::vector<std::thread> threads;

        for (int i = 0; i < num_threads; ++i)
        {
            threads.emplace_back([&, i]() {
                pr::processRange(filename, partitions[i], partitions[i + 1], [&](const std::string &word) {
                    std::lock_guard<std::mutex> lock(mutex);
                    total_words++;
                    um[word]++;
                });
            });
        }

        for (auto &t : threads)
        {
            t.join();
        }

        unique_words = um.size();
        pairs.reserve(unique_words);
        for (const auto &p : um)
            pairs.emplace_back(p);
        pr::printResults(total_words, unique_words, pairs, mode + ".freq");
    }
    else if (mode == "mt_hashes")
    {
        size_t total_words = 0;
        size_t unique_words = 0;
        std::vector<std::unordered_map<std::string, int>> local_maps(num_threads);
        std::vector<size_t> local_counts(num_threads, 0);

        auto partitions = pr::partition(filename, file_size, num_threads);
        std::vector<std::thread> threads;

        for (int i = 0; i < num_threads; ++i)
        {
            threads.emplace_back([&, i]() {
                pr::processRange(filename, partitions[i], partitions[i + 1], [&](const std::string &word) {
                    local_counts[i]++;
                    local_maps[i][word]++;
                });
            });
        }

        for (auto &t : threads)
        {
            t.join();
        }

        // Merge results
        std::unordered_map<std::string, int> um;
        for (int i = 0; i < num_threads; ++i)
        {
            total_words += local_counts[i];
            for (const auto &p : local_maps[i])
            {
                um[p.first] += p.second;
            }
        }

        unique_words = um.size();
        pairs.reserve(unique_words);
        for (const auto &p : um)
            pairs.emplace_back(p);
        pr::printResults(total_words, unique_words, pairs, mode + ".freq");
    }
    else if (mode == "freq")
    {
        size_t total_words = 0;
        size_t unique_words = 0;
        HashMap<std::string, int> hm;
        pr::processRange(filename, 0, file_size, [&](const std::string &word)
                         {
                        total_words++;
                        hm.incrementFrequency(word); });
        pairs = hm.toKeyValuePairs();
        unique_words = pairs.size();
        pr::printResults(total_words, unique_words, pairs, mode + ".freq");
    }
    else
    {
        cerr << "Unknown mode '" << mode << "'. Supported modes: freqstd, freq, freqstdf, partition, mt_naive, mt_hnaive, mt_atomic, mt_mutex, mt_hashes" << endl;
        return 1;
    }

    // print a single total runtime for successful runs
    auto end = steady_clock::now();
    cout << "Total runtime (wall clock) : " << duration_cast<milliseconds>(end - start).count() << " ms" << endl;

    return 0;
}
