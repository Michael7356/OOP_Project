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

Transaction::Transaction(std::string type, std::string d, std::string t, std::string c, const double a, std::string n)
    :type(std::move(type)), date(std::move(d)), time(std::move(t)), category(std::move(c)), amount(a), note(std::move(n)) {}

receipt::receipt(std::string type,std::string date, std::string time, std::string category, const double a, std::string note, std::string receiptNumber)
    : Transaction(std::move(type),std::move(date), std::move(time), std::move(category), a, std::move(note)), receiptNumber(std::move(receiptNumber)) {}

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
            outFile << record.getType() << ","
                    << record.getDate() << ","
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


void receipt::checkUnique( std::vector<receipt>& records,const std::string& filename) {
    std::unordered_set<std::string>receiptID;
    std::vector<receipt> tempRecords;
    std::ifstream inFile(filename);
    if (!inFile.is_open()) {
        std::cerr << "Error at checkUnique" << std::endl;
        return ;
    }
    std::string line;
    while (std::getline(inFile, line)) {
        std::stringstream ss (line);
        std::string input;
        for (int i = 0 ; i < 6 ; i ++) {
            std::getline(ss, input, ','); // First six blanks are not receipt number
        }
        std::getline(ss, input, ',');
        receiptID.insert(input);
    }
    for (const auto& r :records) {
        if (!receiptID.contains(r.getReceiptNumber())) { //contains function was added at C++20
            tempRecords.push_back(r);
        }
    }
    inFile.close();
    records.swap(tempRecords);
}
