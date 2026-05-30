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

std::vector<Transaction> PdfParser::parseBankStatement(const std::string& filepath, const std::string& password) {
    std::vector<Transaction> result;
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
                if (Transaction t("","", "", "", 0.0, ""); processLine(line, t)) {
                    result.push_back(t);
                }
            }
            delete p;
        }
    }
    delete doc;
    return result;
}

bool PdfParser::processLine(const std::string& line, Transaction& outTransaction) {
    const std::regex pattern(R"((\d{4}/\d{2}/\d{2}\s+\d{2}:\d{2}:\d{2})\s{3}([^\s-]+)?.*?(\d{1,3}(?:,\d{3})?).*?(\d{1,3}(?:,\d{3})?)\s{3}([^\s-]+)?\s+([^\s-]+|-)?\s+([^\s-]+)?)");
    std::smatch match;
    if (std::regex_search(line, match, pattern)) {
        std::string dateAndTime = match[1];
        std::string date = match[1].str().substr(0,10);
        date = date.substr(0,4) + date.substr(5,2)+date.substr(8, 2);
        std::string time = match[1].str().substr(11,5);
        std::string summary = match[2];
        std::string amount_str = match[3];
        std::string balance = match[4];
        std::string note = match[5];
        std::string other_acc = match[6];
        std::string description = match[7];
        if (description.find("連加＊") != std::string::npos) description = description.substr(9);



        if (dateAndTime == "2025/09/19 18:51:44") {
            return false;
        }

        std::erase(amount_str, ',');
        try {
            double amount = std::stod(amount_str);

            bool isDeposit = (other_acc != "-" && !other_acc.empty() || note == "租金補");

            if (summary == "現金提") description = summary;

            if (!isDeposit) {
                amount = -amount;
            }


            outTransaction = Transaction("CTBC", date, time, description, amount, "");
            return true;
        }
        catch (...) {
            return false;
        }
    }
    return false;
}

std::optional<Transaction> PdfParser::resolvingRegex_Mail(std::string smsText) {
    std::regex pattern (R"(.*?(\d{2}\/\d{2})\s{1}(\d{2}:\d{2}).*?\d{4}.*?(\d{1,6}).*?)");
    std::smatch match;
    Config config = loadConfig();
    if (std::regex_search(smsText, match, pattern)) {
        std::string date = match[1];
        std::string time = match[2] ;
        time = time.substr(0,2) + time.substr(2, 2);
        std::string amountStr = match[3];
        std::erase(amountStr, ',');
        double amount = std::stod(amountStr);
        Transaction t = {"Bank", date, time, "", amount, ""};
        return t;
    }
    std::cout << "Can't resolve the format of message" << std::endl;
    return std::nullopt;
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
                tempRecords.emplace_back("Receipt", temp[1], "No Time", temp[7], amount, temp[13], temp[2]);
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
    bool cont = true;
    std::getline(inFile, line);
    std::getline(inFile, line); //First two line is useless
    while (std::getline(inFile,line)) {
        std::vector<std::string> tokens;
        std::stringstream ss(line);
        std::string input;
        for (int i = 0 ; i < 8 ; i ++) {
            std::getline(ss, input, ',');
            if (input.find("※本公司每日凌晨0時") != std::string::npos) {
                cont = false;
                break;
            }
            tokens.push_back(input);
        }
        if (!cont) break;
        std::string year = tokens[0].substr(0,3);
        int nowYear = std::stoi(year) + 1911;
        std::string date = std::to_string(nowYear) + tokens[0].substr(4,2) + tokens[0].substr(7,2);
        std::string withdrawAmountStr = tokens[3];
        std::string depositAmountStr = tokens[4];
        if (!withdrawAmountStr.empty()) {
            std::erase(withdrawAmountStr, ',');
            std::cout << withdrawAmountStr << std::endl;
            double withdrawAmount = std::stod(withdrawAmountStr) * -1;
            tempRecords.push_back(std::make_shared<Transaction>("PS", date, "No Time", "No Category",withdrawAmount, ""));
        }
        else if (!depositAmountStr.empty()) {
            std::erase(depositAmountStr, ',');
            std::cout << depositAmountStr << std::endl;
            double depositAmount = std::stod(depositAmountStr);
            tempRecords.push_back(std::make_shared<Transaction> ("PS", date, "No Time", "No Category",depositAmount, ""));
        }
    }
    return tempRecords;
}