#include "Transaction.h"
#include "DisplayUtil.h"
#include <conio.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <utility>
#include <vector>
#include <sstream>
#include <poppler/cpp/poppler-page.h>
#include "../include/httplib.h"
#include "../include/json.hpp"

#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"

Transaction::Transaction(std::string type, std::string d, std::string t, std::string c, const double a, std::string n)
    :type(std::move(type)), date(std::move(d)), time(std::move(t)), category(std::move(c)), amount(a), note(std::move(n)) {}

receipt::receipt(std::string type,std::string date, std::string time, std::string category, const double a, std::string note, std::string receiptNumber)
    : Transaction(std::move(type),std::move(date), std::move(time), std::move(category), a, std::move(note)), receiptNumber(std::move(receiptNumber)) {}

void Transaction::display() const {
    bool leftAlign = true;
    std::string f_type = DisplayUtil::formatOutput(type, 22, leftAlign);
    std::string f_date = DisplayUtil::formatOutput(date.substr(0,8), 12, leftAlign);
    std::string f_time = DisplayUtil::formatOutput(time, 10, leftAlign);
    std::string f_category = DisplayUtil::formatOutput(category, 22, leftAlign);
    std::cout << f_type << f_date<< f_time << f_category
              << std::right << std::setw(8);
    if (amount < 0) std::cout << RED << amount << RESET;
    else {
        std::cout << GREEN << amount << RESET;
    }
    std::cout << " | " << note << std::endl;
}

void Transaction::editType(const std::string& type) {
    this ->type = type;
}

void Transaction::editDate(const std::string &date) {
    this->date = date;
}
void Transaction::editTime(const std::string &time) {
    this->time = time;
}
void Transaction::editDateAndTime(const std::string &date, const std::string &time) {
    this->date = date;
    this->time = time;
}

void Transaction::editCategory(const std::string& category) {
    this->category = category;
}
void Transaction::editNote(const std::string& note) {
    this->note = note;
}
void Transaction::editAmount(double amount) {
    this->amount = amount;
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
        throw std::runtime_error(std::string("Could not open file ") + filename);
    }
}

int Transaction::compareDate(const std::shared_ptr<Transaction>& newRecord, const std::shared_ptr<Transaction>& oldRecord) {
    int month[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
    std::string dateA = newRecord->getDate();
    int yearA =(std::stoi(dateA.substr(0, 4)) - 2000) * 365;
    int monthA = 0;
    for (int i = 1 ; i <= std::stoi(dateA.substr(4,2)); i++) {
        monthA += month[i];
    }
    int daysA = std::stoi(dateA.substr(6,2)) + monthA + yearA;
    std::string dateB = oldRecord->getDate();
    int yearB =(std::stoi(dateB.substr(0, 4)) - 2000) * 365;
    int monthB = 0;
    for (int i = 1 ; i <= std::stoi(dateB.substr(4,2)); i++) {
        monthB += month[i];
    }
    int daysB = std::stoi(dateB.substr(6,2)) + monthB + yearB;

    return daysA - daysB;
}

double Transaction::calculateThisMonthAmount(const std::vector<std::shared_ptr<Transaction>> &records) {
    double totalAmount = 0;
    const std::string date = [] {
        auto now = std::chrono::system_clock::now();
        auto local_zone = std::chrono::current_zone();
        auto local_time = local_zone->to_local(now);
        return std::format("{:%Y%m}", local_time);
    }();
    for (const auto& record : records) {
        if (record->getDate().substr(0,6) == date) totalAmount += record->getAmount() * -1; // We need positive number
    }
    return totalAmount;
}

bool Transaction::operator ==(const std::shared_ptr<Transaction>& other) const {
    if (this->getDate() == other->getDate() && this->getTime() == other->getTime() && this->getCategory() == other->getCategory() && this->getAmount() == other->getAmount() && this->getNote() == other->getNote()) return true;
    return false;
}

void receipt::checkUnique( std::vector<std::shared_ptr<Transaction>>& records,const std::string& filename) {
    std::unordered_set<std::string>receiptID;
    std::vector<std::shared_ptr<Transaction>> tempRecords;
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
        auto rptr = dynamic_cast<const receipt*>(r.get());
        if (rptr) {
            if (!receiptID.contains(rptr->getReceiptNumber())) { //contains function was added at C++20
                tempRecords.push_back(r);
            }
        }
    }
    inFile.close();
    records.swap(tempRecords);
}

std::vector<std::shared_ptr<Transaction>> receipt::getSimpleRecords(const std::vector<std::shared_ptr<Transaction>> &records) {
    std::unordered_map<std::string, size_t> receiptID; //size_t use to tell index of array
    std::vector<std::shared_ptr<Transaction>> tempRecords;

    for (const auto& record : records) {
        if (!record) continue;

        auto rptr = dynamic_cast<const receipt*>(record.get());

        if (!rptr || rptr->getType() == "Deleted") continue;

        std::string receiptNumber = rptr->getReceiptNumber();
        if (!receiptID.contains(receiptNumber)) {
            std::shared_ptr<Transaction> temp = record->clone();
            tempRecords.push_back(temp);
            receiptID[receiptNumber] = tempRecords.size() - 1;
        }
        else {
            size_t index = receiptID[receiptNumber];
            double currAmount = tempRecords[index]->getAmount();
            double incomingAmount = rptr->getAmount();
            std::string currNote = tempRecords[index]->getNote();
            tempRecords[index]->editAmount(currAmount + incomingAmount);
            tempRecords[index]->editNote(currNote + "    " + rptr->getNote());
        }
    }
    return tempRecords;
}

std::shared_ptr<Transaction> Transaction::clone() const {
    return std::make_shared<Transaction>(*this);
}

std::shared_ptr<Transaction> receipt::clone() const {
    return std::make_shared<receipt>(*this);
}

void receipt::deleteReceipt(const std::string &receiptNumber, const std::vector<std::shared_ptr<Transaction>>& transactions) {
    for (const auto& transaction: transactions) {
        auto rptr = dynamic_pointer_cast<receipt>(transaction); //shared_ptr is not an object so couldn't use dynamic_cast
        if (rptr) {
            if (receiptNumber == rptr->getReceiptNumber()) {
                rptr->editType("Deleted");
            }
        }
    }
}

void receipt::editMultiType(const std::vector<std::string> &receiptNumber, const std::string &type, std::vector<std::shared_ptr<Transaction>> &records) {
     for (const auto& number : receiptNumber) {
         for (const auto& record : records) {
             auto rptr = dynamic_pointer_cast<receipt>(record);
             if (rptr) {
                 if (rptr->getReceiptNumber() == number) rptr->editType(type);
             }
         }
     }
}

void receipt::editMultiTime(const std::string &receiptNumber, const std::string &time, std::vector<std::shared_ptr<Transaction>> &records) {
    for (const auto& record : records) {
        auto rptr = dynamic_pointer_cast<receipt>(record);
        if (rptr) {
            if (rptr->getReceiptNumber() == receiptNumber) rptr->editTime(time);
        }
    }
}

void receipt::matchingRecords(std::vector<std::shared_ptr<Transaction>>& records) {
    std::vector<std::shared_ptr<Transaction>> matchRecords;
    std::vector<std::shared_ptr<Transaction>> receiptRecords = getSimpleRecords(records);
    for (const auto& record : records) {
        if (record->getType() == "POST[Unmatched]" || record->getType() == "CTBC" || record->getType() == "IPass") {
            matchRecords.push_back(record);
        }
    }
    for (const auto& record : matchRecords) { //Transaction data
        std::vector<std::string> receiptNumber;
        if (record->getType() == "POST[Unmatched]") {
            std::shared_ptr<Transaction> temp = record->clone();
            temp->editDate(record->getDate().substr(0,8)); //Transaction date
            for (const auto& receiptData : receiptRecords) {
                int diff = compareDate( receiptData, temp);
                if (diff <= 1 && diff >= 0 && receiptData->getType() != "UsedData") {
                    if (std::abs(temp->getAmount()) == std::abs(receiptData->getAmount()) && temp->getCategory() == receiptData->getCategory()) {
                        auto dataPtr = std::dynamic_pointer_cast<receipt>(receiptData);
                        receiptNumber.push_back(dataPtr->getReceiptNumber());
                        receiptData->editType("UsedData");
                        if (receiptData->getAmount() > 0) receiptData->editAmount(receiptData->getAmount() * -1);
                        receiptData->editTime(temp->getTime());
                        record->editType("Deleted"); //It would be deleted after complete the match
                        break;
                    }
                }
            }
            editMultiType(receiptNumber, "Receipt(POST)", records);
        }
        else if (record->getType() == "CTBC") {
            for (const auto& receiptData : receiptRecords) {
                int diff = compareDate(record, receiptData);
                if (diff >= 0 && diff <= 7 && receiptData->getType() != "UsedData") {
                    if (abs(record->getAmount()) == abs(receiptData->getAmount())&& record->getCategory() == receiptData->getCategory()) {
                        auto dataPtr = std::dynamic_pointer_cast<receipt>(receiptData);
                        dataPtr->editType("UsedData");
                        receiptNumber.push_back(dataPtr->getReceiptNumber());
                        if (receiptData->getAmount() > 0) receiptData->editAmount(receiptData->getAmount() * -1);
                        record->editType("Deleted");
                        break;
                    }
                }
            }
            editMultiType(receiptNumber, "Receipt(CTBC)", records);
        }
        else if (record->getType() == "IPass") {
            for (const auto& receiptData : receiptRecords) {
                int diff = compareDate( record, receiptData);
                if (diff >= 0 && diff <= 1 && receiptData->getCategory() == record->getCategory()){
                    if (record->getAmount() == receiptData->getAmount()) {
                        auto dataPtr = std::dynamic_pointer_cast<receipt>(receiptData);
                        dataPtr->editType("UsedData");
                        receiptNumber.push_back(dataPtr->getReceiptNumber());
                        record->editType("Deleted");
                        editMultiTime(dataPtr->getReceiptNumber(), record->getTime(), records);
                        break;
                    }
                    if (receiptData->getNote().find("提貨卡") != std::string::npos && receiptData->getAmount() + 5 == record->getAmount()) {
                        auto dataPtr = std::dynamic_pointer_cast<receipt>(receiptData);
                        dataPtr->editType("UsedData");
                        receiptNumber.push_back(dataPtr->getReceiptNumber());
                        dataPtr->editAmount(record->getAmount());
                        record->editType("Deleted");
                        editMultiTime(dataPtr->getReceiptNumber(), record->getTime(), records);
                        break;
                    }
                }
            }
            editMultiType(receiptNumber, "Receipt(IPass)", records);

        }
    }
}