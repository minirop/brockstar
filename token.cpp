#include "token.h"
#include <ostream>

Token::Token(Type type, std::string value, int line)
    : type { type }, value { value }, line { line }
{
}

Token::Token(Value value, int line)
    : line { line }
{
    if (value.isDouble())
    {
        type = Token::Type::Number;
        this->value = format(value.asDouble());
    }
    else if (value.isString())
    {
        type = Token::Type::String;
        this->value = value.asString();
    }
    else if (value.isNull())
    {
        type = Token::Type::Null;
        this->value = value.asString();
    }
    else if (value.isUndefined())
    {
        type = Token::Type::Mysterious;
        this->value = value.asString();
    }
    else if (value.isBool())
    {
        if (value.asBool()) type = Token::Type::True;
        else type = Token::Type::False;
        this->value = value.asString();
    }
    else
    {
        throw 42;
    }
}

static const char* tokens_names[] = {
    "Article",
    "Pronoun",
    "Keyword",
    "Identifier",
    "String",
    "Number",
    "Variable",
    "Is",
    "Into",
    "Plus",
    "Minus",
    "Times",
    "Over",
    "True",
    "False",
    "Says",
    "Shout",
    "Let",
    "Put",
    "Be",
    "Null",
    "NewLine",
    "Knock",
    "Down",
    "Build",
    "Up",
    "Comma",
    "And",
    "Whisper",
    "Takes",
    "Taking",
    "Give",
    "Back",
    "At",
    "Mysterious",
    "Rock",
    "Like",
    "Roll",
    "Turn",
    "If",
    "Else",
    "While",
    "Until",
    "Or",
    "Not",
    "Isnt",
    "Greater",
    "Lower",
    "As",
    "Great",
    "Little",
    "Than",
    "Nor",
    "Break",
    "Continue",
};
std::ostream& operator<<(std::ostream& os, const Token& t)
{
    os << tokens_names[t.type];
    if (t.type == Token::Type::Variable)
    {
        os << " (" << t.value << ")";
    }
    return os;
}
