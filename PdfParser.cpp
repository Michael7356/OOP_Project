#include "PdfParser.h"
#include <iostream>
#include <regex>
#include <poppler-document.h>
#include <poppler-page.h>
#include <sstream>
#include <map>
#include <optional>
#include "httplib.h"
#include "json.hpp"
#include <filesystem>

using json = nlohmann::json;
namespace fs = std::filesystem;

struct Config {
    std::string script_ID;
    std::string csv_filename;
    std::string deposit;
    std::string cate_path;
};

PdfParser::Config PdfParser::loadConfig() {
    std::ifstream inFile("Storage/config.json");
    if (!inFile) {
        std::cerr << "Error at loadConfig" <<std::endl;
    }
    json j;
    inFile >> j;
    return {j["google_script_id"], j["csv_path"], j["deposit"],};
};

std::vector<std::shared_ptr<Transaction>> PdfParser::parseBankStatement(const std::string& filepath, const std::string& password,const std::string& type) {
    std::vector<std::shared_ptr<Transaction>> result;
    const poppler::document *doc = poppler::document::load_from_file(filepath, password);
    if (!doc) {
        throw std::runtime_error("Error at loadBankStatement");
    }

    for (int i = 0 ; i < doc->pages(); ++i) {
        poppler::page* p = doc->create_page(i);
        if (p) {
            std::string pageText = p->text().to_utf8().data();
            std::stringstream ss(pageText);
            std::string line;

            while (getline(ss, line)) {
                if (Transaction t("","", "", "", 0.0, ""); processLine(line, t, type)) {
                    result.push_back(std::make_shared<Transaction>(t));
                }
            }
            delete p;
        }
    }
    delete doc;
    return result;
}

bool PdfParser::processLine(const std::string& line, Transaction& outTransaction,const std::string& type) {
    std::string date, time, amount_str,description, other_acc, summary, note, finalType;
    if (type == "CTBC"){
        finalType = type;
        const std::regex pattern(R"((\d{4}/\d{2}/\d{2}\s+\d{2}:\d{2}:\d{2})\s{3}([^\s-]+)?.*?(\d{1,3}(?:,\d{3})?).*?(\d{1,3}(?:,\d{3})?)\s{3}([^\s-]+)?\s+([^\s-]+|-)?\s+([^\s-]+)?)");
        std::smatch match;
        if (std::regex_search(line, match, pattern)) {
            std::string dateAndTime = match[1];
            date = match[1].str().substr(0,10);
            date = date.substr(0,4) + date.substr(5,2)+date.substr(8, 2);
            time = match[1].str().substr(11,5);
            summary = match[2];
            amount_str = match[3];
            std::string balance = match[4];
            note = match[5];
            other_acc = match[6];
            description = match[7];
            if (dateAndTime == "2025/09/19 18:51:44") {
                return false;
            }
        }
    }
    else if (type == "POST") {
        finalType = "POST[Unmatched]";
        const std::regex pattern(R"(.*?\s{1,}(\d{0,2}\/\d{0,2})\s{1,}(\d{0,2}\/\d{0,2})\s{1,}(.*?)\s{1,}(.*?)\s{1,}(.*?)\s{1,}(\d{0,5}.\d{2})\s{1,}(\d{0,5}))");
        std::smatch match;
        if (std::regex_search(line, match, pattern)) {
            std::string dateA = match[1];
            std::string dateB = match[2];
            std::string year = [] {
                auto now = std::chrono::system_clock::now();
                auto local_zone = std::chrono::current_zone();
                auto local_time = local_zone->to_local(now);
                return std::format("{:%Y}", local_time);
            }();
            date = year + dateA.substr(0,2) + dateA.substr(3,2) + "/" + year + dateB.substr(0,2) + dateB.substr(3,2);

            time = "No time";
            description = match[3];
            amount_str = match[7];
        }
    }
    else {
        std::cerr << "Invalid input" << std::endl;
        return false;
    }

    if (description.find("連加＊" ) != std::string::npos) description = description.substr(9);
    if (description.find("連支＊" ) != std::string::npos) description = description.substr(9);

    std::erase(amount_str, ',');
    try {
        double amount = std::stod(amount_str);

        bool isDeposit = (other_acc != "-" && !other_acc.empty() || note == "租金補");

        if (summary == "現金提") description = summary;

        if (!isDeposit) {
            amount = -amount;
        }
        outTransaction = Transaction(finalType, date, time, description, amount, "No receipt data");
        return true;
    }
    catch (...) {
        return false;
    }
    return false;
}

Transaction PdfParser::resolvingRegex_Mail(std::string smsText) {
    std::regex pattern (R"(.*?(\d{2}\/\d{2})\s{1}(\d{2}:\d{2}).*?\d{4}.*?(\d{1,6}).*?)");
    std::smatch match;
    Config config = loadConfig();
    if (std::regex_search(smsText, match, pattern)) {
        std::string date = match[1];
        date = date.substr(0,2) + date.substr(3,2);
        std::string year = [] {
            auto now = std::chrono::system_clock::now();
            auto local_zone = std::chrono::current_zone();
            auto local_time = local_zone->to_local(now);
            return std::format("{:%Y}", local_time);
        }();
        date = year + date;
        std::string time = match[2] ;
        std::string amountStr = match[3];
        std::erase(amountStr, ',');
        double amount = std::stod(amountStr);
        Transaction t = {"POST[Unconfirmed]", date, time, "No Category", amount, "No note"};
        return t;
    }
    throw std::runtime_error("Error at loadBankStatement");
}

std::string PdfParser::getCSVfile(const std::string& path) {
    fs::path file;
    fs::file_time_type time;

    for (const auto& entry : fs::directory_iterator(path)) {
        if (entry.path().extension() == ".csv") {
            if (file.empty() || fs::last_write_time(entry) > time) {
                file = entry.path();
                time = fs::last_write_time(entry);
            }
        }
    }
    return file.string();
}


std::vector<receipt> csvParser::loadFromFile(const std::string& filename) {
    std::vector<receipt> tempRecords;
    std::ifstream inFile(filename);
    if (!inFile.is_open()) {
        std::cerr << "Error opening file " << filename<< std::endl;
        return tempRecords;
    }
    std::string line;
    std::getline(inFile, line); // We don't need first and last line
    bool cont = true;
    std::vector<std::string> tokens;
    while (std::getline(inFile, line)) {
        std::stringstream ss(line);
        std::vector<std::string> temp;
        std::string input;
        for (int i = 0 ; i < 14 ; i ++) {
            std::getline(ss,input, ',');
            if (input == "捐贈或作廢之發票，字軌號碼均會隱末3碼") {
                cont = false;
                break;
            }
            temp.push_back(input);
        }
        if (!cont) break;
        std::string amountStr = temp[3];
        try {
            if (!amountStr.empty()) {
                double amount = std::stod(amountStr);
                tempRecords.emplace_back("Receipt", temp[1], "No Time", temp[7], amount * -1, temp[13], temp[2]);
            }
        }
        catch (std::invalid_argument& e) {
            std::cerr << "Error when trying to convert " << input << std::endl;
        }

    }
    inFile.close();
    return tempRecords;
}

std::vector<std::shared_ptr<Transaction>> csvParser::loadFromFile_PS(const std::string& filename) {
    std::vector<std::shared_ptr<Transaction>> tempRecords;
    std::ifstream inFile(filename);
    if (!inFile.is_open()) {
        throw std::runtime_error("Error opening file " + filename);
    }
    std::string line;
    std::getline(inFile, line);
    std::getline(inFile, line); //First two line is useless
    int count = 0;
    while (std::getline(inFile,line)) {
        count ++;
        std::stringstream ss(line);
        std::string input;
        std::string date, time, type,AmountStr,balanceStr;
        std::regex csv_regex(R"((\d{3}\/\d{2}\/\d{2})\s{1}(\d{2}:\d{2}).*?,(.*?),.*?,"?,?"?(\d{0,3},?\d{0,3}).*?,.*?,?"(\d{0,3},?\d{0,3}).*?,(.*),)");
        std::smatch match;
        if (regex_search(line, match, csv_regex)) {
            date = match[1];
            time = match[2];
            type = match[3];
            AmountStr = match[4];
            std::erase(AmountStr, ',');
            balanceStr = match[5];
            std::erase(balanceStr, ',');
        }
        if (date.empty()) break;
        std::string year = date.substr(0,3);
        int nowYear = 0;
        nowYear = std::stoi(year) + 1911;
        date = std::to_string(nowYear) + date.substr(4,2) + date.substr(7,2);
        if (!AmountStr.empty()) {
            double amount;
            amount = std::stod(AmountStr);
            if (type.find("轉入") != std::string::npos || type.find("回饋") != std::string::npos) {
                tempRecords.push_back(std::make_shared<Transaction>("POST[Uncategorized]", date, time, "No Category",amount, ""));
            }
            else {
                tempRecords.push_back(std::make_shared<Transaction>("POST[Uncategorized]", date, time, "No Category",amount * -1, ""));
            }
        }
    }
    return tempRecords;
}