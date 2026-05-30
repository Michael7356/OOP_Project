#include "DisplayUtil.h"
#include <iostream>
#include <vector>
#include <memory>
#include <iomanip>
#include <string>
#include <sstream>
#include <unordered_set>
#include <ranges>
#include <algorithm>
#include <functional>
#include <chrono>
#include <thread>
#include <conio.h>

void DisplayUtil::displayInList(const std::vector<std::shared_ptr<Transaction>>& records) {
    int RecordSize = records.size(), page = 1, maxPage = (RecordSize + 10 - 1) / 10;
    double totalAmount = 0;
    char chInput = 's';
    do {
        if (chInput == 'q' || chInput == 'Q') break;
        if ((chInput == 'l' || chInput == 'L') && page > 1) page --;
        else if ((chInput == 'n' || chInput == 'N') && page < maxPage) page ++;
        else {
            if (chInput != 's') std::cout << "Invalid input [Reach the max or min page]" << std::endl;
        }
        std::cout << std::left << std::setw(12) << "type" <<std::setw(12) << "date" << std::setw(10) << "time" << std::setw(20) <<"category" << std::right << std::setw(6) <<"amount" << std::setw(6) <<" | note" << std::endl;
        int index = (page - 1) * 10;
        for (int i = index ; i <std::ranges::min(page*10, RecordSize)  ; i ++) {
            if (records[i]) {
                std::cout << std::right << std::setw(4) << i << ": " ;
                records[i]->display();
            }
        }
        std::cout << "Page: " << page << "/" << maxPage<<  std::endl;
        std::cout << "[Press 'n' to next page, 'l' to last page, 'd' to delete data or press q to quit]" << std::endl;
        chInput = getch();
        if (chInput == 'd' || chInput == 'D') {
            std::string deleteList;
            std::cout << "Choose one or multiple things to delete [e.g. 1 2 31 231]" ;
            std::string delIndex, temp;
            std::getline(std::cin, delIndex);
            std::stringstream ss(delIndex);
            std::unordered_set<int> check;
            while (ss >> temp) {
                if (std::ranges::all_of(temp, isdigit)) {
                    if (check.insert(std::stoi(temp)).second) {
                        if (0 <= stoi(temp) && stoi(temp) < records.size()) {
                            if (records[stoi(temp)]->getType() == "Receipt(comp)") records[stoi(temp)]->editType("deleted(comp)");
                            else records[stoi(temp)]->editType("deleted");
                            deleteList += " " + temp;
                        }
                        else {
                            std::cout << temp << " is not a valid index" << std::endl;
                        }
                    }
                }
                else {
                    std::cout << "This is not a valid index" << std::endl;
                }
            }
            if (!deleteList.empty()) std::cout << "You have deleted" << deleteList << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
        std::system("cls");
    }while (chInput);
}

std::vector<std::string> DisplayUtil::deleteRecords(const std::vector<std::shared_ptr<Transaction>>& record) {
    std::vector<std::string> deleteList;
    for (const auto& delRecord: record) {
        auto rptr = dynamic_cast<receipt*>(delRecord.get());
        if (rptr) {
            if (rptr->getType() == "deleted(comp)") {
                deleteList.push_back(rptr->getReceiptNumber());
                rptr->editType("deleted");
            }
        }
    }
    return deleteList;
}

void DisplayUtil::deleteMulti(const std::vector<std::string> &list, const std::vector<std::shared_ptr<Transaction>>& transactions) {
    for (const auto& delRecord: list) {
        for (const auto& transaction: transactions) {
            auto rptr = dynamic_pointer_cast<receipt>(transaction); //shared_ptr is not an object so couldn't use dynamic_cast
            if (rptr) {
                std::string target = delRecord;
                if (delRecord == rptr->getReceiptNumber()) {
                    rptr->editType("deleted");
                }
            }
        }
    }
}