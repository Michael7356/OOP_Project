#include "Transaction.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <utility>
#include <vector>
#include <utility>
#include <sstream>
#include <poppler/cpp/poppler-document.h>
#include <poppler/cpp/poppler-page.h>
#include "httplib.h"
#include "json.hpp"

Transaction::Transaction(std::string d, std::string t, std::string c, const double a, std::string n)
    : date(std::move(d)), time(std::move(t)), category(std::move(c)), amount(a), note(std::move(n)) {}

receipt::receipt(std::string date, std::string time, std::string category, const double a, std::string note, std::string receiptNumber)
    : Transaction(std::move(date), std::move(time), std::move(category), a, std::move(note)), receiptNumber(std::move(receiptNumber)) {}

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
            outFile << "\xEF\xBB\xBF Date,Time,Category,Amount,Note, ReceiptNumber\n";
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

void receipt::saveToFile(const std::vector<receipt> &records, const std::string &filename) {
    std::ifstream inFile(filename);
    bool isNew = !inFile.is_open() || inFile.peek() == std::ifstream::traits_type::eof();
    inFile.close();

    std::ofstream outFile(filename, std::ios::out|std::ios::app);
    if (outFile.is_open()) {
        if (isNew){
            outFile << "\xEF\xBB\xBF Date,Time,Category,Amount,Note, ReceiptNumber\n";
        }
        for (const auto& record : records) {
            outFile << record.getDate() << ",";
            outFile << record.getTime() << ",";
            outFile << record.getCategory() << ",";
            outFile << record.getAmount() << ",";
            outFile << record.getNote() << ",";
            outFile << record.getReceiptNumber() << "\n";
        }
        outFile.close();
        std::cout << "Date saved to " << filename << " successfully" << std::endl;
    }
    else {
        std::cerr << "Error" << std::endl;
    }
}
