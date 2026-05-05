#include "Transaction.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <utility>
#include <sstream>
#include <poppler/cpp/poppler-document.h>
#include <poppler/cpp/poppler-page.h>
#include "httplib.h"
#include "json.hpp"

Transaction::Transaction(std::string d, std::string t, std::string c, const double a, std::string n)
    : date(std::move(d)), time(std::move(t)), category(std::move(c)), amount(a), note(std::move(n)) {}

void Transaction::display() const {
    std::cout << std::left <<std::setw(12) << date << std::setw(10) << time << std::setw(8) << category << std::setw(6) << amount << " | " << note << std::endl;
}

void Transaction::saveToFile(const std::vector<Transaction>& records, const std::string& filename) {
    std::ifstream inFile(filename);
    bool isNew = !inFile.is_open() || inFile.peek() == std::ifstream::traits_type::eof();
    inFile.close();

    std::ofstream outFile(filename, std::ios::out|std::ios::app);
    if (outFile.is_open()) {
        if (isNew){
            outFile << "\xEF\xBB\xBF Date,Time,Category,Amount,Note\n";
        }
        for (const auto& record : records) {
            outFile << record.getDate() << ","
                    << record.getTime() << ","
                    << record.getCategory() << ","
                    << record.getAmount() << ","
                    << record.getNote() << "\n";
        }
        outFile.close();
        std::cout<<"Data saved to "<< filename << " successfully" << std::endl;
    }
    else {
        std::cerr << "Error" << std::endl;
    }
}

std::vector<Transaction> Transaction::loadFromFile(const std::string& filename) {
    std::vector<Transaction> tempRecords;
    std::ifstream inFile(filename);
    if (!inFile.is_open()) {
        std::cerr << "Error opening file " << filename << std::endl;
        return tempRecords;
    }
    std::string line;
    std::getline(inFile, line);
    while (std::getline(inFile, line)) {
        std::stringstream ss(line);
        std::string d, c, a_str, n, t;

        std::getline(ss, d, ',');
        std::getline(ss, t, ',');
        std::getline(ss, c, ',');
        std::getline(ss, a_str, ',');
        std::getline(ss, n, ',');

        try {
            if (!a_str.empty()) {
                double a = std::stod(a_str);
                tempRecords.emplace_back(d, t, c, a, n);
            }
        }
        catch (std::invalid_argument& e) {
            std::cerr << "Continue" << std::endl;
            continue;
        }




    }
    inFile.close();
    return tempRecords;
}

void Transaction::extractFromFile(const std::string& filepath, const std::string& password) {
    poppler::document* doc = poppler::document::load_from_file(filepath, password);
    if (!doc) {
        std::cerr << "Could not load file " << filepath << std::endl;
        return;
    }

    for (int i = 0 ; i < doc->pages() ; ++i) {
        poppler::page* p = doc->create_page(i);
        if (p) {
            std::string pageText = p->text().to_utf8().data();
            std::cout << "Page: " << i + 1 << std::endl;
            std::cout << pageText << std::endl;
            delete p;
        }
    }
    delete doc;
}
