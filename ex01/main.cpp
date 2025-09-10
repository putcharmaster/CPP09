#include "RPN.hpp"
#include <iostream>

int main(int ac, char **av){
    if (ac != 2){
        std::cerr << "Error" << std::endl;
        return 1;
    }

    try{
        RPN calc;
        long result = calc.evaluate(av[1]);
        std::cout << result << std::endl;
    }
    catch (const std::exception& e){
        std::cerr << "Error" << std::endl;
        return 1;
    }
    return 0;
}