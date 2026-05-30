#ifndef OOP_PROJECT_DISPLAYUTIL_H
#define OOP_PROJECT_DISPLAYUTIL_H
#include <format>
#include <memory>
#include <vector>
#include "Transaction.h"


class DisplayUtil {
public:
    static void displayInList(const std::vector<std::shared_ptr<Transaction>> &records);

    static std::vector<std::string> deleteRecords(const std::vector<std::shared_ptr<Transaction>>& record);

    static void deleteMulti(const std::vector<std::string> &list, const std::vector<std::shared_ptr<Transaction>>& transactions);

    static int getVisualWidth(std::string str);

    static std::string formatOutput(std::string str, int length, bool leftAlign);
};


#endif //OOP_PROJECT_DISPLAYUTIL_H
