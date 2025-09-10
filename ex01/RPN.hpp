#ifndef RPN_HPP
#define RPN_HPP

#include <string>
#include <stack>

class RPN{
    public:
        RPN();
        RPN(const RPN& other);
        RPN& operator=(const RPN& other);
        ~RPN();
        
        long evaluate(const std::string& expr) const;
    
    private:
        static bool isOperator(char c);
        static long applyOp(long a, long b, char op);
};

#endif