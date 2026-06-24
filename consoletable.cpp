#include "consoletable.h"

ConsoleTable::ConsoleTable(const std::vector<std::vector<std::string>>& data) : data_{data}
{
    init();
}

void ConsoleTable::show()
{
    if (data_.empty()) return;

    std::cout << GREEN << "+";
    for (size_t w : widths_) {
        std::cout << std::string(w + 2, '-') << "+";
    }
    std::cout << RESET << "\n";

    for (size_t r{0}; r < data_.size(); ++r) {
        std::cout << GREEN << "|" << RESET;
        for (size_t c{0}; c < data_[r].size(); ++c) {
            std::string cell = data_[r][c];
            size_t cleanLen = widths_[c];
            size_t pos;
            while ((pos = cell.find("\033[")) != std::string::npos) {
                size_t end = cell.find("m", pos);
                if (end != std::string::npos) {
                    cell.erase(pos, end - pos + 1);
                }
            }
            std::cout << " " << data_[r][c]
                      << std::string(cleanLen - cell.length() + 1, ' ')
                      << GREEN << "|" << RESET;
        }
        std::cout << "\n";

        if (r == 0) {
            std::cout << GREEN << "+";
            for (size_t w : widths_) {
                std::cout << std::string(w + 2, '-') << "+";
            }
            std::cout << RESET << "\n";
        }
    }

    std::cout << GREEN << "+";
    for (size_t w : widths_) {
        std::cout << std::string(w + 2, '-') << "+";
    }
    std::cout << RESET << "\n";
}

void ConsoleTable::init()
{
    if (data_.empty()) {
        return;
    }
    widths_.assign(data_[0].size(), 0);
    for (const auto& row : data_) {
        for (size_t i{0}; i < row.size(); ++i) {
            std::string clean = row[i];
            size_t pos;
            while ((pos = clean.find("\033[")) != std::string::npos) {
                size_t end = clean.find("m", pos);
                if (end != std::string::npos) {
                    clean.erase(pos, end - pos + 1);
                }
            }
            widths_[i] = std::max(widths_[i], clean.length());
        }
    }
}
