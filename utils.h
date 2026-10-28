#ifndef UTILS_H
#define UTILS_H

#include <future>
#include <iostream>
#include <filesystem>

#include <ctime>

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

//inline long long stringToMsFromEpochUTC(const std::string& s) {
//    std::tm tm{};
//    std::istringstream ss(s);
//    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
//    if (ss.fail()) throw std::runtime_error("bad time: " + s);

//    auto sys = std::chrono::system_clock::from_time_t(timegm(&tm));
//    // sys -= std::chrono::hours(3);
//    return std::chrono::duration_cast<std::chrono::milliseconds>(
//               sys.time_since_epoch()).count();
//}

inline long long stringToMsFromEpochUTC(const std::string& s) {
    std::tm tm{};
    std::istringstream ss(s);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    if (ss.fail()) {
        throw std::runtime_error("Error time extracting manipulator: " + s);
    }
//    tm.tm_hour -= 3;
    std::time_t t = timegm(&tm);
    return static_cast<long long>(t) * 1000;
}

#endif // UTILS_H
