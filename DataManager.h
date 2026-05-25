#ifndef OOP_PROJECT_STORE_H
#define OOP_PROJECT_STORE_H
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "Transaction.h"


class DataManager {
private:
    static std::map<std::string, std::string> data;
public:
    static std::map<std::string, std::string> getCategories();

    static std::vector<std::shared_ptr<Transaction>> loadFromFile(const std::string& fileName);

    static void saveToFile(const std::vector<std::shared_ptr<Transaction>>& records, const std::string& fileName);

    static bool compare(const std::shared_ptr<Transaction> &a, const std::shared_ptr<Transaction> &b);

    static void categoryMapping (const std::shared_ptr<Transaction>&transaction);

    static void addCategory(const std::string &OriginCategory, const std::string &CategoryName);

    static void removeCategory(const std::string &category);
};


#endif //OOP_PROJECT_STORE_H