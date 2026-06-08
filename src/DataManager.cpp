#include "DataManager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include "../include/json.hpp"

using json = nlohmann::json;
std::map<std::string, std::string> DataManager::data;

void DataManager::checkInitFile(const std::string &filename) {
    std::filesystem::path filePath(filename); //Won't pops out error if the file doesn't exist in anywhere.

    if (filePath.has_parent_path()) {
        std::filesystem::path parentDir = filePath.parent_path();
        if (!std::filesystem::exists(parentDir)) {
            try {
                std::filesystem::create_directory(parentDir);
                std::cout << "Folder: " <<parentDir << " is not exist, we have create the folder" << std::endl;
            }catch (const std::exception& e) {
                std::cerr << "Error happened at creating parent folder [Error code:" << e.what() << "]" << std::endl;
            }
        }
    }
    if (!std::filesystem::exists(filename)) {
        std::ofstream newFile (filename, std::ios::out);
        std::cout << "File: " << filename << " is not exist, we have create the file" << std::endl;
        if (newFile.is_open()) {
            if (filename == "Storage/Transaction.csv") {
                newFile << "\xEF\xBB\xBF";
                newFile << "Type, Date, Time, Category, Amount, Note, ReceiptNumber\n";
            }
            else if (filename == "Storage/config.json") {
                std::string input;
                std::map<std::string, std::string> initConfig = {{"csv_path", "Storage/Transaction.csv"}, {"Account", ""}, {"Password", ""}};
                std::cout << "Looks like you're first time using this program, you have to type these necessary data" << std::endl;
                std::cout << "Your account [財政部電子發票平台帳號]: ";
                std::cin >> input; initConfig["Account"] = input;
                std::cout << "Your password [財政部電子發票平台密碼]: ";
                std::cin >> input; initConfig["Password"] = input;
                std::cout << "Your IPass card number[一卡通背後卡號]: ";
                std::cin >> input; initConfig["IPassCardNumber"] = input;
                std::cout << "Your ID card's last four digits[身分證後四碼(用於查詢一卡通資料)]: ";
                std::cin >> input; initConfig["IDLastFourDigits"] = input;
                std::cout << "Your google script url [Read Readme.md if you don't know what it is]: ";
                std::cin >> input; initConfig["google_script_id"] = input;
                json j = initConfig;
                newFile << j.dump(4);
                std::cout << "Create .json file successfully" << std::endl;
            }
            else if (filename == "Storage/cate.json") {
                json defaultJson = json::object();
                newFile << defaultJson.dump(4);
            }
            else if (filename == "Storage/del.json") {
                json defaultJson = json::array();
                newFile << defaultJson.dump(4);
            }
            newFile.close();
        }
        else {
            std::cerr << "Error happened at creating file" << std::endl;
        }
    }
}

std::vector<std::shared_ptr<Transaction>> DataManager::loadFromFile(const std::string &fileName) {
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
        if (type.empty()) continue;
        if (type.find("Receipt") != std::string::npos) {
            std::string receiptNumber;
            std::getline(ss, receiptNumber, ',');
            result.push_back(std::make_shared<receipt>(type, date, time, category, amount, note, receiptNumber));
        }
        else {
            result.push_back(std::make_shared<Transaction>(type, date, time, category, amount, note));
        }
    }
    inFile.close();
    return result;
}

void DataManager::saveToFile(const std::vector<std::shared_ptr<Transaction>>& records, const std::string &fileName) {

    std::ofstream outFile(fileName, std::ios::out|std::ios::trunc);
    if (!outFile.is_open()) {
        std::cerr << "Failed at open file (Datamanager.save)" << std::endl;
        return;
    }
    outFile << "\xEF\xBB\xBF";
    outFile << "Type,Date,Time,Category,Amount,Note, ReceiptNumber\n";
    for (const auto& tptr : records) {
        if (!tptr) continue;
        if (tptr->getType() == "Deleted") {
            continue;
        }
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

void DataManager::loadCategory(const std::string& filepath) {
    //Only accept .json file
    if (!std::filesystem::exists(filepath)) {
        checkInitFile("Storage/cate.json");
    }
    std::ifstream inFile(filepath);
    if (inFile.is_open()) {
        json j;
        try {
            inFile >> j;
            inFile.close();
            if (j.is_object() && !j.is_null()) {
                data = j.get<std::map<std::string, std::string>>();
                std::cout << "Load categories successfully" << std::endl;
            }
            else {
                data.clear();
                std::cout << "Failed to load categories, please check the .json file" << std::endl;
            }
        }catch (json::exception& e) {
            std::cerr << "Failed to load categories from .json file [Error code: " << e.what() << "]" << std::endl;
            data.clear();
        }
    }
}

void DataManager::saveCategory(const std::string& filepath) {
    std::ofstream outFile(filepath, std::ios::out|std::ios::trunc);
    if (outFile.is_open()) {
        json j = data;
        outFile << j.dump(4);
        outFile.close();
        std::cout << "Save categories successfully" << std::endl;
    }
}

std::map<std::string, std::string> DataManager::getCategories() {
    return data;
}

void DataManager::categoryMapping(const std::vector<std::shared_ptr<Transaction>> &transactions) {
    for (const auto& transaction : transactions) {
        if (!transaction) {return;}

        auto it = data.find(transaction->getCategory());
        if (it != data.end()) {
            transaction->editCategory(it->second);
            continue;
        }
        for (const auto& [fst, snd] : data) {
            if (transaction->getCategory().find(fst) != std::string::npos) {
                transaction->editCategory(snd);
                break;
            }
        }
    }
}

void DataManager::addCategory(const std::string& OriginCategory, const std::string& CategoryName) {
    if (!data.contains(OriginCategory)) {
        data[OriginCategory] = CategoryName;
        std::cout << "Successfully add a category" << std::endl;
    }
    else {
        std::cout << "This category is already exist in category map" << std::endl;
    }
}

void DataManager::removeCategory(const std::string& category) {
    data.erase(category);
}