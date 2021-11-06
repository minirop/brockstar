#include "scanner.h"
#include <algorithm>
#include <boost/algorithm/string/split.hpp>
#include <iostream>
#include <utf8proc.h>

using namespace std::string_literals;

Scanner::Scanner(std::string source)
{
    std::vector<std::string> words;

    std::string preSource;
    for (size_t i = 0; i < source.size(); i++)
    {
        auto c = source[i];
        if (c == '(')
        {
            while (c != ')')
            {
                c = source[++i];
            }
        }
        else
        {
            preSource += c;
        }
    }

    source = preSource;

    for (size_t i = 0; i < source.size();)
    {
        auto c = source[i];
        std::string word;

        if (isIdentifier(c))
        {
            while (isIdentifier(c))
            {
                word += c;
                c = source[++i];
            }

            words.emplace_back(word);
        }
        else if (isNumber(c))
        {
            while (isNumber(c))
            {
                word += c;
                c = source[++i];
            }

            words.emplace_back(word);
        }
        else if (c == '"')
        {
            do
            {
                word += c;
                c = source[++i];
            } while (c != '"');
            word += c;
            i++;

            words.emplace_back(word);
        }
        else if (c == '\n')
        {
            words.push_back("\n");
            i++;
        }
        else if (c == ',')
        {
            words.push_back(",");
            i++;
        }
        // skipped characters like spaces
        else
        {
            i++;
        }
    }

    int lineIndex = 1;

    std::vector<Token> preTokens;
    auto isString = false;
    std::string String;
    for (auto word : words)
    {
        auto type = Token::Type::Identifier;

        if (isString)
        {
            String += " " + word;
            if (!word.ends_with("\""))
            {
                continue;
            }

            type = Token::Type::String;
            word = String;
            removeQuotes(word);
            preTokens.emplace_back(type, word, lineIndex);
            isString = false;
            continue;
        }

        if (isArticle(word))
        {
            type = Token::Type::Article;
        }
        else if (isPronoun(word))
        {
            type = Token::Type::Pronoun;
        }
        else if (isKeyword(word))
        {
            type = Token::Type::Keyword;
        }
        else if (word.starts_with("\""))
        {
            if (!word.ends_with("\""))
            {
                String += word;
                isString = true;
                continue;
            }

            removeQuotes(word);
            type = Token::Type::String;
        }
        else if (word.size() == 0)
        {
            continue;
        }
        else if (word == "\n")
        {
            type = Token::Type::NewLine;
            lineIndex++;
        }
        else if (word == ",")
        {
            type = Token::Type::Comma;
        }
        else if (std::all_of(begin(word), end(word), [this](auto c) { return isNumber(c); }))
        {
            type = Token::Type::Number;
        }

        preTokens.emplace_back(type, word, lineIndex);
    }

    auto isProperVariable = false;
    auto mergeNextWord = false;
    auto isPoeticNumberLiteral = false;
    auto isPoeticStringLiteral = false;
    bool hadPeriod = false;
    for (int i = 0; auto tok : preTokens)
    {
        if (tok.type == Token::Type::NewLine)
        {
            isPoeticNumberLiteral = false;
            isPoeticStringLiteral = false;
            hadPeriod = false;
        }

        if (isPoeticNumberLiteral)
        {
            if (tok.type != Token::Type::Comma)
            {
                if (tok.type == Token::Type::Number)
                {
                    if (!hadPeriod)
                    {
                        if (tok.value.find('.') != std::string::npos)
                        {
                            tokens.back().value += '.';
                            hadPeriod = true;
                        }
                    }
                }
                else
                {
                    if (tokens.back().type == Token::Type::Number)
                    {
                        tokens.back().value += std::to_string(poeticNumberLiteralCount(tok.value));
                    }
                    else
                    {
                        tokens.emplace_back(Token::Type::Number, std::to_string(poeticNumberLiteralCount(tok.value)), tok.line);
                    }
                }
            }
        }
        else if (isPoeticStringLiteral)
        {
            if (tokens.back().type == Token::Type::String)
            {
                tokens.back().value += " " + tok.value;
            }
            else
            {
                tokens.emplace_back(Token::Type::String, tok.value, lineIndex);
            }
        }
        else if (!mergeNextWord && tok.type == Token::Type::Identifier && std::isupper(tok.value[0]))
        {
            if (isProperVariable)
            {
                properVariableCase(tok.value);
                tokens.back().value += " " + tok.value;
            }

            if (preTokens[i + 1].type == Token::Type::Identifier)
            {
                if (!std::isupper(preTokens[i + 1].value[0]))
                {
                    std::cerr << "Invalid proper variable on line " << preTokens[i + 1].line << '\n';
                    std::exit(1);
                }

                if (!isProperVariable)
                {
                    isProperVariable = true;
                    properVariableCase(tok.value);
                    tok.type = Token::Type::Variable;
                    tokens.push_back(tok);
                }
            }
            else
            {
                if (!isProperVariable)
                {
                    lowerCase(tok.value);
                    tok.type = Token::Type::Variable;
                    tokens.push_back(tok);
                }

                isProperVariable = false;
            }
        }
        else if (tok.type == Token::Type::Identifier)
        {
            auto name = tok.value;
            bool nextIsIs = false;
            if (name.ends_with("'s") || name.ends_with("'re"))
            {
                name = name.substr(0, name.find('\''));
                tok.value = name;
                nextIsIs = true;
            }
            else
            {
                name.erase (std::remove(name.begin(), name.end(), '\''), name.end());
            }

            if (mergeNextWord)
            {
                lowerCase(tok.value);
                tokens.back().value += " " + tok.value;
                mergeNextWord = false;
            }
            else
            {
                tok.type = Token::Type::Variable;
                tokens.push_back(tok);
            }

            if (nextIsIs)
            {
                tokens.push_back(Token(Token::Type::Is, "is", tok.line));

                auto t = preTokens[i + 1].type;
                isPoeticNumberLiteral = (t != Token::Type::Number);

                if (t == Token::Type::Keyword)
                {
                    auto value = preTokens[i + 1].value;
                    auto kType = convertKeyword(value);
                    if (kType == Token::Type::Null || kType == Token::Type::Mysterious || kType == Token::Type::Not)
                    {
                        isPoeticNumberLiteral = false;
                    }
                }
            }
        }
        else if (tok.type == Token::Type::Article)
        {
            mergeNextWord = true;
            lowerCase(tok.value);
            tok.type = Token::Type::Variable;
            tokens.push_back(tok);

            if (preTokens[i + 1].type != Token::Type::Identifier)
            {
                std::cerr << "Unterminated common variable, got token " << preTokens[i + 1] << " on line " << lineIndex << '\n';
                std::exit(1);
            }
        }
        else
        {
            if (tok.type == Token::Type::Keyword)
            {
                tok.type = convertKeyword(tok.value);
            }
            tokens.push_back(tok);

            if (tok.type == Token::Type::Is)
            {
                auto t = preTokens[i + 1].type;
                isPoeticNumberLiteral = (t != Token::Type::Number);

                if (t == Token::Type::Keyword)
                {
                    auto value = preTokens[i + 1].value;
                    auto kType = convertKeyword(value);
                    if (kType == Token::Type::Null || kType == Token::Type::Mysterious || kType == Token::Type::Not)
                    {
                        isPoeticNumberLiteral = false;
                    }
                }
            }
            else if (tok.type == Token::Type::Says)
            {
                isPoeticStringLiteral = true;
            }
            else if (tok.type == Token::Type::Like)
            {
                isPoeticNumberLiteral = true;
            }
        }

        i++;
    }

    /*for (auto t : tokens)
    {
        std::cerr << t << '\n';
    }*/
}

std::vector<Token> Scanner::getTokens()
{
    return tokens;
}

bool Scanner::isNumber(int c)
{
    return isdigit(c) || c == '.' || c == '+' || c == '-';
}

bool Scanner::isIdentifier(int c)
{
    return isalpha(c) || c == '\'';
}

static const std::vector<std::string> pronouns = {
    "it",
    "he",
    "she",
    "him",
    "her",
    "they",
    "them",
    "ze",
    "hir",
    "zie",
    "zir",
    "xe",
    "xem",
    "ve",
    "ver",
};
bool Scanner::isPronoun(std::string & word)
{
    auto s = word;
    lowerCase(s);
    bool found = std::count(begin(pronouns), end(pronouns), s) > 0;
    if (found)
    {
        word = s;
    }
    return found;
}

static const std::vector<std::string> articles = {
    "a", "an", "the", "my", "your"
};
bool Scanner::isArticle(std::string & word)
{
    auto s = word;
    lowerCase(s);
    bool found = std::count(begin(articles), end(articles), s) > 0;
    if (found)
    {
        word = s;
    }
    return found;
}

static const std::vector<std::string> keywords = {
    "is", "into", "put", "shout", "plus", "minus", "times", "over",
    "says", "true", "false", "null", "knock", "down", "build", "up",
    "let", "be", "and", "whisper", "takes", "taking", "give", "back",
    "at", "rock", "like", "roll", "turn", "mysterious", "if", "while",
    "else", "or", "until", "not", "isnt", "is",
};
bool Scanner::isKeyword(std::string & word)
{
    auto s = word;
    lowerCase(s);
    checkIfAlias(s);
    bool found = std::count(begin(keywords), end(keywords), s) > 0;
    if (found)
    {
        word = s;
    }
    return found;
}

void Scanner::checkIfAlias(std::string & word)
{
    if (word == "are" || word == "were" || word == "was") word = "is";
    else if (word == "say" || word == "whisper") word = "shout";
    else if (word == "with") word = "plus";
    else if (word == "without") word = "minus";
    else if (word == "of") word = "times";
    else if (word == "between") word = "over";
    else if (word == "nothing" || word == "gone" || word == "nowhere" || word == "nobody") word = "null";
    else if (word == "wrong" || word == "no" || word == "lies") word = "false";
    else if (word == "right" || word == "yes" || word == "ok") word = "true";
    else if (word == "wants") word = "takes";
    else if (word == "return") word = "give";
}

void Scanner::properVariableCase(std::string & word)
{
    lowerCase(word);
    word[0] = std::toupper(word[0]);
}

void Scanner::lowerCase(std::string & word)
{
    std::transform(begin(word), end(word), begin(word), tolower);
}

void Scanner::removeQuotes(std::string & word)
{
    word.erase(0, 1);
    word.erase(word.size() - 1, 1);
}

int Scanner::poeticNumberLiteralCount(std::string word)
{
    auto c = std::count_if(begin(word), end(word), [](char c) { return isalpha(c) || c == '-'; });
    return c % 10;
}

Token::Type Scanner::convertKeyword(std::string name)
{
    properVariableCase(name);

#define P(k) if (name == #k) return Token::Type::k
    P(Into);
    P(Is);
    P(Shout);
    P(Plus);
    P(Minus);
    P(Times);
    P(Over);
    P(Says);
    P(True);
    P(False);
    P(Let);
    P(Put);
    P(Be);
    P(Null);
    P(Knock);
    P(Down);
    P(Build);
    P(Up);
    P(And);
    P(Takes);
    P(Taking);
    P(Give);
    P(At);
    P(Rock);
    P(Like);
    P(Roll);
    P(Turn);
    P(Mysterious);
    P(Back);
    P(If);
    P(Else);
    P(While);
    P(Until);
    P(Or);
    P(Not);
    P(Isnt);
#undef P

    return Token::Token::Identifier;
}
