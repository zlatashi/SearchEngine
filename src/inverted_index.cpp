#include "inverted_index.h"
#include <sstream>
#include <future>
#include <map>

void InvertedIndex::updateDocumentBase(const std::vector<std::string>& input_docs) {
    freq_dictionary.clear();

    std::vector<std::future<std::map<std::string, size_t>>> futures;

    for (const auto& doc : input_docs) {
        futures.push_back(std::async(std::launch::async, [&doc]() {
            std::map<std::string, size_t> word_count;
            std::istringstream ss(doc);
            std::string word;
            while (ss >> word) {
                ++word_count[word];
            }
            return word_count;
            }));
    }

    for (size_t doc_id = 0; doc_id < futures.size(); ++doc_id) {
        auto word_count = futures[doc_id].get();
        for (const auto& [word, count] : word_count) {
            std::lock_guard<std::mutex> lock(_dictionary_mtx);
            freq_dictionary[word][doc_id] = count;
        }
    }
}

std::vector<Entry> InvertedIndex::getWordCount(const std::string& word) {
    std::vector<Entry> result;

    std::lock_guard<std::mutex> lock(_dictionary_mtx);
    if (freq_dictionary.count(word)) {
        for (const auto& [doc_id, count] : freq_dictionary[word]) {
            result.emplace_back(doc_id, count);
        }
    }
    return result;
}

void InvertedIndex::updateDocument(const std::string& word, size_t doc_id) {
    std::lock_guard<std::mutex> lock(_dictionary_mtx);
    ++freq_dictionary[word][doc_id];
}
