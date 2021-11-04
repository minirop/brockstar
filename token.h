#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include "value.h"

class Token
{
public:
    enum Type {
        Article,
        Pronoun,
        Keyword,
        Identifier,
        String,
        Number,
        Variable,
        Is,
        Into,
        Plus,
        Minus,
        Times,
        Over,
        True,
        False,
        Says,
        Shout,
        Let,
        Put,
        Be,
        Null,
        NewLine,
        Knock,
        Down,
        Build,
        Up,
        Comma,
        And,
        Whisper,
        Takes,
        Taking,
        Give,
        Back,
        At,
        Mysterious,
        Rock,
        Like,
        Roll,
        EndofFile,
    };

    Token(Type type, std::string value, int line);
    Token(Value value, int line);

    Type type;
    std::string value;
    int line;
};

std::ostream& operator<<(std::ostream& os, const Token& t);

#endif // TOKEN_H
