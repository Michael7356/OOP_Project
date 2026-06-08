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

#include "Deposit.h"
#include "../include/httplib.h"
#include "DataManager.h"

void DisplayUtil::displayInList(const std::vector<std::shared_ptr<Transaction>>& records) {
    std::system("cls");
    int RecordSize = records.size(), page = 1, maxPage = (RecordSize + 10 - 1) / 10;
    char chInput = '-';
    if (RecordSize == 0) {
        std::cout << "[No data here]" << std::endl;
        std::cout << "Press any key to quit" << std::endl;
        chInput = getch();
        std::system("cls");
        return;
    }
    do {
        if (chInput == 'q' || chInput == 'Q') break;

        if ((chInput == 'l' || chInput == 'L') && page > 1) page --;

        else if ((chInput == 'n' || chInput == 'N') && page < maxPage) page ++;

        else if (chInput == 'f' || chInput == 'F') {
            std::vector<std::shared_ptr<Transaction>> tempRecord;
            std::string input;
            std::cout << "Type the data you want to show" << std::endl;
            std::cout << "1.Receipt     2.CTBC      3.POST OFFICE     4.IPass"  << std::endl;
            if (std::cin.peek() == '\n')  std::cin.ignore();
            std::getline(std::cin, input);
            std::stringstream ss(input);
            std::string temp;
            while (ss >> temp) {
                std::vector<std::shared_ptr<Transaction>> record;
                if (temp == "1") {
                    record = receipt::getSimpleRecords(records);
                }
                else if (temp == "2") {
                    record = CTBC::find_CTBC_Record(records);
                }
                else if (temp == "3") {
                    record = POST::find_POST_Record(records);
                }
                else if (temp == "4") {
                    record = IPass::find_IPass_Record(records);
                }
                else {
                    std::cout << "Invalid input [" << temp << "]" << std::endl;
                    continue;
                }
                tempRecord.insert(tempRecord.end(), record.begin(), record.end());
            }
            std::ranges::sort(tempRecord, DataManager::compare);
            displayInList(tempRecord);
            return ;
        }

        else if (chInput != '-') {
            std::cout << "Invalid input [Reach the max or min page]" << std::endl;
        }

        std::cout << std::left << std::setw(8) << "Index" << std::setw(21) << "type" <<std::setw(12) << "date" << std::setw(10) << "time" << std::setw(22) <<"category" << std::right << std::setw(8) <<"amount | note" << std::endl;
        int index = (page - 1) * 10;
        for (int i = index ; i <std::ranges::min(page*10, RecordSize)  ; i ++) {
            if (records[i]) {
                std::cout << std::right << std::setw(4) << i << ": " ;
                records[i]->display();
            }
        }
        std::cout << "Page: " << page << "/" << maxPage<<  std::endl;
        std::cout << "[Press 'n' to next page, 'l' to last page, 'd' to delete data, 'f' to filter data, 's' to search data or press q to quit]" << std::endl;
        chInput = getch();
        if (chInput == 'd' || chInput == 'D') {
            chInput = '-';
            std::string deleteList;
            std::string delIndex, temp;
            std::cout << "Choose one or multiple things to delete [e.g. 1 2 31 231]" ;
            if (std::cin.peek() == '\n')  std::cin.ignore();
            std::getline(std::cin, delIndex);
            std::stringstream ss(delIndex);
            std::unordered_set<int> check;
            while (ss >> temp) {
                if (std::ranges::all_of(temp, isdigit)) {
                    if (check.insert(std::stoi(temp)).second) {
                        if (0 <= stoi(temp) && stoi(temp) < records.size()) {
                            if (records[stoi(temp)]->getType() == "Receipt(comp)") {
                                auto  rptr = dynamic_cast<receipt*>(records[stoi(temp)].get());
                                receipt::deleteReceipt(rptr->getReceiptNumber(), records);
                            }
                            else records[stoi(temp)]->editType("Deleted");
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
        else if (chInput == 'S' || chInput == 's') {
            std::string input;
            std::cout << "[Search Mode]"<< std::endl;
            std::cout << "Search: ";
            if (std::cin.peek() == '\n')  std::cin.ignore();
            std::getline(std::cin, input);
            std::vector<std::shared_ptr<Transaction>> tempRecord;
            for (const auto& record : records) {
                if (record->getNote().find(input) != std::string::npos || record->getCategory().find(input) != std::string::npos) {
                    tempRecord.push_back(record);
                }
            }
            displayInList(tempRecord);
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
                rptr->editType("Deleted");
            }
        }
    }
    return deleteList;
}

int DisplayUtil::getVisualWidth(const std::string str) {
    int width = 0;
    for (size_t i = 0 ; i < str.length();) {
        unsigned char ch = str[i];
        if (ch <= 127) { //ASCII code 0~127
            width += 1;
            i ++;
        }
        else if ((ch & 0xE0) == 0xE0) {
            width += 2; // Full size character
            i += 2;
        }
        else if ((ch & 0xF0) == 0xF0) {
            width += 2; // Chinese width
            i += 3;
        }
        else if ((ch & 0xF8) == 0xF8) {
            width += 2;
            i += 4;
        }
        else {
            i ++;
        }
    }
    return width;
}

std::string DisplayUtil::formatOutput(const std::string str, const int length, bool leftAlign) { //Another version of setw() but fit Chinese words
    int currentLength = getVisualWidth(str);
    int spaceNeeded = length - currentLength;
    if (spaceNeeded <= 0) return str;

    std::string spaces(spaceNeeded, ' ');
    if (leftAlign) {
        return str + spaces;
    }
    else {
        return spaces + str;
    }

}
