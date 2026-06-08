#include "../include/httplib.h"
#include "../include/json.hpp"
#include <vector>
#include <iostream>
#include <windows.h>
#include <regex>
#include <fstream>
#include <conio.h>
#include "Transaction.h"
#include "PdfParser.h"
#include "DataManager.h"
#include "Deposit.h"
#include "DisplayUtil.h"


using json = nlohmann::json;

void menu() {
    std::cout << "Welcome to my bookkeeping." << std::endl;
    std::cout << "1.Transaction" <<std::endl;
    std::cout << "2.Category" << std::endl;
    std::cout << "3.Importing data" << std::endl;
    std::cout << "4.Refresh" << std::endl;
    std::cout << "5.Exit with store data" << std::endl;
    std::cout << "6.Exit without store data" << std::endl;
    std::cout << "Type 1-6 to use this program" << std::endl;
}

void syncWithGoogle() {
    PdfParser::Config config = PdfParser::loadConfig();
    std::cout << "Catching data from cloud" << std::endl;
    std::string url = "https://script.google.com/macros/s/" + config.script_ID + "/exec";
    std::string cmd = "curl -s -L -A \"Mozilla/5.0\" -o temp_sync.json \"" + url + "\"";
    system(cmd.c_str());

    std::ifstream inFile("temp_sync.json");
    if (inFile.is_open()) {
        std::string content((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
        inFile.close();
        if (!content.empty() && content[0] == '<') {
            std::cerr << "[!] Error: Captured HTML. Possible permission or script error." << std::endl;
            return;
        }
        try {
            auto data = json::parse(content);
            std::vector<Transaction> temp;
            for (auto& item : data) {
                try {
                    Transaction t = PdfParser::resolvingRegex_Mail(item["message"]);
                    temp.push_back(t);
                }catch (const std::exception& e) {
                    std::cerr << "Error happened at resolving regex [Error code:"<< e.what() << "]" << std::endl;
                }
            }
            try {
                Transaction::saveToFile(temp, config.csv_filename);
            }catch (const std::exception& e) {
                std::cerr << "Error happened at saveToFile [Error code: "<< e.what() << "]" << std::endl;
            }
        }
        catch (const json::parse_error& e) {
            std::cerr << "JSON Parse Failed: " << e.what() << std::endl;
        }
        std::remove("temp_sync.json");
    }
}

std::string getTodayDate() {
    auto now = std::chrono::system_clock::now();
    auto local_zone = std::chrono::current_zone();
    auto local_time = local_zone->to_local(now);
    return std::format("{:%Y%m%d}", local_time);
}

bool validInput(const std::string& prompt, const std::string& input) {
    // Use in check time and amount valid or not
    if (prompt[0] == 'D') {
        if (input.size() != 8) return false;
        if (!std::ranges::all_of(input, isdigit)) return false;

        int year = std::stoi(input.substr(0,4));
        int month = std::stoi(input.substr(4,2));
        int date = std::stoi(input.substr(6,2));

        if (year < 2000 || year > 2050) return false;
        if (month < 1 || month > 12) return false;
        int dateInYear[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
        if (year%4 == 0 && year % 100 != 0 || year % 400 == 0) dateInYear[2] = 29;
        if (date < 1 || date > dateInYear[month]) return false;
        return true;
    }
    if (prompt[0] == 'T') {
        if (input.size() != 5) return false;
        std::string temp = input.substr(0,2)+input.substr(3,2);
        if (!std::ranges::all_of(temp, isdigit)) return false;
        const int hour = std::stoi(input.substr(0,2));
        const int minute = std::stoi(input.substr(3,2));
        if (hour > 24 || hour < 0) return false;
        if (minute > 60 || minute < 0) return false;
        return true;
    }
    if (prompt[0] == 'A') {
        try {
            std::stoi(input);
        }catch (std::invalid_argument& e) {
            return false;
        }
        return true;
    }
    return true;
}

std::string defaultInput(const std::string& prompt, const std::string& defaultValue) {
    //Use in add transaction
    std::string input;
    std::cout << prompt << " Press 'Enter' to use default value: " << defaultValue << std::endl;
    if (std::getline(std::cin, input) && !input.empty() ) {
        if (validInput(prompt, input)) {
            return input;
        }
        std::cout << "This is not the valid input, please check the input format. (We would use the default value)" << std::endl;
        return defaultValue;
    }
    return defaultValue;
}

int main() {
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);

    DataManager::checkInitFile("Storage/config.json");
    DataManager::checkInitFile("Storage/Transaction.csv");
    DataManager::loadCategory("Storage/cate.json");
    const PdfParser::Config config = PdfParser::loadConfig();
    syncWithGoogle();

    std::vector<std::shared_ptr<Transaction>> myBookkeeping = DataManager::loadFromFile(config.csv_filename);
    int choice = 0;
    while (choice != 6 && choice != 5) {
        menu();
        std::cout << "===== This month you had spent: " << Transaction::calculateThisMonthAmount(myBookkeeping) << " dollars =====" << std::endl;
        if (!(std::cin >> choice)) {
            std::cerr << "Invalid choice" << std::endl;
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            continue;
        }
        std::ranges::sort(myBookkeeping, DataManager::compare);
        std::cin.ignore(1000, '\n');
        switch (choice) {
            case 1: { // Transaction
                std::cout << "======Transaction======" << std::endl;
                std::cout << "1. Add Transaction" << std::endl;
                std::cout << "2. Show Transaction" << std::endl;
                int input;
                std::cin >> input;
                switch (input) {
                    case 1: {
                        std::system("cls");
                        std::string d,c,n,t,aStr;
                        double a = 0;
                        char confirm = 'n';
                        if (std::cin.peek() == '\n')  std::cin.ignore();
                        do {
                            std::string today = getTodayDate();
                            d=defaultInput("Date[YYYYMMDD]: ", today);
                            t=defaultInput("Time[HH:MM]: ", "No Time");
                            c=defaultInput("Category: ", "No Category");
                            aStr=defaultInput("Amount(): ", "0");
                            a = std::stod(aStr);
                            std::cout << "Note: "; std::getline(std::cin>> std::ws, n);

                            std::cout << "\nPlease confirm the data you've input in " << std::endl;
                            std::cout << "Date: " << d
                                      << "\nTime: " << t
                                      << "\nCategory: " << c
                                      << "\nAmount: " << a
                                      << "\nNote: " << n << std::endl;
                            std::cout << "Is this correct data ? [y to complete / n to re-enter data / d to discard data(quit) ]" << std::endl;
                            confirm = getch();
                            if (confirm == 'd' || confirm == 'D') {
                                std::cout << "Data was discarded " << std::endl;
                                break;
                            }
                            if (confirm == 'y' || confirm == 'Y') {
                                break;
                            }
                            if (confirm == 'n' || confirm == 'N') {
                                system("cls");
                                std::cout << "[Re-enter data]" << std::endl;
                                continue;
                            }
                            std::cout << "Invalid input " << std::endl;
                            std::cout << "Data has been discarded" << std::endl;
                            break;
                        }while (true);
                        if (confirm == 'y' || confirm == 'Y') {
                            myBookkeeping.push_back(make_shared<Transaction>("Other", d, t, c, a, n));
                            std::cout << "Successfully add into data" << std::endl;
                        }
                        std::this_thread::sleep_for(std::chrono::seconds(1));
                        std::system("cls");
                        break;
                    }
                    case 2: {
                        std::cout << "Choose a record type to display" << std::endl;
                        std::cout << "1. Receipt record" << std::endl;
                        std::cout << "2. Detailed record" << std::endl;
                        int tempInput;
                        std::cin >> tempInput;
                        switch (tempInput) {
                            case 1: {
                                std::vector<std::shared_ptr<Transaction>> temp = receipt::getSimpleRecords(myBookkeeping);
                                DisplayUtil::displayInList(temp);
                                break;
                            }
                            case 2: {
                                DisplayUtil::displayInList(myBookkeeping);
                                break;
                            }
                            default: {
                                std::cout << "Invalid input" << std::endl;
                                std::system("cls");
                            }
                        }
                        break;
                    }
                    default:
                        std::cout << "Invalid input" << std::endl;
                }
                break;
            }

            case 2: { // Category
                std::cout << "======Category======" << std::endl;
                std::cout << "1. Add Category" << std::endl;
                std::cout << "2. Show Category" << std::endl;
                int input;
                std::cin >> input;
                switch (input) {
                    case 1: {
                        std::string input1;
                        char chInput;
                        do {
                            std::string oc, cn;
                            std::cout << "Do you want to add from the category of file ? [y or n] " << std::endl;
                            chInput = getch();
                            std::map<std::string, std::string> loadedCategories = DataManager::getCategories();
                            if (chInput == 'y' || chInput == 'Y') {
                                std::set<std::string> categories;
                                for (const auto& m : myBookkeeping) {
                                    bool contain = false;
                                    for (const auto& [fst, snd] :loadedCategories) {
                                        if (fst == m->getCategory() || snd == m->getCategory()) {
                                            contain = true;
                                            break;
                                        }
                                    }
                                    if (!contain) categories.insert(m->getCategory());
                                }
                                int count = 0;
                                std::cout << "Here are the categories that you could choose to add in category map:" << std::endl;
                                for (const auto& index : categories) {
                                    std::cout << std::right << std::setw(3) << count << ". " << index << std::endl;
                                    count++;
                                }
                                std::string intInput;
                                std::cout << "Choose a category you want to link:" << std::endl;
                                std::cin >> intInput;
                                if (std::ranges::all_of(intInput, isdigit) && 0<=stoi(intInput) && stoi(intInput) < categories.size()) {
                                    int index = std::stoi(intInput);
                                    oc = *std::next(categories.begin(), index);
                                }
                                else {
                                    std::cout << "Invalid input" << std::endl;
                                    break;
                                }
                                std::cout << "What does this category link (" << oc << ") to ?" << std::endl;
                                std::cin>>cn;
                            }
                            else {
                                std::cout << "What does this category call ?" << std::endl;
                                std::cin >> oc;
                                if (std::cin.peek() == '\n')  std::cin.ignore();
                                cn = defaultInput("What does this category link to ?", oc);
                            }
                            std::cout << "Old category name: "<< oc << "\nNew category name:  " << cn << std::endl;
                            std::cout << "Is the data correct? [y or n]" << std::endl;
                            chInput = getch();
                            if (chInput == 'y' || chInput == 'Y') {
                                DataManager::addCategory(oc, cn);
                                DataManager::categoryMapping(myBookkeeping);
                            }
                            else if (chInput == 'N' || chInput == 'n') {
                                std::cout << "Discard the data" << std::endl;
                                std::system("cls");
                            }
                            std::cout << "Do you want to add more category ? [y to continue / other keys quit]" << std::endl;
                            chInput = getch();
                        }while (chInput == 'y' || chInput == 'Y');
                        break;
                    }
                    case 2: {
                        std::system("cls");
                        std::map<std::string, std::string> categories = DataManager::getCategories();
                        std::vector<std::string> categoryKeys;
                        int count = 0;
                        for (const auto& [fst, snd] : categories) {
                            std:: cout << count << ". " << fst << " is linked to " << snd << std::endl;
                            categoryKeys.push_back(fst);
                            count++;
                        }
                        char chInput;
                        std::cout << "Press 'e' to erase link or press any key to quit" << std::endl;
                        chInput = getch();
                        if (chInput == 'e' || chInput == 'E') {
                            std::string tempInput, temp;
                            std::cout << "Choose one or multiple category to delete [e.g. 1 2 31 231]" << std::endl;
                            if (std::cin.peek() == '\n')  std::cin.ignore();
                            std::getline(std::cin, tempInput);
                            std::stringstream ss(tempInput);
                            std::set<std::string> categoriesDeleted;
                            while (ss >> temp) {
                                int index;
                                if (std::ranges::all_of(temp, isdigit)) index = std::stoi(temp);
                                else {
                                    std::cout << temp << " is not a valid index" << std::endl;
                                    continue;
                                }
                                if (0 <= index && index < categoryKeys.size()) {
                                    if (categoriesDeleted.insert(categoryKeys[index]).second) DataManager::removeCategory(categoryKeys[index]);
                                }
                                else std::cout << temp << " is not a valid index" << std::endl;
                            }
                            if (!categoriesDeleted.empty()) std::cout << "You had deleted";
                            for (const auto& dele : categoriesDeleted) {
                                 std::cout << " " << dele;
                            }
                            std::cout << "\nBack to main menu in 2 seconds" << std::endl;
                            std::this_thread::sleep_for(std::chrono::seconds(1));
                            std::cout <<"Back to main menu in 1 second" << std::endl;
                            std::this_thread::sleep_for(std::chrono::seconds(1));
                        }
                        break;
                    }
                    default:
                        std::cout << "Invalid input" << std::endl;
                }
                std::system("cls");
                break;
            }

            case 3: { //Importing data
                std::cout << "======Importing Data======" << std::endl;
                std::cout << "Choose one source to download the data" << std::endl;
                std::cout << "1.Bank and IPass" << std::endl;
                std::cout << "2.Receipt" << std::endl;
                int input;
                std::cin >> input;
                switch (input) {
                    case 1: {
                        std::vector<std::shared_ptr<Transaction>> temp;
                        std::shared_ptr<Deposit> deposit = Deposit::deposit_ptr();
                        temp = deposit->get_Record();
                        if (dynamic_cast<POST*>(deposit.get())) {
                            DataManager::categoryMapping(temp);
                            POST::mergeOriginal(temp, myBookkeeping);
                        }
                        else {
                            for (const auto& a : temp) {
                                if (dynamic_cast<IPass*>(deposit.get())) {
                                    if (IPass::checkRecord(a, myBookkeeping)){
                                        continue;
                                    }
                                }
                                myBookkeeping.push_back(a);
                            }
                        }
                        DisplayUtil::displayInList(temp);
                        break;
                    }
                    case 2: {
                        std::cout << "Choose one way to download the data " << std::endl;
                        std::cout << "1. Automatically crawl data   2.Add it manually(.csv file)" << std::endl;
                        int inputNum; std::cin >> inputNum;
                        std::string path;
                        switch (inputNum) {
                            case 1: {
                                if(Deposit::callPython("CaptureReceipt.py"))  path = PdfParser::getCSVfile("downloads");
                                break;
                            }
                            case 2: {
                                path = Deposit::getFilePathWithWindow("CSV");
                                break;
                            }
                            default:
                                std::cout << "Invalid input" << std::endl;
                                std::system("cls");
                                break;
                        }
                        if (!path.empty()) {
                            std::vector<std::shared_ptr<Transaction>> receipt_all =  csvParser::loadFromFile(path);
                            receipt::checkUnique(receipt_all, config.csv_filename);
                            for (const auto& r : receipt_all) {
                                myBookkeeping.push_back(r);
                            }
                            std::cout << "Alright, here's result" << std::endl;
                            std::vector<std::shared_ptr<Transaction>> temp;
                            double totalAmount = 0;
                            for (const auto& item : receipt_all) {
                                totalAmount += item->getAmount();
                                temp.push_back(item);
                            }
                            std::cout << totalAmount << std::endl;
                            DisplayUtil::displayInList(temp);
                        }
                        break;
                    }
                    default: {
                        std::cout << "Not a valid choice" << std::endl;
                        std::system("cls");
                    }
                }
                break;
            }

            case 4: {// Refresh
                DataManager::categoryMapping(myBookkeeping);
                receipt::matchingRecords(myBookkeeping);
                std::ranges::sort(myBookkeeping, DataManager::compare);
                std::system("cls");
                std::cout << "[Refresh completed]" << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                break;
            }

            case 5: { //store data
                std::ranges::sort(myBookkeeping, DataManager::compare);
                DataManager::saveToFile(myBookkeeping, config.csv_filename);
                DataManager::saveCategory("Storage/cate.json");
                std::cout << "Complete"<< std::endl;
                std::cout << "Program end in 2 seconds" << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(1));
                std::cout << "Program end in 1 seconds" << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(1));
                break;
            }

            case 6: { // don't store data
                std::cout << "Complete" <<std::endl;
                std::cout << "Program end in 2 seconds" << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(1));
                std::cout << "Program end in 1 seconds" << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(1));
                break;
            }

            default:
                std::cout << "Not a valid choice" << std::endl;
            }
    }

    return 0;
}