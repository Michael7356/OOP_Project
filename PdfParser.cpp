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

using json = nlohmann::json;

struct Config {
    std::string script_ID;
    int port;
    std::string csv_filename;
    std::string deposit;
};

PdfParser::Config PdfParser::loadConfig() {
    std::ifstream inFile("config.json");
    if (!inFile) {
        std::cerr << "hi" <<std::endl;
        return {"DEFAULT_ID", 8080};
    }
    json j;
    inFile >> j;
    return {j["google_script_id"], j["port"], j["csv_path"], j["deposit"]};
};

std::vector<Transaction> PdfParser::parseBankStatement(const std::string& filepath, const std::string& password) {
    std::vector<Transaction> result;
    const poppler::document *doc = poppler::document::load_from_file(filepath, password);
    if (!doc) {
        std::cerr << "Failed to load document: " << filepath << std::endl;
        return result;
    }

    for (int i = 0 ; i < doc->pages(); ++i) {
        poppler::page* p = doc->create_page(i);
        if (p) {
            std::string pageText = p->text().to_utf8().data();
            std::stringstream ss(pageText);
            std::string line;

            while (getline(ss, line)) {
                Transaction t("", "", "", 0.0, "");
                if (processLine(line, t)) {
                    result.push_back(t);
                }
            }
            delete p;
        }
    }
    delete doc;
    return result;
}

std::map <std::string, std::string> PdfParser::categoryMap = {
    {"國外交易手續費","Handling fee"},
    {"ＧＯＯＧＬＥ","Google"},
    {"全家","全家"},
    {"全聯","全聯"},
    {"美聯社","美聯社"},
    {"ＡＰＰＬＥ", "Apple"},
    {"ＳＴＥＡＭ", "Steam"}
};

bool PdfParser::processLine(const std::string& line, Transaction& outTransaction) {
    const std::regex pattern(R"((\d{4}/\d{2}/\d{2}\s+\d{2}:\d{2}:\d{2})\s{3}([^\s-]+)?.*?(\d{1,3}(?:,\d{3})?).*?(\d{1,3}(?:,\d{3})?)\s{3}([^\s-]+)?\s+([^\s-]+|-)?\s+([^\s-]+)?)");
    std::smatch match;
    if (std::regex_search(line, match, pattern)) {
        std::string dateAndTime = match[1];
        std::string date = match[1].str().substr(0,10);
        std::string time = match[1].str().substr(11,5);
        std::string summary = match[2];
        std::string amount_str = match[3];
        std::string balance = match[4];
        std::string note = match[5];
        std::string other_acc = match[6];
        std::string description = match[7];



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

            std::string category = (description.find("ＧＯＯＧＬＥ") != std::string::npos) ? "Subscription" : "General";
            outTransaction = Transaction(date, time, category, amount, description);
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
        std::string time = match[2];
        std::string amountStr = match[3];
        std::erase(amountStr, ',');
        double amount = std::stod(amountStr);
        Transaction t = {date, time, "", amount, ""};
        return t;
    }
    std::cout << "Can't resolve the format of message" << std::endl;
    return std::nullopt;
}