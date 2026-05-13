#ifndef OOP_PROJECT_PDFPARSER_H
#define OOP_PROJECT_PDFPARSER_H

#include <vector>
#include "Transaction.h"
#include <string>
#include <map>
#include <optional>
#pragma once

class PdfParser {
private:
    static bool processLine(const std::string &line, Transaction& outTransaction);
    static std::map<std::string, std::string> categoryMap;

public:
    static std::vector<Transaction> parseBankStatement(const std::string& filepath, const std::string& password);

    static void addCategoryMapping(const std::string& keyword, const std::string& categoryName);

    static std::optional<Transaction> resolvingRegex_Mail(std::string smsText);

    struct Config {
        std::string script_ID;
        int port;
        std::string csv_filename;
        std::string deposit;
    };

    static Config loadConfig();

    static std::string getCSVfile(const std::string& dirPath);

    static std::vector<Transaction> loadFromFile(const std::string &filename);
};

class csvParser : public PdfParser {
public:
    static std::vector<receipt> loadFromFile (const std::string &filename) ;
};


#endif //OOP_PROJECT_PDFPARSER_H