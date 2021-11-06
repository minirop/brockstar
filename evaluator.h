#ifndef EVALUATOR_H
#define EVALUATOR_H

#include "token.h"
#include "value.h"
#include <vector>
#include <unordered_map>
#include "function.h"

enum class Operator {
    Equal,
    NotEqual,
    GreaterThan,
    LowerThan,
    GreaterOrEqual,
    LowerOrEqual,
};

class Evaluator
{
public:
    Evaluator(std::vector<Token> tokens);

    void setVariable(std::string name, Value value);
    void setVariable(std::string name, int index, Value value);
    Value eval();

private:
    std::vector<std::vector<Token>> lines;
    size_t line = 0;
    std::vector<Token> tokens;
    size_t pc = 0;
    std::string lastVariableNamed;
    std::string isInFunction;
    std::vector<size_t> loops;

    void parseVariable(std::string name);
    void parsePoeticNumberVariable(std::string name);
    void parsePoeticStringVariable(std::string name);
    Value evaluateExpression(std::string variable = {});
    Array evaluateList();
    Value calculate(std::vector<Token> result);
    bool isExpressionToken(Token::Type type);
    bool isParameterSeparator(Token::Type type);
    bool isConditional(Token::Type type);
    bool isNegated();
    void setPronoun(std::string name);
    void startFunctionDeclaration(std::string name);
    Value executeFunction(std::string name);
    Operator checkOperator();

    std::unordered_map<std::string, Function> functions;

    void let();
    void put();
    void build();
    void knock();
    void rock();
    Value roll();
    Value turn();

    std::unordered_map<std::string, Value> variables;
};

#endif // EVALUATOR_H
