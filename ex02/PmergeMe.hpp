#ifndef PMERGEME_HPP
#define PMERGEME_HPP

#include <vector>
#include <deque>
#include <string>

class PmergeMe {
public:
    static std::vector<int> parseArgs(int argc, char** argv);

    static void sortVector(std::vector<int>& v);
    static void sortDeque(std::deque<int>& d);

private:
    // ----- Vector implementation -----
    struct PairV { int big; int small; bool used; };
    static void fordJohnsonVector(std::vector<int>& v);
    static void buildPairsVector(const std::vector<int>& v,
                                 std::vector<PairV>& pairs,
                                 bool& hasStraggler, int& straggler);
    static void sortBigsVector(std::vector<PairV>& pairs, std::vector<int>& bigs);
    static void reorderPairsByBigsVector(std::vector<PairV>& pairs, const std::vector<int>& bigs);
    static void insertSmallsVector(std::vector<int>& chain, const std::vector<PairV>& pairs);
    static std::vector<size_t> jacobsthalOrder(size_t n);
    static void boundedInsertVector(std::vector<int>& chain, int value, int boundValue);

    // ----- Deque implementation -----
    struct PairD { int big; int small; bool used; };
    static void fordJohnsonDeque(std::deque<int>& d);
    static void buildPairsDeque(const std::deque<int>& d,
                                std::vector<PairD>& pairs,
                                bool& hasStraggler, int& straggler);
    static void sortBigsDeque(std::vector<PairD>& pairs, std::deque<int>& bigs);
    static void reorderPairsByBigsDeque(std::vector<PairD>& pairs, const std::deque<int>& bigs);
    static void insertSmallsDeque(std::deque<int>& chain, const std::vector<PairD>& pairs);
    static void boundedInsertDeque(std::deque<int>& chain, int value, int boundValue);

    static bool isPositiveInteger(const char* s);
};

#endif
