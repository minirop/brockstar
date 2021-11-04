#ifndef FUNCTION_H
#define FUNCTION_H

#include "token.h"
#include <vector>

class Function
{
public:
    Function();

    void addToken(Token token);
    int args() const;
    void addParameter(std::string name);
    Value call(Array arguments);

private:
    std::vector<std::string> parameters;
    std::vector<Token> tokens;
};

#endif // FUNCTION_H
