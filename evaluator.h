#ifndef EVALUATOR_H
#define EVALUATOR_H

#include "token.h"
#include "value.h"
#include <vector>
#include <unordered_map>
#include "function.h"

class Evaluator
{
public:
    Evaluator(std::vector<Token> tokens);

    void setVariable(std::string name, Value value);
    void setVariable(std::string name, int index, Value value);
    Value eval();

private:
    std::vector<Token> tokens;
    size_t pc = 0;
    std::string lastVariableNamed;
    std::string isInFunction;

    void parseVariable(std::string name);
    void parsePoeticNumberVariable(std::string name);
    void parsePoeticStringVariable(std::string name);
    Value evaluateExpression(std::string variable = {});
    Array evaluateList();
    Value calculate(std::vector<Token> result);
    bool isExpressionToken(Token::Type type);
    void setPronoun(std::string name);
    void startFunctionDeclaration(std::string name);
    Value executeFunction(std::string name);

    std::unordered_map<std::string, Function> functions;

    void shout();
    void let();
    void put();
    void build();
    void knock();
    void rock();
    Value roll();

    std::unordered_map<std::string, Value> variables;
};

#endif // EVALUATOR_H
