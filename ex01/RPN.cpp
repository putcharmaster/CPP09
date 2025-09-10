#include "RPN.hpp" 
#include <stdexcept>
#include <sstream>
#include <cctype>

RPN::RPN(){}
RPN::RPN(const RPN& other) {(void)other;}
RPN& RPN::operator=(const RPN& other){
    (void)other;
    return *this;
}
RPN::~RPN(){}

bool RPN::isOperator(char c){
    return (c == '+' ||c == '-' ||c == '*' ||c == '/');
}

long RPN::applyOp(long a, long b, char op){
    switch(op){
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/':
            if (b == 0)
                throw std::runtime_error("division by zero");
            return a / b;
    }
    throw std::runtime_error("unknown operator");
}

long RPN::evaluate(const std::string& expr) const{
    if (expr.empty())
        throw std::runtime_error("empty expression");

    std::istringstream iss(expr.c_str());
    std::string tok;
    std::stack<long> st;

    while (iss >> tok){
        if (tok.size() != 1)
            throw std::runtime_error("invalid token length");
            
        char c = tok[0];

        if (std::isdigit(static_cast<unsigned char>(c))){
            st.push(static_cast<unsigned char>(c - '0'));
        }
        else if (isOperator(c)){
            if (st.size() < 2)
                throw std::runtime_error("not enough operands");
            long b = st.top();
            st.pop();
            long a = st.top();
            st.pop();
            long r = applyOp(a,b,c);
            st.push(r);
        }
        else{
            throw std::runtime_error("invalid character");
        }
    }

    if (st.size() != 1)
        throw std::runtime_error("leftover values");
    
    return st.top();
}