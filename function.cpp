#include "function.h"
#include "evaluator.h"

Function::Function()
{
}

void Function::addToken(Token token)
{
    tokens.push_back(token);
}

int Function::args() const
{
    return parameters.size();
}

void Function::addParameter(std::string name)
{
    parameters.push_back(name);
}

Value Function::call(Array arguments)
{
    Evaluator evaluator(tokens);
    for (size_t i = 0; i < parameters.size(); i++)
    {
        evaluator.setVariable(parameters[i], arguments[i]);
    }

    return evaluator.eval();
}
