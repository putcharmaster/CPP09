#include "PmergeMe.hpp"
#include <stdexcept>
#include <climits>
#include <cstdlib>
#include <algorithm>
#include <cctype>
#include <set>

// ---------- Public: parsing ----------
std::vector<int> PmergeMe::parseArgs(int argc, char** argv) {
    if (argc <= 1) throw std::runtime_error("Error");

    std::vector<int> out;
    out.reserve(argc - 1);
    std::set<int> seen; // duplicate detection

    for (int i = 1; i < argc; ++i) {
        const char* s = argv[i];
        if (!isPositiveInteger(s)) throw std::runtime_error("Error");

        // overflow-safe parse into int (C++98-friendly, no long long)
        int val = 0;
        for (const char* p = s; *p; ++p) {
            int digit = *p - '0';
            if (val > (INT_MAX - digit) / 10) throw std::runtime_error("Error"); // overflow
            val = val * 10 + digit;
        }
        if (val <= 0) throw std::runtime_error("Error");

        if (!seen.insert(val).second) throw std::runtime_error("Error"); // duplicate
        out.push_back(val);
    }
    return out;
}

bool PmergeMe::isPositiveInteger(const char* s) {
    if (!s || !*s) return false;
    for (const char* p = s; *p; ++p)
        if (!std::isdigit(static_cast<unsigned char>(*p)))
            return false;
    if (s[0] == '0') return false; // reject zero + leading zeros
    return true;
}

// ---------- Public: sort entry points ----------
void PmergeMe::sortVector(std::vector<int>& v) { fordJohnsonVector(v); }
void PmergeMe::sortDeque(std::deque<int>& d) { fordJohnsonDeque(d); }

// ====================== VECTOR VERSION ======================
void PmergeMe::buildPairsVector(const std::vector<int>& v,
                                std::vector<PairV>& pairs,
                                bool& hasStraggler, int& straggler) {
    hasStraggler = (v.size() % 2 != 0);
    if (hasStraggler) straggler = v.back();
    size_t nPairs = v.size() / 2;
    pairs.clear();
    pairs.reserve(nPairs);
    for (size_t i = 0; i < nPairs; ++i) {
        int a = v[2*i], b = v[2*i + 1];
        PairV pr;
        if (a >= b) { pr.big = a; pr.small = b; }
        else        { pr.big = b; pr.small = a; }
        pr.used = false;
        pairs.push_back(pr);
    }
}

void PmergeMe::sortBigsVector(std::vector<PairV>& pairs, std::vector<int>& bigs) {
    bigs.clear();
    bigs.reserve(pairs.size());
    for (size_t i = 0; i < pairs.size(); ++i) bigs.push_back(pairs[i].big);
    fordJohnsonVector(bigs);
}

void PmergeMe::reorderPairsByBigsVector(std::vector<PairV>& pairs, const std::vector<int>& bigs) {
    std::vector<PairV> ordered;
    ordered.reserve(pairs.size());
    for (size_t i = 0; i < bigs.size(); ++i) {
        int target = bigs[i];
        for (size_t j = 0; j < pairs.size(); ++j) {
            if (!pairs[j].used && pairs[j].big == target) {
                ordered.push_back(pairs[j]);
                pairs[j].used = true;
                break;
            }
        }
    }
    pairs.swap(ordered);
    for (size_t i = 0; i < pairs.size(); ++i) pairs[i].used = false;
}

std::vector<size_t> PmergeMe::jacobsthalOrder(size_t n) {
    if (n == 0) return std::vector<size_t>();
    if (n == 1) return std::vector<size_t>(1, 0);
    std::vector<size_t> J; J.push_back(0); J.push_back(1);
    while (J.back() < n) {
        size_t a = J[J.size()-1], b = J[J.size()-2];
        J.push_back(a + 2*b);
        if (J.back() == J[J.size()-2]) break;
    }
    std::vector<size_t> order;
    order.push_back(0);
    size_t k = 2;
    while (k < J.size() && J[k] < n) {
        size_t hi = J[k];
        size_t lo = J[k-1] + 1;
        if (hi >= n) hi = n - 1;
        if (lo >= n) { ++k; continue; }
        for (size_t idx = hi; idx >= lo; --idx) {
            order.push_back(idx);
            if (idx == lo) break;
        }
        ++k;
    }
    size_t lastJ = (J.size() >= 2) ? J[J.size()-2] : 1;
    size_t start = (n-1);
    size_t end = (lastJ + 1);
    if (end < n) {
        for (size_t idx = start; idx >= end; --idx) {
            if (std::find(order.begin(), order.end(), idx) == order.end())
                order.push_back(idx);
            if (idx == end) break;
        }
    }
    return order;
}

void PmergeMe::boundedInsertVector(std::vector<int>& chain, int value, int boundValue) {
    std::vector<int>::iterator boundPos = std::lower_bound(chain.begin(), chain.end(), boundValue);
    std::vector<int>::iterator pos = std::lower_bound(chain.begin(), boundPos, value);
    chain.insert(pos, value);
}

void PmergeMe::insertSmallsVector(std::vector<int>& chain, const std::vector<PairV>& pairs) {
    if (pairs.empty()) return;

    chain.clear();
    chain.reserve(pairs.size());

    // 1) Start the chain with all "big" elements in their (sorted-by-big) order
    for (size_t i = 0; i < pairs.size(); ++i)
        chain.push_back(pairs[i].big);

    // 2) Insert "small" elements following Jacobsthal order (bounded binary insert)
    std::vector<size_t> order = jacobsthalOrder(pairs.size());
    for (size_t t = 0; t < order.size(); ++t) {
        size_t i = order[t];
        int small = pairs[i].small;
        int bound = pairs[i].big;
        boundedInsertVector(chain, small, bound);
    }

    // 3) SAFETY PASS: insert any remaining "small" elements that weren't scheduled
    for (size_t i = 0; i < pairs.size(); ++i) {
        int small = pairs[i].small;
        // if not already present, insert it bounded by its pair's big
        if (std::find(chain.begin(), chain.end(), small) == chain.end()) {
            int bound = pairs[i].big;
            boundedInsertVector(chain, small, bound);
        }
    }
}


void PmergeMe::fordJohnsonVector(std::vector<int>& v) {
    if (v.size() <= 1) return;
    std::vector<PairV> pairs;
    bool hasStraggler = false; int straggler = 0;
    buildPairsVector(v, pairs, hasStraggler, straggler);
    std::vector<int> bigs;
    sortBigsVector(pairs, bigs);
    reorderPairsByBigsVector(pairs, bigs);
    std::vector<int> chain;
    insertSmallsVector(chain, pairs);
    if (hasStraggler) {
        std::vector<int>::iterator pos = std::lower_bound(chain.begin(), chain.end(), straggler);
        chain.insert(pos, straggler);
    }
    v.swap(chain);
}

// ====================== DEQUE VERSION ======================
void PmergeMe::buildPairsDeque(const std::deque<int>& d,
                               std::vector<PairD>& pairs,
                               bool& hasStraggler, int& straggler) {
    hasStraggler = (d.size() % 2 != 0);
    if (hasStraggler) straggler = d.back();
    size_t nPairs = d.size() / 2;
    pairs.clear();
    pairs.reserve(nPairs);
    for (size_t i = 0; i < nPairs; ++i) {
        int a = d[2*i], b = d[2*i + 1];
        PairD pr;
        if (a >= b) { pr.big = a; pr.small = b; }
        else        { pr.big = b; pr.small = a; }
        pr.used = false;
        pairs.push_back(pr);
    }
}

void PmergeMe::sortBigsDeque(std::vector<PairD>& pairs, std::deque<int>& bigs) {
    bigs.clear();
    for (size_t i = 0; i < pairs.size(); ++i) bigs.push_back(pairs[i].big);
    fordJohnsonDeque(bigs);
}

void PmergeMe::reorderPairsByBigsDeque(std::vector<PairD>& pairs, const std::deque<int>& bigs) {
    std::vector<PairD> ordered;
    ordered.reserve(pairs.size());
    for (size_t i = 0; i < bigs.size(); ++i) {
        int target = bigs[i];
        for (size_t j = 0; j < pairs.size(); ++j) {
            if (!pairs[j].used && pairs[j].big == target) {
                ordered.push_back(pairs[j]);
                pairs[j].used = true;
                break;
            }
        }
    }
    pairs.swap(ordered);
    for (size_t i = 0; i < pairs.size(); ++i) pairs[i].used = false;
}

void PmergeMe::boundedInsertDeque(std::deque<int>& chain, int value, int boundValue) {
    std::deque<int>::iterator boundPos = std::lower_bound(chain.begin(), chain.end(), boundValue);
    std::deque<int>::iterator pos = std::lower_bound(chain.begin(), boundPos, value);
    chain.insert(pos, value);
}

void PmergeMe::insertSmallsDeque(std::deque<int>& chain, const std::vector<PairD>& pairs) {
    if (pairs.empty()) return;

    chain.clear();

    // 1) Start the chain with all "big" elements in their (sorted-by-big) order
    for (size_t i = 0; i < pairs.size(); ++i)
        chain.push_back(pairs[i].big);

    // 2) Insert "small" elements following Jacobsthal order (bounded binary insert)
    std::vector<size_t> order = jacobsthalOrder(pairs.size());
    for (size_t t = 0; t < order.size(); ++t) {
        size_t i = order[t];
        int small = pairs[i].small;
        int bound = pairs[i].big;
        boundedInsertDeque(chain, small, bound);
    }

    // 3) SAFETY PASS: insert any remaining "small" elements that weren't scheduled
    for (size_t i = 0; i < pairs.size(); ++i) {
        int small = pairs[i].small;
        if (std::find(chain.begin(), chain.end(), small) == chain.end()) {
            int bound = pairs[i].big;
            boundedInsertDeque(chain, small, bound);
        }
    }
}


void PmergeMe::fordJohnsonDeque(std::deque<int>& d) {
    if (d.size() <= 1) return;
    std::vector<PairD> pairs;
    bool hasStraggler = false; int straggler = 0;
    buildPairsDeque(d, pairs, hasStraggler, straggler);
    std::deque<int> bigs;
    sortBigsDeque(pairs, bigs);
    reorderPairsByBigsDeque(pairs, bigs);
    std::deque<int> chain;
    insertSmallsDeque(chain, pairs);
    if (hasStraggler) {
        std::deque<int>::iterator pos = std::lower_bound(chain.begin(), chain.end(), straggler);
        chain.insert(pos, straggler);
    }
    d.swap(chain);
}
