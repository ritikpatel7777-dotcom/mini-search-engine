#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <iomanip>
#include <cctype>
#include <filesystem>

using namespace std;

class Document {
private:
    int id;
    string fileName;
    string content;

public:
    Document(int i, const string& name, const string& text)
        : id(i), fileName(name), content(text) {}

    int getId() const { return id; }
    string getFileName() const { return fileName; }
    string getContent() const { return content; }
};

struct SearchResult {
    int documentId;
    string fileName;
    int score;
};

class MiniSearchEngine {
private:
    vector<Document> documents;
    unordered_map<string, vector<int>> invertedIndex;

    string cleanWord(const string& word) const {
        string result;
        for (unsigned char ch : word) {
            if (isalnum(ch)) {
                result += static_cast<char>(tolower(ch));
            }
        }
        return result;
    }

    vector<string> tokenize(const string& text) const {
        vector<string> words;
        string word;
        stringstream ss(text);

        while (ss >> word) {
            word = cleanWord(word);
            if (!word.empty()) words.push_back(word);
        }
        return words;
    }

    int countKeyword(const string& content, const string& keyword) const {
        int count = 0;
        for (const string& word : tokenize(content)) {
            if (word == keyword) ++count;
        }
        return count;
    }

    vector<SearchResult> calculateResults(const string& query) const {
        vector<SearchResult> results;
        vector<string> keywords = tokenize(query);
        unordered_map<int, int> scores;

        for (const string& keyword : keywords) {
            auto it = invertedIndex.find(keyword);
            if (it == invertedIndex.end()) continue;

            for (int docId : it->second) {
                scores[docId] += countKeyword(
                    documents[docId].getContent(), keyword
                );
            }
        }

        for (const auto& [docId, score] : scores) {
            results.push_back({
                docId,
                documents[docId].getFileName(),
                score
            });
        }

        sort(results.begin(), results.end(),
             [](const SearchResult& a, const SearchResult& b) {
                 if (a.score != b.score) return a.score > b.score;
                 return a.fileName < b.fileName;
             });

        return results;
    }

public:
    bool addDocument(const string& fileName) {
        ifstream file(fileName);
        if (!file) {
            cout << "Error: Cannot open file: " << fileName << "\n";
            return false;
        }

        string content, line;
        while (getline(file, line)) content += line + " ";
        file.close();

        int id = static_cast<int>(documents.size());
        documents.emplace_back(id, fileName, content);

        for (const string& word : tokenize(content)) {
            auto& docList = invertedIndex[word];
            if (find(docList.begin(), docList.end(), id) == docList.end())
                docList.push_back(id);
        }

        cout << "Document indexed successfully: " << fileName << "\n";
        return true;
    }

    void displayDocuments() const {
        if (documents.empty()) {
            cout << "No documents indexed.\n";
            return;
        }

        cout << "\nIndexed Documents\n";
        cout << "-----------------\n";
        for (const auto& doc : documents) {
            cout << "[" << doc.getId() << "] " << doc.getFileName() << "\n";
        }
    }

    void search(const string& query, bool saveToFile = false) const {
        if (documents.empty()) {
            cout << "No documents available for search.\n";
            return;
        }

        vector<SearchResult> results = calculateResults(query);

        cout << "\n============================================\n";
        cout << "              SEARCH RESULTS\n";
        cout << "============================================\n";
        cout << "Query: " << query << "\n\n";

        if (results.empty()) {
            cout << "No matching documents found.\n";
            return;
        }

        cout << left << setw(8) << "Rank"
             << setw(10) << "Doc ID"
             << setw(30) << "Document"
             << setw(10) << "Score" << "\n";
        cout << string(58, '-') << "\n";

        int rank = 1;
        for (const auto& result : results) {
            cout << left << setw(8) << rank++
                 << setw(10) << result.documentId
                 << setw(30) << result.fileName
                 << setw(10) << result.score << "\n";
        }

        cout << "============================================\n";

        if (saveToFile) {
            ofstream out("search_results.txt", ios::app);
            if (out) {
                out << "\nQuery: " << query << "\n";
                for (const auto& result : results)
                    out << result.fileName << " | Score: "
                        << result.score << "\n";
                out << "--------------------------------------------\n";
                cout << "Search result saved to search_results.txt\n";
            } else {
                cout << "Warning: Could not save search result.\n";
            }
        }
    }

    void saveSearchResult(const string& query) const {
        search(query, true);
    }
};

int main() {
    MiniSearchEngine engine;
    int choice;

    do {
        cout << "\n=========================================\n";
        cout << "          MINI SEARCH ENGINE\n";
        cout << "=========================================\n";
        cout << "1. Add / Index Document\n";
        cout << "2. View Indexed Documents\n";
        cout << "3. Search Documents\n";
        cout << "4. Search and Save Result\n";
        cout << "5. Exit\n";
        cout << "\nEnter your choice: ";

        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input.\n";
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        switch (choice) {
            case 1: {
                string fileName;
                cout << "Enter text file path: ";
                getline(cin, fileName);
                engine.addDocument(fileName);
                break;
            }
            case 2:
                engine.displayDocuments();
                break;
            case 3: {
                string query;
                cout << "Enter search keyword(s): ";
                getline(cin, query);
                engine.search(query);
                break;
            }
            case 4: {
                string query;
                cout << "Enter search keyword(s): ";
                getline(cin, query);
                engine.saveSearchResult(query);
                break;
            }
            case 5:
                cout << "Thank you for using Mini Search Engine!\n";
                break;
            default:
                cout << "Invalid choice!\n";
        }
    } while (choice != 5);

    return 0;
}
