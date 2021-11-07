#include "evaluator.h"
#include <iostream>
#include <stack>
#include <cassert>
#include <cmath>
#include <optional>

#define C(c) case Token::Type::c

Evaluator::Evaluator(std::vector<Token> tokens)
{
    std::vector<Token> currentLine;
    for (auto & token : tokens)
    {
        if (token.type == Token::Type::NewLine)
        {
            // keep empty lines for "end of block"
            if (currentLine.empty()) currentLine.push_back(token);

            lines.push_back(currentLine);
            currentLine.clear();
        }
        else
        {
            currentLine.push_back(token);
        }
    }

    if (currentLine.size())
    {
        lines.push_back(currentLine);
    }

    lines.push_back({ Token(Token::Type::EndOfFile, "", -1) });
}

void Evaluator::setVariable(std::string name, Value value)
{
    variables[name] = value;
}

void Evaluator::setVariable(std::string name, int index, Value value)
{
    if (!variables[name].isArray())
    {
        variables[name] = Value(Value::Special::Array);
    }

    variables[name].setIndex(index, value);
}

Value Evaluator::eval()
{
    int skippingBlocks = 0;
    int depth = 0;

    while (line < lines.size())
    {
        tokens = lines[line++];
        pc = 1;
        auto tok = tokens.front();

        if (isInFunction.size())
        {
            if (tok.type == Token::Type::NewLine)
            {
                if (depth > 0)
                {
                    depth--;
                    continue;
                }

                // an empty line finishes a function declaration
                isInFunction.clear();
            }
            else
            {
                switch (tok.type)
                {
                C(While):
                C(If):
                C(Until):
                    depth++;
                    break;
                default:
                    ;
                }

                for (auto tok : tokens)
                {
                    functions[isInFunction].addToken(tok);
                }
                continue;
            }
        }

        if (skippingBlocks)
        {
            switch (tok.type)
            {
            C(If):
            C(Until):
            C(While):
                skippingBlocks++;
                break;
            C(Else):
                if (skippingBlocks != 1)
                    break;
                [[fallthrough]];
            C(NewLine):
                skippingBlocks--;
                break;
            C(EndOfFile):
                skippingBlocks = 0;
                break;
            default:
                break;
            }

            continue;
        }

        switch (tok.type)
        {
        C(Variable):
        {
            parseVariable(tok.value);

            if (isInFunction.size()) continue;
            break;
        }
        C(NewLine):
        C(EndOfFile):
        {
            if (nextEmptyLine.size())
            {
                auto t = nextEmptyLine.top();
                nextEmptyLine.pop();

                if (t == Token::Type::While || t == Token::Type::Until)
                {
                    line = loops.top();
                    loops.pop();
                }
            }
            continue;
        }
        C(Shout):
        {
            std::cout << evaluateExpression() << '\n';
            break;
        }
        C(Let):
        {
            let();
            break;
        }
        C(Put):
        {
            put();
            break;
        }
        C(Build):
        {
            build();
            break;
        }
        C(Knock):
        {
            knock();
            break;
        }
        C(Give):
        {
            if (tokens[pc].type == Token::Type::Back)
            {
                // remove optional "back"
                pc++;
            }
            return evaluateExpression();
        }
        C(Rock):
        {
            rock();
            break;
        }
        C(Roll):
        {
            auto val = roll();

            if (tokens[pc].type == Token::Type::Into)
            {
                tok = tokens[++pc];

                if (tok.type != Token::Type::Variable)
                {
                    std::cerr << "----\n";
                    std::exit(1);
                }

                variables[tok.value] = val;

                pc++;
            }

            break;
        }
        C(Turn):
        {
            turn();
            break;
        }
        C(If):
        {
            auto b = evaluateExpression();
            if (!b.asBool())
            {
                skippingBlocks++;
            }
            else
            {
                nextEmptyLine.push(tok.type);
            }
            break;
        }
        C(Else):
        {
            skippingBlocks++;
            break;
        }
        C(Until):
        C(While):
        {
            auto res = evaluateExpression();
            if (res.asBool() == (tok.type == Token::Type::While))
            {
                loops.push(line - 1);
                nextEmptyLine.push(tok.type);
            }
            else
            {
                skippingBlocks++;
            }
            break;
        }
        default:
            std::cerr << "Unexpected token " << tok << " on line " << tok.line << '\n';
            std::exit(1);
        }

        if (pc + 1 < tokens.size())
        {
            std::cerr << "not everything has been eaten. " << pc << " " << tokens.size() << "\n";
            std::exit(1);
        }
    }

    return Value();
}

void Evaluator::parseVariable(std::string name)
{
    const auto & tok = tokens[pc++];

    switch (tok.type)
    {
    C(Is):
    {
        setPronoun(name);
        parsePoeticNumberVariable(name);
        break;
    }
    C(Says):
    {
        setPronoun(name);
        parsePoeticStringVariable(name);
        break;
    }
    C(Takes):
    {
        startFunctionDeclaration(name);
        break;
    }
    C(Taking):
    {
        executeFunction(name);
        break;
    }
    default:
        std::cerr << "Unexpected token " << tok << " after variable on line " << tok.line << '\n';
        std::exit(1);
    }
}

void Evaluator::parsePoeticNumberVariable(std::string name)
{
    const auto & tok = tokens[pc++];

    switch (tok.type)
    {
    C(Number):
    {
        auto d = std::stod(tok.value);
        setVariable(name, Value(d));
        break;
    }
    C(True):
        setVariable(name, Value(true));
        break;
    C(False):
        setVariable(name, Value(false));
        break;
    C(Null):
        setVariable(name, Value(0.0));
        break;
    C(Mysterious):
        setVariable(name, Value(Value::Special::Undefined));
        break;
    C(String):
        setVariable(name, Value(tok.value));
        break;
    default:
        std::cerr << "Unexpected token " << tok << " after 'is' on line " << tok.line << '\n';
        std::exit(1);
    }
}

void Evaluator::parsePoeticStringVariable(std::string name)
{
    const auto & tok = tokens[pc++];

    switch (tok.type)
    {
    C(String):
    {
        setVariable(name, Value(tok.value));
        break;
    }
    default:
        std::cerr << "Unexpected token " << tok << " after 'says' on line " << tok.line << '\n';
        std::exit(1);
    }
}

Value Evaluator::evaluateExpression(std::string variable)
{
    auto current = tokens[pc];

    std::vector<Token> result;
    std::stack<Token> stack;

    auto evalExpr = [&]() {
        while (!stack.empty())
        {
            result.push_back(stack.top());
            stack.pop();
        }

        auto res = calculate(result);
        result.clear();
        return res;
    };

    std::optional<bool> shortCircuitResult;

    while (isExpressionToken(current.type))
    {
        if (shortCircuitResult.has_value())
        {
            // skip the rest of the expression
            current = tokens[++pc];
            continue;
        }

        switch (current.type)
        {
        C(String):
        C(Number):
        C(True):
        C(False):
        C(Null):
        C(Not):
        C(Mysterious):
            result.push_back(current);
            break;
        C(Plus):
        C(Minus):
        {
            if (!result.size() && variable.size())
            {
                result.emplace_back(Token::Type::Variable, variable, current.line);
            }

            while (!stack.empty())
            {
                result.push_back(stack.top());
                stack.pop();
            }
            stack.push(current);
            break;
        }
        C(Times):
        C(Over):
            if (!result.size() && variable.size())
            {
                result.emplace_back(Token::Type::Variable, variable, current.line);
            }

            stack.push(current);
            break;
        C(Variable):
            result.push_back(current);
            break;
        C(Pronoun):
            result.emplace_back(Token::Type::Variable, lastVariableNamed, current.line);
            break;
        C(Taking):
        {
            if (result.size() == 0 || result.back().type != Token::Type::Variable)
            {
                if (result.size() == 0)
                {
                    std::cerr << "Unexpected 'taking', trying to call a fuction without naming it on line " << current.line << '\n';
                }
                else
                {
                    std::cerr << "Trying to call " << result.back() << " on line " << current.line << '\n';
                }
                std::exit(1);
            }
            pc++;
            auto res = executeFunction(result.back().value);
            pc--;
            pc--;
            result.pop_back();
            result.push_back(Token(res, current.line));
            break;
        }
        C(At):
        {
            if (result.size() == 0 || result.back().type != Token::Type::Variable)
            {
                std::cerr << "Unexpected 'at' on line " << current.line << '\n';
                std::exit(1);
            }

            pc++;
            auto index = evaluateExpression();

            if (!index.isDouble())
            {
                std::cerr << "An array can only be indexed with numbers, on line " << result.back().line << '\n';
                std::exit(1);
            }

            pc--;

            auto variableName = result.back().value;
            const auto & var = variables[variableName];

            auto res = var.getIndex(static_cast<int>(index.asDouble()));
            auto type = res.type();

            result.pop_back();
            result.push_back(Token(res, current.line));
            break;
        }
        C(Roll):
        {
            pc++;
            auto val = roll();
            result.push_back(Token(val, current.line));
            pc--;
            break;
        }
        C(Is):
        C(Isnt):
        {
            bool negate = current.type == Token::Type::Isnt;

            auto val = evalExpr();
            auto op = checkOperator();

            pc++;
            auto other = tokens[pc];
            auto val2 = calculate({ other });

            bool res = false;
            switch (op)
            {
            case Operator::Equal:
                res = val == val2;
                break;
            case Operator::NotEqual:
                res = val == val2;
                break;
            case Operator::GreaterOrEqual:
                res = val >= val2;
                break;
            case Operator::GreaterThan:
                res = val > val2;
                break;
            case Operator::LowerOrEqual:
                res = val <= val2;
                break;
            case Operator::LowerThan:
                res = val < val2;
                break;
            }

            if (negate) res = !res;

            result.push_back(Token(Value(res), current.line));
            break;
        }
        C(And):
        {
            auto val = evalExpr();

            if (!val.asBool())
            {
                shortCircuitResult = false;
            }
            else
            {
                pc++;
                auto res = evaluateExpression();
                shortCircuitResult = res.asBool();
            }
            break;
        }
        C(Or):
        {
            auto val = evalExpr();

            if (val.asBool())
            {
                shortCircuitResult = true;
            }
            else
            {
                pc++;
                auto res = evaluateExpression();
                shortCircuitResult = res.asBool();
            }
            break;
        }
        default:
            std::cerr << "Unexpected token " << current << " in expression on line " << current.line << '\n';
            std::exit(1);
        }

        if (pc + 1 >= tokens.size())
        {
            break;
        }
        else
        {
            current = tokens[++pc];
        }
    }

    if (shortCircuitResult.has_value())
    {
        return Value(shortCircuitResult.value());
    }
    else
    {
        return evalExpr();
    }
}

Array Evaluator::evaluateList()
{
    Array result;

    Token tok = tokens[pc];
    do
    {
        tok = tokens[pc];
        if (tok.type == Token::Type::And)
        {
            tok = tokens[++pc];
        }
        auto value = evaluateExpression();
        result.push_back(value);

        tok = tokens[pc++];
    } while (tok.type == Token::Type::Comma);
    pc--;

    return result;
}

Value Evaluator::calculate(std::vector<Token> result)
{
    bool nextIsNegated = false;
    std::stack<Value> values;
    for (auto res : result)
    {
        switch (res.type)
        {
        C(Not):
            nextIsNegated = !nextIsNegated;
            break;
        C(Number):
            values.push(Value(std::stod(res.value)));
            break;
        C(String):
            values.push(Value(res.value));
            break;
        C(True):
            values.push(Value(true));
            break;
        C(False):
            values.push(Value(false));
            break;
        C(Null):
            values.push(Value());
            break;
        C(Mysterious):
            values.push(Value(Value::Special::Undefined));
            break;
        C(Variable):
        {
            if (functions.contains(res.value))
            {
                values.push(Value(true));
            }
            else
            {
                values.push(variables[res.value]);
            }
            break;
        }
        C(Plus):
        {
            assert(values.size() >= 2);
            auto r = values.top(); values.pop();
            auto l = values.top(); values.pop();
            values.push(l + r);
            break;
        }
        C(Minus):
        {
            assert(values.size() >= 2);
            auto r = values.top(); values.pop();
            auto l = values.top(); values.pop();
            values.push(l - r);
            break;
        }
        C(Times):
        {
            assert(values.size() >= 2);
            auto r = values.top(); values.pop();
            auto l = values.top(); values.pop();
            values.push(l * r);
            break;
        }
        C(Over):
        {
            assert(values.size() >= 2);
            auto r = values.top(); values.pop();
            auto l = values.top(); values.pop();
            values.push(l / r);
            break;
        }
        default:
            throw 42;
        }

        if (nextIsNegated && res.type != Token::Type::Not)
        {
            auto v = values.top();
            values.pop();
            values.push(Value(!v.asBool()));
            nextIsNegated = false;
        }
    }

    if (values.size() != 1)
    {
        throw 69;
    }

    return Value(values.top());
}

bool Evaluator::isExpressionToken(Token::Type type)
{
    switch (type)
    {
    C(Number):
    C(Variable):
    C(String):
    C(Plus):
    C(Minus):
    C(Times):
    C(Over):
    C(True):
    C(False):
    C(Null):
    C(Pronoun):
    C(Taking):
    C(At):
    C(Roll):
    C(Turn):
    C(Is):
    C(And):
    C(Or):
    C(Not):
    C(Mysterious):
    C(Isnt):
        return true;
    default:
        return false;
    }
}

bool Evaluator::isParameterSeparator(Token::Type type)
{
    switch (type)
    {
    C(Comma):
    C(And):
        return true;
    default:
        return false;
    }
}

bool Evaluator::isConditional(Token::Type type)
{
    switch (type)
    {
    C(And):
    C(Or):
        return true;
    default:
        return false;
    }
}

bool Evaluator::isNegated()
{
    auto type = tokens[pc].type;

    if (type == Token::Type::Not)
    {
        pc++;
        return true;
    }
    return false;
}

void Evaluator::let()
{
    auto tok = tokens[pc++];
    std::string variableName;
    int arrayIndex = -1;

    switch (tok.type)
    {
    C(Variable):
    {
        variableName = tok.value;
        break;
    }
    default:
        std::cerr << "Unexpected token " << tok << ", expecting a variable after 'let' on line " << tok.line << '\n';
        std::exit(1);
    }

    setPronoun(variableName);

    if (tokens[pc].type == Token::Type::At)
    {
        pc++;

        auto res = evaluateExpression();
        if (!res.isDouble())
        {
            std::cerr << "Unexpected value " << res << ", expecting an number after 'at' on line " << tok.line << '\n';
            std::exit(1);
        }

        arrayIndex = static_cast<int>(res.asDouble());

        if (arrayIndex < 0)
        {
            std::cerr << "Invalid index " << arrayIndex << ", expecting an positive number after 'at' on line " << tok.line << '\n';
            std::exit(1);
        }
    }

    if (tokens[pc++].type != Token::Type::Be)
    {
        std::cerr << "Unexpected token " << tok << ", expecting 'be' after the variable on line " << tok.line << '\n';
        std::exit(1);
    }

    if (tokens.size() > pc + 3 && (tokens[pc + 2].type == Token::Type::Comma || tokens[pc + 3].type == Token::Type::Comma))
    {
        auto initialValue = tok;
        auto op = tokens[pc];
        bool hasValue = false;
        if (tokens[pc + 3].type == Token::Type::Comma)
        {
            initialValue = tokens[pc];
            op = tokens[pc + 1];
            hasValue = true;
        }

        pc += (hasValue ? 2 : 1);

        std::vector<Token> result;
        result.push_back(initialValue);

        auto vals = evaluateList();
        for (auto val : vals)
        {
            result.push_back(Token(val, tok.line));
            result.push_back(op);
        }

        auto res = calculate(result);
        setVariable(variableName, res);
    }
    else
    {
        auto res = evaluateExpression(variableName);

        if (arrayIndex != -1)
        {
            setVariable(variableName, arrayIndex, res);
        }
        else
        {
            setVariable(variableName, res);
        }
    }
}

void Evaluator::put()
{
    auto tok = tokens[pc++];

    Value value;
    switch (tok.type)
    {
    C(Number):
    C(Variable):
    C(String):
    {
        pc--;
        value = evaluateExpression();
        break;
    }
    default:
        std::cerr << "Unexpected token " << tok << " after 'put' on line " << tok.line << '\n';
        std::exit(1);
    }

    tok = tokens[pc++];
    if (tok.type != Token::Type::Into)
    {
        std::cerr << "Unexpected token " << tok << ", expecting 'into' after the expression on line " << tok.line << '\n';
        std::exit(1);
    }

    auto var = tokens[pc++];
    if (var.type != Token::Type::Variable)
    {
        std::cerr << "Unexpected token " << tok << ", expecting a variable after 'into' on line " << tok.line << '\n';
        std::exit(1);
    }

    setVariable(var.value, value);
    setPronoun(var.value);
}

void Evaluator::build()
{
    auto tok = tokens[pc++];

    if (tok.type != Token::Type::Variable && tok.type != Token::Type::Pronoun)
    {
        std::cerr << "Unexpected token " << tok << ", expecting a variable after 'build' on line " << tok.line << '\n';
        std::exit(1);
    }

    auto name = (tok.type == Token::Type::Pronoun ? lastVariableNamed : tok.value);

    int count = 0;
    do {
        tok = tokens[pc++];
        bool isComma = tok.type == Token::Type::Comma;
        if(tok.type != Token::Type::Up && !isComma)
        {
            std::cerr << "Unexpected token " << tok << ", expecting 'up' after a variable or others 'up' on line " << tok.line << '\n';
            std::exit(1);
        }

        if (!isComma)
        {
            count++;
        }
    } while (pc < tokens.size());


    auto v = variables[name];
    if (v.isDouble())
    {
        auto d = v.asDouble();
        d += count;
        setVariable(name, Value(d));
    }
    else if (v.isBool())
    {
        auto b = v.asBool();
        if (count % 2)
        {
            b = !b;
        }
        setVariable(name, Value(b));
    }
    else
    {
        std::cerr << "You can't increment a variable that is not a number or a boolean, on line " << tok.line << '\n';
        std::exit(1);
    }

    pc--;
}

void Evaluator::knock()
{
    auto tok = tokens[pc++];

    if(tok.type != Token::Type::Variable)
    {
        std::cerr << "Unexpected token " << tok << ", expecting a variable after 'knock' on line " << tok.line << '\n';
        std::exit(1);
    }

    auto name = tok.value;

    int count = 0;
    do {
        tok = tokens[pc++];
        bool isComma = tok.type == Token::Type::Comma;
        if(tok.type != Token::Type::Down && !isComma)
        {
            std::cerr << "Unexpected token " << tok << ", expecting 'down' after a variable or others 'down' on line " << tok.line << '\n';
            std::exit(1);
        }

        if (!isComma)
        {
            count++;
        }
    } while (pc < tokens.size());

    auto v = variables[name];
    if (v.isDouble())
    {
        auto d = v.asDouble();
        d -= count;
        setVariable(name, Value(d));
    }
    else if (v.isBool())
    {
        auto b = v.asBool();
        if (count % 2)
        {
            b = !b;
        }
        setVariable(name, Value(b));
    }
    else
    {
        std::cerr << "You can't decrement a variable that is not a number or a boolean, on line " << tok.line << '\n';
        std::exit(1);
    }

    pc--;
}

void Evaluator::rock()
{
    auto tok = tokens[pc++];
    setPronoun(tok.value);

    if(tok.type != Token::Type::Variable)
    {
        std::cerr << "Unexpected token " << tok << ", expecting a variable after 'rock' on line " << tok.line << '\n';
        std::exit(1);
    }

    auto & var = variables[tok.value];
    if (!var.isArray())
    {
        var = Value(Value::Special::Array);
    }

    if (tokens[pc].type == Token::Type::Plus)
    {
        pc++;
        auto values = evaluateList();

        for (auto val : values)
        {
            var.push(val);
        }
    }
    else if (tokens[pc].type == Token::Type::Like)
    {
        tok = tokens[++pc];
        var.push(Value(std::stod(tok.value)));
        pc++;
    }
}

Value Evaluator::roll()
{
    auto tok = tokens[pc++];
    setPronoun(tok.value);

    if (tok.type != Token::Type::Variable)
    {
        std::cerr << "Unexpected token " << tok << ", expecting a variable after 'roll' on line " << tok.line << '\n';
        std::exit(1);
    }

    auto & var = variables[tok.value];
    if (!var.isArray())
    {
        std::cerr << "Can't roll from a " << var.type() << ", only from an array on line " << tok.line << '\n';
        std::exit(1);
    }

    return var.pop();
}

Value Evaluator::turn()
{
    auto op = tokens[pc++];

    if (op.type != Token::Type::Up && op.type != Token::Type::Down)
    {
        std::cerr << "Unexpected token " << op << ", expecting 'up' or 'down' after 'turn' on line " << op.line << '\n';
        std::exit(1);
    }

    auto tok = tokens[pc++];
    setPronoun(tok.value);

    if (tok.type != Token::Type::Variable)
    {
        std::cerr << "Unexpected token " << tok << ", expecting a variable after 'turn " << op.value << "' on line " << tok.line << '\n';
        std::exit(1);
    }

    auto & var = variables[tok.value];
    if (!var.isDouble())
    {
        std::cerr << "You can 'turn " << op.value << "' only a number, got a " << var.type() << ", on line " << tok.line << '\n';
        std::exit(1);
    }

    auto d = var.asDouble();
    switch (op.type)
    {
    C(Up):
        d = std::ceil(d);
        break;
    C(Down):
        d = std::floor(d);
        break;
    default:
        std::cerr << "Unexpected error. Got " << op << " instead of 'up' or 'down' on line " << op.line << '\n';
        std::exit(1);
    }

    variables[tok.value] = Value(d);
    return Value(d);
}

void Evaluator::setPronoun(std::string name)
{
    lastVariableNamed = name;
}

void Evaluator::startFunctionDeclaration(std::string name)
{
    isInFunction = name;

    Function func;

    auto tok = tokens[pc++];
    if(tok.type != Token::Type::Variable)
    {
        std::cerr << "Unexpected token " << tok << ", expecting a variable since a function requires at least one parameter on line " << tok.line << '\n';
        std::exit(1);
    }
    func.addParameter(tok.value);

    while (pc < tokens.size())
    {
        tok = tokens[pc++];
        if (isParameterSeparator(tok.type))
        {
            std::cerr << "Unexpected token " << tok << " on line " << tok.line << '\n';
            std::exit(1);
        }

        tok = tokens[pc++];
        if(tok.type != Token::Type::Variable)
        {
            std::cerr << "Unexpected token " << tok << ", function expects variable as parameters on line " << tok.line << '\n';
            std::exit(1);
        }
        func.addParameter(tok.value);
    }

    functions[name] = func;
}

Value Evaluator::executeFunction(std::string name)
{
    auto & func = functions[name];
    Array arguments;
    for (int i = 0; i < func.args(); i++)
    {
        auto t = tokens[pc].type;
        arguments.push_back(evaluateExpression());

        t = tokens[pc++].type;
        switch (t)
        {
        C(Comma):
            // skip
            break;
        default:
            if (i + 1 < func.args())
            {
                throw 42;
            }
        }
    }

    return func.call(arguments);
}

Operator Evaluator::checkOperator()
{
    auto op = tokens[pc + 1].type;
    auto opType = Operator::Equal;

    if (op == Token::Type::As)
    {
        pc++;
        auto op2 = tokens[++pc];

        switch (op2.type)
        {
        C(Great):
            opType = Operator::GreaterOrEqual;
            break;
        C(Little):
            opType = Operator::LowerOrEqual;
            break;
        default:
            std::cerr << "Unexpected token " << op2 << " after 'as' on line " << op2.line << '\n';
            std::exit(1);
        }

        auto secondAs = tokens[++pc];
        if (secondAs.type != Token::Type::As)
        {
            std::cerr << "Unexpected token " << secondAs << ", expecting 'as' after '" << op2 << "' on line " << op2.line << '\n';
            std::exit(1);
        }
    }
    else if (op == Token::Type::Not)
    {
        opType = Operator::NotEqual;
    }
    else if (op == Token::Type::Greater || op == Token::Type::Lower)
    {
        pc++;
        if (op == Token::Type::Greater) opType = Operator::GreaterThan;
        else if (op == Token::Type::Lower) opType = Operator::LowerThan;

        auto secondAs = tokens[++pc];
        if (secondAs.type != Token::Type::Than)
        {
            std::cerr << "Unexpected token " << secondAs << ", expecting 'than' on line " << secondAs.line << '\n';
            std::exit(1);
        }
    }

    return opType;
}
