#include <vector>
#include <iostream>
#include "Transaction.h"
#include "PdfParser.h"
#include <poppler-document.h>
#include <poppler-page.h>
#include <windows.h>
#include <regex>

void menu() {
    std::cout << "Welcome to my bookkeeping." << std::endl;
    std::cout << "1. Add a transaction" << std::endl;
    std::cout << "2. Show all the transaction" << std::endl;
    std::cout << "3. Store the data of your bank account" << std::endl;
    std::cout << "4. Exit with store data" << std::endl;
    std::cout << "5. Exit without store data" << std::endl;
    std::cout << "Type 1-5 to using this program" << std::endl;
}

void extractFromFile(const std::string& filepath, const std::string& password) {
    poppler::document* doc = poppler::document::load_from_file(filepath, password);
    if (!doc) {
        std::cerr << "Could not load file " << filepath << std::endl;
        return;
    }

    for (int i = 0 ; i < doc->pages() ; ++i) {
        poppler::page* p = doc->create_page(i);
        if (p) {
            std::string pageText = p->text().to_utf8().data();
            std::cout << "Page: " << i + 1 << std::endl;
            std::cout << pageText << std::endl;
            delete p;
        }
    }
    delete doc;
}

int main() {

    SetConsoleOutputCP(CP_UTF8);

    std::vector<Transaction> myBookkeeping = Transaction::loadFromFile("Something.csv");

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
                std::string d,c,n;
                double a;
                std::cout << "Date (YYYY-MM-DD): "; std::cin >> d;
                std::cout << "Category: "; std::cin >> c;
                std::cout << "Amount: "; std::cin >> a;
                std::cout << "Note: "; std::cin >> n;
                myBookkeeping.emplace_back(d,c,a,n);
                std::cout << "Complete"<< std::endl;
                break;
                }

            case 2: {
                double total = 0;
                std::cout << "\nDate        Category      Cost     | Note\n";
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
                std::vector <Transaction> imported = PdfParser::parseBankStatement("Deposit-record.pdf", password);
                std::cout << "Correct password! Now importing from Deposit-record.pdf" << std::endl;
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
                Transaction::saveToFile(myBookkeeping, "Something.csv");
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