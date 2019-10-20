#include<iostream>
#include<fstream>
#include<string.h>
#include <algorithm>

using namespace std;

#include <string>
#include <set>
#include <vector>
#include <queue>

#define ALPHABET_SIZE 26

typedef struct trieNode TrieNode;

struct trieNode {
    bool validWord = false;
    bool noFurtherWords = true;
    TrieNode* nodeLetters[ALPHABET_SIZE];

    trieNode() {
        memset(nodeLetters, 0, sizeof(TrieNode*) * ALPHABET_SIZE);
    }
};

class Trie {
 private:
    TrieNode* root;

    void insertWord(const string& word) {
        TrieNode* currentNode = root;
        for (auto ch : word) {
            char normalizedCh = tolower(ch);
            unsigned index = normalizedCh - 'a';
            if (currentNode->nodeLetters[index] != NULL) {
                currentNode = currentNode->nodeLetters[index];
            } else {
                currentNode->nodeLetters[index] = new TrieNode();
                currentNode->noFurtherWords = false;
                currentNode = currentNode->nodeLetters[index];
            }
        }
        // Word found.
        currentNode->validWord = true;
    }

    void printTrie(std::string s, const TrieNode* node) const {
        if (node->validWord) std::cout << s << std::endl;
        for (unsigned i = 0; i < ALPHABET_SIZE; i++) {
            if (node->nodeLetters[i] == NULL) continue;
            char ch = i + 'a';
            printTrie(s + ch, node->nodeLetters[i]); 
        }
    }

 public:
    Trie() {
        root = new TrieNode();
        ifstream file;
        file.open("wordlist/wordlist.txt");
        if (!file.is_open()) return;
        string word;
        while (file >> word) {
            bool someNonAlphaNum = std::any_of(word.begin(), word.end(),
                    [](char c) { return !isalpha(c); } );
            if (someNonAlphaNum) {
                // std::cout << " Found word with non alphanumeric character: " << word <<
                //    std::endl;
                continue;
            }
            insertWord(word);
        }
    }

    bool count(const std::string& str) const {
        TrieNode* currentNode = root;
        for (auto ch : str) {
            unsigned index = ch - 'a';
            if (currentNode->nodeLetters[index] == NULL) return false;
            currentNode = currentNode->nodeLetters[index];
        }
        return currentNode->validWord;
    }

    bool validPath(const std::string& s) const {
        TrieNode* currentNode = root;
        for (auto ch : s) {
            unsigned index = ch - 'a';
            if (currentNode->nodeLetters[index] == NULL) return false;
            currentNode = currentNode->nodeLetters[index];
        }
        return !currentNode->noFurtherWords;
    }
};

class SpellingBee {
 private:
    static constexpr int MAX_LENGTH = 26;
    const std::vector<char>& letters;
    Trie dictionary;

 public:
    SpellingBee(std::vector<char>& l) : letters(l) { }

    set<string> getAllWords() const {
        set<string> rv;
        queue<string> queuedWords;
        for (auto ch : letters) queuedWords.push(string(1, ch));
        while (!queuedWords.empty()) {
            auto top = queuedWords.front();
            queuedWords.pop();
            if (top.length() > MAX_LENGTH) break;
            for (auto ch : letters) {
                auto newWord = top + ch;
                if (newWord.length() < MAX_LENGTH && dictionary.validPath(newWord))
                    queuedWords.push(newWord);
                if (newWord.length() < 4) continue;
                std::size_t found = newWord.find(letters[0]);
                if (found == std::string::npos) continue;
                if (dictionary.count(newWord)) rv.insert(newWord);
            }
        }
        return rv;
    }
};

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << " Too few arguments provided" << std::endl;
        return 1;
    }

    std::vector<char> input;
    int i = 0;
    while (char ch = argv[1][i++]) {
        if (!isalpha(ch)) {
            std::cerr << " Invalid input: nonalphabetic character " << ch << " input." << std::endl;
            return 1;
        }
        input.push_back(tolower(ch));
    }
    if (i != 8) {
        std::cerr << " Exactly 7 characters required for spelling bee" <<
            std::endl;
        return 1;
    }
    SpellingBee spellBee(input);
    auto words = spellBee.getAllWords();
    std::cout << "Number of words found: " << words.size() << std::endl;
    unsigned totalScore = 0;
    for (auto& word : words) {
        auto thisWordScore = word.length();
        if (word.length() >= input.size()) {
            std::set<char> pangramCheck;
            for (auto ch : word) pangramCheck.insert(ch);
            if (pangramCheck.size() == input.size())
                thisWordScore += input.size();
        }
        std::cout << " " << word << " (" << thisWordScore << ")" << std::endl;
        totalScore += thisWordScore;
    }
    std::cout << "Total score: " << totalScore << std::endl;
    return 0;
}
