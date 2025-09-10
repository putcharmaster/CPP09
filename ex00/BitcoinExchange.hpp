#ifndef BITCOINEXCHANGE_HPP
#define BITCOINEXCHANGE_HPP

#include <map>
#include <string>

class BitcoinExchange{
    public:
        BitcoinExchange();
        ~BitcoinExchange();

        void loadCSV(const std::string& path);
        void processInputFile(const std::string& inputPath) const;

    private:
        std::map<std::string, double> _rates;

        static std::string trim(const std::string& s);
        static bool isDigits(const std::string& s);
        static bool parseDouble(const std::string& s, double& out);        
        static bool isValidDate(const std::string& date);
        static bool parseYMD(const std::string& d, int& y, int& m, int& day);        
        static bool isLeap(int y);
        static int daysInMonth(int y, int m);

        bool getRateForDate(const std::string& date, double& rate) const;
        static std::string formatDouble(double x);
};

#endif
