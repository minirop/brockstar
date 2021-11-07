#ifndef FUNCTION_H
#define FUNCTION_H

#include "token.h"
#include <vector>

class Evaluator;
class Function
{
public:
    Function();

    void addToken(Token token);
    int args() const;
    void addParameter(std::string name);
    Value call(Evaluator * parent, Array arguments);

private:
    std::vector<std::string> parameters;
    std::vector<Token> tokens;
};

#endif // FUNCTION_H
