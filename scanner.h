#ifndef SCANNER_H
#define SCANNER_H

#include <vector>
#include "token.h"
#include <string>

class Scanner
{
public:
    Scanner(std::string source);

    std::vector<Token> getTokens();

private:
    std::vector<Token> tokens;

    bool isNumber(int c);
    bool isIdentifier(int c);
    bool isPronoun(std::string & word);
    bool isArticle(std::string & word);
    bool isKeyword(std::string & word);
    void checkIfAlias(std::string & word);
    void properVariableCase(std::string & word);
    void lowerCase(std::string & word);
    void removeQuotes(std::string & word);
    int poeticNumberLiteralCount(std::string word);
    Token::Type convertKeyword(std::string name);
};

#endif // SCANNER_H
