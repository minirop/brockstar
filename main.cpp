#include <iostream>
#include <fstream>
#include <string>
#include "scanner.h"
#include "evaluator.h"

void runFile(std::string filename);
void run(std::string string);

int main(int argc, char** argv)
{
    switch (argc)
    {
    case 1:
        runFile("demo.rock");
        break;
    case 2:
        runFile(argv[1]);
        break;
    default:
        std::cerr << "Usage: rockstar [script.rock]";
        break;
    }

    return 0;
}

void runFile(std::string filename)
{
    std::ifstream ifs { filename };
    std::string content { std::istreambuf_iterator<char>(ifs),
                          std::istreambuf_iterator<char>() };
    run(content);
}

void run(std::string string)
{
    Scanner scanner(string);
    Evaluator evaluator(scanner.getTokens());
    evaluator.eval();
}
