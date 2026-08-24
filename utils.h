#ifndef UTILS_H
#define UTILS_H

#include <future>
#include <iostream>
#include <filesystem>

template<typename Iterator>
inline void func_async(Iterator first, Iterator last) {
    unsigned long const length{static_cast<unsigned long const>(std::distance(first, last))};
    unsigned long const max_chunk_size{7};
    if (length < max_chunk_size)
    {
        for (auto it{first}; it != last; ++it)
        {
            (*it)();
        }
    }
    else
    {
        Iterator mid_point{first};
        std::advance(mid_point, length / 2);
        std::future<void> first_half = std::async(func_async<Iterator>, first, mid_point);
        func_async(mid_point, last);
        first_half.get();
    }
}

inline bool createOutputDirectory(const std::string& path) {
    try {
        if (!std::filesystem::exists(path)) {
            if (std::filesystem::create_directories(path)) {
                std::cout << "Output directory created: " << path << std::endl;
                return true;
            } else {
                std::cerr << "Failed to create output directory: " << path << std::endl;
                return false;
            }
        }
        std::cout << "Output directory already exists: " << path << std::endl;
        return true;
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
        return false;
    }
}

#endif // UTILS_H
