#include "PmergeMe.hpp"
#include <iostream>
#include <iomanip>
#include <vector>
#include <deque>
#include <sys/time.h>
#include <algorithm>


static double now_us() {
    struct timeval tv;
    gettimeofday(&tv, 0);
    return tv.tv_sec * 1e6 + tv.tv_usec;
}

template <typename It>
static void printRange(const char* prefix, It first, It last) {
    std::cout << prefix;
    for (It it = first; it != last; ++it) {
        std::cout << *it;
        if (it + 1 != last) std::cout << ' ';
    }
    std::cout << '\n';
}

int main(int argc, char** argv) {
    try {
        std::vector<int> input = PmergeMe::parseArgs(argc, argv);

        printRange("Before: ", input.begin(), input.end());

        // Vector timing (includes data management)
        double t0v = now_us();
        std::vector<int> v(input.begin(), input.end());
        PmergeMe::sortVector(v);
        double t1v = now_us();

        // Deque timing (includes data management)
        double t0d = now_us();
        std::deque<int>  d(input.begin(), input.end());
        PmergeMe::sortDeque(d);
        double t1d = now_us();

        if (v.size() != d.size() || !std::equal(v.begin(), v.end(), d.begin())) {
            std::cerr << "Error\n";
            return 1;
        }

        printRange("After:  ", v.begin(), v.end());

        std::cout.setf(std::ios::fixed);
        std::cout << std::setprecision(5);

        std::cout << "Time to process a range of " << v.size()
                  << " elements with std::vector : " << (t1v - t0v) << " us\n";
        std::cout << "Time to process a range of " << d.size()
                  << " elements with std::deque  : " << (t1d - t0d) << " us\n";

        return 0;
    } catch (...) {
        std::cerr << "Error\n";
        return 1;
    }
}
