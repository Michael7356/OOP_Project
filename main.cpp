#include <vector>
#include <iostream>
#include "Transaction.h"
#include "PdfParser.h"
#include <poppler-document.h>
#include <poppler-page.h>
#include <windows.h>
#include <regex>
#include "httplib.h"
#include "json.hpp"
#include <fstream>

using json = nlohmann::json;

void menu() {
    std::cout << "Welcome to my bookkeeping." << std::endl;
    std::cout << "1. Add a transaction" << std::endl;
    std::cout << "2. Show all the transaction" << std::endl;
    std::cout << "3. Store the data of your bank account" << std::endl;
    std::cout << "4. Exit with store data" << std::endl;
    std::cout << "5. Exit without store data" << std::endl;
    std::cout << "Type 1-5 to using this program" << std::endl;
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
                Transaction t = PdfParser::resolvingRegex_Mail(item["message"]).value();
                temp.push_back(t);
            }
            Transaction::saveToFile(temp, config.csv_filename);
        }
        catch (const json::parse_error& e) {
            std::cerr << "JSON Parse Failed: " << e.what() << std::endl;
        }
        std::remove("temp_sync.json");
    }
}

bool callPython() {
    std::string Pypath = R"(..\Python_auto\.venv\Scripts\python.exe)";
    std::string script = "..\\Python_auto\\main.py";

    std::string command = Pypath + " " + script;
    std::cout << "Hold on a second" << std::endl;

    int result = std::system(command.c_str());
    if (result != 0) {
        std::cerr << "Failed to run script" << std::endl;
        std::cout << "Do you want to call it again? [y or n]" << std::endl;
        std::string input;
        while (std::cin >> input) {
            if(input == "y" || input == "Y" || input == "N"|| input == "n") {
                if (input == "y" || input == "Y") {
                    callPython();
                    break;
                }
                return false;
            }
            std::cout << "Not a valid choice" << std::endl;
        }
    }
    std::cout << "Success" << std::endl;
    return true;
}

int main() {

    SetConsoleOutputCP(CP_UTF8);

    const PdfParser::Config config = PdfParser::loadConfig();

    syncWithGoogle();

    std::vector<Transaction> myBookkeeping = PdfParser::loadFromFile(config.csv_filename);

    int choice = 0;
    while (choice != 4 && choice != 5) {
        menu();

        if (!(std::cin >> choice)) {
            std::cerr << "Invalid choice" << std::endl;
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            continue;
        }
        std::cin.ignore(1000, '\n');
        switch (choice) {
            case 1: {
                // std::string d,c,n,t;
                // double a;
                // std::cout << "Date (YYYY-MM-DD): "; std::cin >> d;
                // std::cout << "Time (HH:mm): "; std::cin >> t;
                // std::cout << "Category: "; std::cin >> c;
                // std::cout << "Amount: "; std::cin >> a;
                // std::cout << "Note: "; std::cin >> n;
                // myBookkeeping.emplace_back(d,t,c,a,n);
                // std::cout << "Complete"<< std::endl;
                if(callPython()){
                    std::string path = PdfParser::getCSVfile("downloads");
                    std::vector<receipt> receipt_all =  csvParser::loadFromFile(path);
                    receipt::saveToFile(receipt_all, config.csv_filename);
                    std::cout << "Alright, here's result" << std::endl;
                    double totalAmount = 0;
                    for (const auto& item : receipt_all) {
                        totalAmount += item.getAmount();
                        item.display();
                    }
                    std::cout << "Total amount: " << totalAmount << std::endl;
                }
                break;
                }

            case 2: {
                double total = 0;
                std::cout << "\nDate    Time    Category      Cost     | Note\n";
                for (const auto& record : myBookkeeping) {
                    record.display();
                    total +=  record.getAmount();
                }
                std::cout << "\nYou've spent " << total << std::endl;
                break;
            }

            case 3: {
                std::string password;
                std::cout << "Enter password: "; std::cin >> password;
                std::vector <Transaction> imported = PdfParser::parseBankStatement(config.deposit, password);
                std::cout << "Correct password! Now importing from "<< config.deposit << std::endl;
                if (imported.empty()) {
                    std::cerr << "Import failed" << std::endl;
                }
                else {
                    myBookkeeping.insert(myBookkeeping.end(), imported.begin(), imported.end());
                    std::cout << "Success" << std::endl;
                }
                break;
            }

            case 4: {
                Transaction::saveToFile(myBookkeeping, config.csv_filename);
                std::cout << "Complete"<< std::endl;
                break;
            }

            case 5: {
                std::cout << "Complete" <<std::endl;
                break;
            }
            default:
                std::cout << "Not a valid choice" << std::endl;
            }
    }

    return 0;
}