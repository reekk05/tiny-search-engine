#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <cctype>
#include <cmath>

namespace fs = std::filesystem;


// storing every pos of a word in one doc

struct Posting {
    std::string docName;
    std::vector<int> positions;
};

//full index - word - list of postings (one per doc)
using InvertedIndex = std::map<std::string, std::vector<Posting>>;

using DocSizes = std::map<std::string, int>;

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

//tf = no fo time word appeared / total words in a doc
double computeTF(int termCount, int docSize) {
    return (double)termCount / docSize;
}

//idf = log(total doc/ docs that contain the word)
//+1 on denominator to avoid division by 0
double computeIDF(int totalDocs, int docsWithTerm) {
    return std::log((double)(totalDocs+1) / (docsWithTerm+1))+1;
}

struct SearchResult {
    std::string docName;
    double score;
};

std::vector<SearchResult> search(
    const std::string& query,
    const InvertedIndex& index,
    const DocSizes& docSizes,
    int totalDocs
){
    std::vector <std::string> queryTokens=tokenize(query); //tokenize the query like the docs
    std::map<std::string, double>scores; //accumulate tf-idf scores per docs

    for(const std::string& term : queryTokens) {
        auto it = index.find(term);
        if(it==index.end()) continue; //if word not in index then skip.
        
        const std::vector<Posting>& postings=it->second;
        double idf=computeIDF(totalDocs, (int)postings.size());

        for (const Posting& posting : postings) {
            int termCount = (int)posting.positions.size();
            int docSize = docSizes.at(posting.docName);
            double tf = computeTF(termCount, docSize);
            scores[posting.docName] += tf *idf;
        }
    }
    //converting map to vector for sorting
    std::vector <SearchResult> results;
    for(const auto& [doc, score] : scores) {
        results.push_back({doc, score});
    }
    //now sorting by descending
    std::sort(results.begin(), results.end(), [](const SearchResult& a, const SearchResult& b) {
        return a.score>b.score;
    });
    return results;
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
    DocSizes docSizes;
    int totalDocs = 0;

    for(const auto& entry : fs::directory_iterator(docsPath)) {
        if(entry.path().extension() != ".txt") continue;
        std::string docName=entry.path().filename().string();
        std::string content = readFile(entry.path());
        std::vector<std::string> tokens = tokenize(content);

        buildIndex(index, docName, tokens);
        docSizes[docName] = (int)tokens.size();
        totalDocs++;
        std::cout<<"Indexed: " << docName << " (" << tokens.size()  << "tokens)\n";
        }

        std::cout << "\nTotal documents indexed: " <<totalDocs << "\n";
        std::cout << "Total unique wordds: " << index.size() << "\n\n";

        // loop searching 
        std::string query;
        while (true) {
        std::cout << "Enter search query (or 'exit' to quit): ";
        std::getline(std::cin, query);
        if (query=="exit") break;
        
        auto results= search(query, index, docSizes, totalDocs);

        if(results.empty()) {
        std::cout << "No results found.\n\n";
        }else {
        std::cout << "\nResults for \"" <<query << "\":\n";
        for (int i=0; i<(int)results.size(); i++) {
        std::cout<<i+1<<". "<<results[i].docName << "(score: " << results[i].score <<")\n";
        }
        std::cout <<"\n";
        }
    }
    return 0;
}
