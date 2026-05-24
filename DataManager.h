#ifndef OOP_PROJECT_STORE_H
#define OOP_PROJECT_STORE_H
#include <memory>
#include <string>
#include <vector>

#include "Transaction.h"


class DataManager {
public:
    static std::vector<std::shared_ptr<Transaction>> loadFromFile(const std::string& fileName);

    static void saveToFile(const std::vector<std::shared_ptr<Transaction>>& records, const std::string& fileName);

    static bool compare(const std::shared_ptr<Transaction> &a, const std::shared_ptr<Transaction> &b);
    
};


#endif //OOP_PROJECT_STORE_H