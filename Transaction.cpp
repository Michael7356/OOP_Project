#include "Transaction.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <utility>
#include <sstream>
#include <poppler/cpp/poppler-document.h>
#include <poppler/cpp/poppler-page.h>


Transaction::Transaction(std::string d, std::string c, const double a, std::string n)
    : date(std::move(d)), category(std::move(c)), amount(a), note(std::move(n)) {}

void Transaction::display() const {
    std::cout << std::left <<std::setw(12) << date << std::setw(10) << category << std::setw(8) << amount << " | " << note << std::endl;
}

void Transaction::saveToFile(const std::vector<Transaction>& records, const std::string& filename) {
    std::ofstream outFile(filename);

    if (outFile.is_open()) {
        outFile << "\xEF\xBB\xBF";
        outFile << "Date,Category,Amount,Note\n";

        for (const auto& record : records) {
            outFile << record.getDate() << ","
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
        std::string d, c, a_str, n;

        std::getline(ss, d, ',');
        std::getline(ss, c, ',');
        std::getline(ss, a_str, ',');
        std::getline(ss, n, ',');

        try {
            if (!a_str.empty()) {
                double a = std::stod(a_str);
                tempRecords.emplace_back(d, c, a, n);
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
