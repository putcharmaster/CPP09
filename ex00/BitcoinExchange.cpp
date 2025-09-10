#include "BitcoinExchange.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <climits>
#include <cctype>

BitcoinExchange::BitcoinExchange(){}
BitcoinExchange::~BitcoinExchange(){}

std::string BitcoinExchange::trim(const std::string& s){
    std::string::size_type start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) ++start;
    std::string::size_type end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) --end;
    return s.substr(start, end - start);
}

bool BitcoinExchange::isDigits(const std::string& s){
    for (std::string::size_type i = 0; i < s.size(); ++i){
        if (!std::isdigit(static_cast<unsigned char>(s[i]))) 
            return false;
    }
    return !s.empty();
}

bool BitcoinExchange::parseDouble(const std::string& s, double& out){
    char* endptr = 0;
    const char* c = s.c_str();
    errno = 0;
    double val = std::strtod(c, &endptr);
    if (c == endptr)
        return false;
    while (*endptr && std::isspace(static_cast<unsigned char>(*endptr))) ++endptr;
    if (*endptr != '\0')
        return false;
    if (errno == ERANGE)
        return false;
    out = val;
    return true;
}

bool BitcoinExchange::isLeap(int y){
    return ((y % 4 == 0) && (y % 100 != 0)) || (y % 400 == 0);
}

int BitcoinExchange::daysInMonth(int y, int m){
    static const int d[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (m == 2) return d[1] + (isLeap(y) ? 1 : 0);
    if (m >= 1 && m <= 12) return d[m-1];
    return 0;
}

bool BitcoinExchange::parseYMD(const std::string& d, int& y, int& m, int& day){
    if (d.size() != 10 || d[4] != '-' || d[7] != '-') return false;
    std::string ys = d.substr(0,4);
    std::string ms = d.substr(5,2);
    std::string ds = d.substr(8,2);
    if (!isDigits(ys) || !isDigits(ms) ||!isDigits(ds)) return false;
    y = std::atoi(ys.c_str());
    m = std::atoi(ms.c_str());
    day = std::atoi(ds.c_str());
    return true;
}

bool BitcoinExchange::isValidDate(const std::string& date){
    int y,m,d;

    if(!parseYMD(date, y, m, d)) return false;
    if (m < 1 || m > 12) return false;
    int dim = daysInMonth(y,m);
    if (d < 1 || d > dim) return false;
    return true;
}

bool BitcoinExchange::getRateForDate(const std::string& date, double& rate) const {
    std::map<std::string, double>::const_iterator it = _rates.lower_bound(date);
    if (it != _rates.end() && it->first == date){
        rate = it->second;
        return true;
    }
    if (it == _rates.begin()) return false;
    --it;
    rate = it->second;
    return true;
}

std::string BitcoinExchange::formatDouble(double x){
    std::ostringstream oss;
    oss.setf(std::ios::fixed);
    oss.precision(10);
    oss << x;
    std::string s = oss.str();

    std::string::size_type pos = s.find('.');
    if (pos != std::string::npos){
        std::string::size_type end = s.size();
        while (end > pos + 1 && s[end - 1] == '0')   --end;
        if (end > 0 && s[end - 1] == '.')   --end;
        s.erase(end);
    }
    return s;
}

void BitcoinExchange::loadCSV(const std::string& path){
    std::ifstream in(path.c_str());
    if (!in) {
        throw std::runtime_error("Error: could not open file.");
    }

    std::string line;
    while(std::getline(in, line)){
        if (line.empty()) continue;
        std::string::size_type comma = line.find(',');
        if (comma == std::string::npos)  continue;
        std::string date = trim(line.substr(0, comma));
        std::string rateStr = trim(line.substr(comma + 1));
        if (!isValidDate(date)) continue;
        
        double rate;
        if (!parseDouble(rateStr, rate)) continue;
        _rates[date] = rate;
    }
}

void BitcoinExchange::processInputFile(const std::string& inputPath) const{
    std::ifstream in(inputPath.c_str());
    if (!in){
        std::cout << "Error: could not open file." << std::endl;
        return;
    }
    std::string line;
    bool first = true;
    while (std::getline(in, line)){
        std::string original = line;
        line = trim(line);
        if (line.empty()) continue;
        if (!line.empty() && line[0] == '#') continue;

        if(first){
            first = false;
            std::string lower = line;
            for (size_t i = 0; i < lower.size(); ++i)
                lower[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(lower[i])));
            std::string noSpaces;
            for (size_t i = 0; i < lower.size(); ++i){
                if (!std::isspace(static_cast<unsigned char>(lower[i])))
                    noSpaces += lower[i];
            }
            if (noSpaces == "date|value") continue;
        }

        std::string::size_type bar = line.find('|');
        if (bar == std::string::npos){
            std::cout << "Error: bad input => " << original << std::endl;
            continue;
        }

        std::string date = trim(line.substr(0, bar));
        std::string valueStr = trim(line.substr(bar + 1));

        if (!isValidDate(date)){
            std::cout << "Error: bad input => " << original << std::endl;
            continue;
        }

        double value;
        if (!parseDouble(valueStr, value)){
            std::cout << "Error: bad input => " << original << std::endl;
            continue;
        }

        if (value < 0.0) {
            std::cout << "Error: not a positive number." <<std::endl;
            continue;
        }

        if (value > 1000.0) {
            std::cout << "Error: too large a number." <<std::endl;
            continue;
        }

        double rate;
        if (!getRateForDate(date, rate)){
            std::cout << "Error: bad input => " << original << std::endl;
            continue;
        }

        double result = value * rate;
        std::cout << date << " => " << valueStr << " = " << formatDouble(result) << std::endl;
    }
}