#include "DataManager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

std::vector<std::shared_ptr<Transaction> > DataManager::loadFromFile(const std::string &fileName) {
    std::vector<std::shared_ptr<Transaction> > result;
    std::ifstream inFile(fileName);

    if (!inFile.is_open()) {
        std::cerr<< "Failed at open file (Datamanager.load)"<<std::endl;
        return result;
    }
    std::string line;
    std::getline(inFile, line);
    while (std::getline(inFile, line)) {
        std::stringstream ss(line);
        std::string type, date, time, category, amountStr, note;
        std::getline(ss, type, ',');
        std::getline(ss, date, ',');
        std::getline(ss, time, ',');
        std::getline(ss, category, ',');
        std::getline(ss, amountStr, ',');
        std::getline(ss, note, ',');
        double amount = amountStr.empty()? 0.0 : std::stod(amountStr);
        if (type == "Bank") {
            result.push_back(std::make_shared<Transaction>(type, date, time, category, amount, note));
        }
        else if (type == "Receipt") {
            std::string receiptNumber;
            std::getline(ss, receiptNumber, ',');
            result.push_back(std::make_shared<receipt>(type, date, time, category, amount, note, receiptNumber));
        }
    }
    inFile.close();
    return result;
}

void DataManager::saveToFile(const std::vector<std::shared_ptr<Transaction>>& records, const std::string &fileName) {
    std::ifstream inFile(fileName);
    bool isNew = !inFile.is_open() || inFile.peek() == std::ifstream::traits_type::eof();
    inFile.close();

    std::ofstream outFile(fileName, std::ios::out|std::ios::trunc);
    if (!outFile.is_open()) {
        std::cerr << "Failed at open file (Datamanager.save)" << std::endl;
        return;
    }
    outFile << "\xEF\xBB\xBF";
    if (isNew) {
        outFile << "Date,Time,Category,Amount,Note, ReceiptNumber\n";
    }
    for (const auto& tptr : records) {
        if (!tptr) continue;
        auto rptr = dynamic_cast<const receipt*>(tptr.get());
        if (rptr) {
            outFile << rptr->getType() << ","
                    << rptr->getDate() << ","
                    << rptr->getTime() << ","
                    << rptr->getCategory() << ","
                    <<rptr->getAmount() << ","
                    << rptr->getNote() << ","
                    << rptr->getReceiptNumber() << "\n";
        }
        else {
            outFile << tptr->getType() << ","
                    << tptr->getDate() << ","
                    << tptr->getTime() << ","
                    << tptr->getCategory() << ","
                    << tptr->getAmount() << ","
                    << tptr->getNote() << ",No receipt data\n";
        }
    }
    outFile.close();
    std::cout << "Data successfully save" << std::endl;
}

bool DataManager::compare(const std::shared_ptr<Transaction>& a, const std::shared_ptr<Transaction>& b) {
    if (!a || !b)  return a < b;

    if (a->getDate() != b->getDate())  return a->getDate() > b->getDate();

    auto rA = dynamic_cast<const receipt*>(a.get());
    auto rB = dynamic_cast<const receipt*>(b.get());
    if (rA && rB) { //if rA and rB are all receipt, then compare it by receipt number
        if (rA->getReceiptNumber() != rB->getReceiptNumber()) {
            return rA->getReceiptNumber() > rB->getReceiptNumber();
        }
        return rA->getAmount() > rB->getAmount();
    }

    if (a->getTime() != b->getTime())  return a->getTime() > b->getTime();

    return a->getNote() > b->getNote();
}
