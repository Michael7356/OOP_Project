#ifndef OOP_PROJECT_PDFPARSER_H
#define OOP_PROJECT_PDFPARSER_H

#include <vector>
#include "Transaction.h"
#include <string>
#include <map>
#pragma once

class PdfParser {
private:
    static bool processLine(const std::string &line, Transaction& outTransaction);
    static std::map<std::string, std::string> categoryMap;

public:
    static std::vector<Transaction> parseBankStatement(const std::string& filepath, const std::string& password);
    static void addCategoryMapping(const std::string& keyword, const std::string& categoryName);

};


#endif //OOP_PROJECT_PDFPARSER_H