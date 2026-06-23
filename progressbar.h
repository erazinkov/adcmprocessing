#ifndef PROGRESSBAR_H
#define PROGRESSBAR_H

#include <iostream>
#include <iomanip>
#include <math.h>

template<typename T>
class ProgressBar
{
public:
    ProgressBar(){}
    static void show(T current, T total)
    {
        const int width{50};
        double progress{static_cast<double>(current) / static_cast<double>(total)};
        int pos{static_cast<int>(width * progress)};
        std::cout << "[";
        for (int i = 0; i < width; i++) {
            if (i < pos) {
                std::cout << "=";
            }
            else if (i == pos) {
                std::cout << ">";
            }
            else {
                std::cout << " ";
            }
        }
        std::cout << "]" << std::setw(4) << std::ceil(static_cast<double>(pos) /  static_cast<double>(width) * 100.0) << "%\r";
        std::cout.flush();

    }
};

#endif // PROGRESSBAR_H
