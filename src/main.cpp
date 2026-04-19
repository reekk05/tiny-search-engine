#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <cctype>

namespace fs = std::filesystem;


// storing every pos of a word in one doc

struct Posting {
    std::string docName;
    std::vector<int> positions;
};

//full index - word - list of postings (one per doc)
using InvertedIndex = std::map<std::string, std::vector<Posting>>;

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

void buildIndex(InvertedIndex& index, const std::string& docName, const std::vector<std::string>&tokens) {
    for (int i=0; i<tokens.size(); i++){
        const std::string& word = tokens[i];

        //to see if the doc already have a posting for this word
        auto& postings = index[word];
        auto it = std::find_if(postings.begin(), postings.end(), [&](const Posting& p) {
            return p.docName==docName;
        });
        if(it==postings.end()) {
            postings.push_back({docName, {i}}); // (first time we seeing word)
        } else {
            it->positions.push_back(i); //word already seen, just asigning pos here
        }
    }
}

void printIndex(const InvertedIndex& index) {
    for (const auto& [word, postings]: index){
        std::cout <<word<<":\n";
        for(const auto& posting:postings) {
            std::cout << " " << posting.docName << " -> positions: ";
            for (int pos : posting.positions) {
                std::cout << pos << " ";
            }
            std::cout << "\n";
        }
    }
}
int main() {
    fs::path docsPath = "../docs";

    if(!fs::exists(docsPath)) {
        std::cerr << "docs/ folder not found" << std::endl;
        return 1;
    }

    InvertedIndex index;

    for(const auto& entry : fs::directory_iterator(docsPath)) {
        if(entry.path().extension() != ".txt") continue;
        std::string docName=entry.path().filename().string();
        std::string content = readFile(entry.path());
        std::vector<std::string> tokens = tokenize(content);

        buildIndex(index, docName, tokens);
        std::cout<<"Indexed: " << docName << " (" << tokens.size()  << "tokens)\n";

        }
        std::cout << "\n---INVERTED INDEX---\n\n";
        printIndex(index);

    return 0;
}
