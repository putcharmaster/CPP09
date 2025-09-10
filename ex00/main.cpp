#include "BitcoinExchange.hpp"
#include <iostream>

int main(int ac, char **av){
    if (ac != 2){
        std::cout << "Error: could not open file." << std::endl;
        return 1;
    }

    BitcoinExchange b;
    try{
        b.loadCSV("data.csv");

    }
    catch(const std::exception& e){
        std::cout << e.what() << std::endl;
        return 1;
    }

    b.processInputFile(av[1]);
    return 0;
}