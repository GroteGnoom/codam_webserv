#include "conf.hpp"
#include <string>
#include <vector>

int main() {
    std::string input = "bla{}bloe;ble\nblie#comment";
    std::vector<std::string> tokens = tokenize(input);
    for (int i = 0; i < tokens.size(); i++) {
        std::cout << tokens[i] << " ";
    }
}
