#ifndef CONSOLETABLE_H
#define CONSOLETABLE_H

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>

// ANSI color codes
#define RESET   "\033[0m"
#define BLACK   "\033[30m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"
#define BOLD    "\033[1m"

class ConsoleTable
{
public:
    ConsoleTable(const std::vector<std::vector<std::string>> &);

    void show();
private:
    std::vector<std::vector<std::string>> data_;
    std::vector<size_t> widths_;

    void init();
};

#endif // CONSOLETABLE_H
