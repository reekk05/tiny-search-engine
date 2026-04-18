#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <cctype>

namespace fs = std::filesystem;

//read the file
std::string readFile(const fs::path& filePath) {
    std::ifstream file(filePath);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// takes raw texts ad returns lower case words
std::vector<std::string> tokenize(const std::string& text) {
    std::vector<std::string> tokens;
    std::string word;

    for(char ch:text) {
        if (std::isalpha(ch)) {
            word += std::tolower(ch);
        } else {
            if (!word.empty()) {
                tokens.push_back(word);
                word.clear();
            }
        }
    }

    if (!word.empty()) tokens.push_back(word);

    return tokens;
}
int main() {
    fs::path docsPath = "../docs";

    std::cout << "Looking for docs at: " << fs::absolute(docsPath) << std::endl;
    std::cout<< "Exists: " << fs::exists(docsPath) <<std::endl;

    if(!fs::exists(docsPath)) {
        std::cerr << "docs/ folder not found" << std::endl;
        return 1;
    }

    for(const auto& entry : fs::directory_iterator(docsPath)) {
        if(entry.path().extension() != ".txt") continue;

        std::string content = readFile(entry.path());
        std::vector<std::string> tokens = tokenize(content);

        std::cout << "File:" << entry.path().filename() << std::endl;
        std::cout << "Token count:" << tokens.size() << std::endl;
        std::cout << "first 5 tokens: ";
        for(int i=1; i < 5 && i<tokens.size(); i++) {
            std::cout<<tokens[i] << " ";
        }
        std::cout << "\n\n";
    }
    return 0;
}
