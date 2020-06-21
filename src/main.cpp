//#include "../include/Enact.h"
#include "../lib/parser/Lexer.h"

int main(int argc, char *argv[]) {
    //enact::Options options{argc, argv};
    //enact::Context context{options};

    //return static_cast<int>(context.run());

    while (true) {
        std::cout << "enact > ";
        std::string input;
        std::getline(std::cin, input);

        enact::Lexer lexer{std::move(input)};
        enact::Token token = lexer.scanToken();
        while (token.type != enact::TokenType::END_OF_FILE) {
            std::cout << token.lexeme << '\n';
            token = lexer.scanToken();
        }
    }
}