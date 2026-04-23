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

struct Config {
    std::string script_ID;
    int port;
    std::string csv_filename;
    std::string deposit;
};

Config loadConfig() {
    std::ifstream inFile("config.json");
    if (!inFile) {
        std::cerr << "hi" <<std::endl;
        return {"DEFAULT_ID", 8080};
    }
    json j;
    inFile >> j;
    return {j["google_script_id"], j["port"], j["csv_path"], j["deposit"]};
};

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

std::optional<Transaction> resolvingRegex_Mail(std::string smsText) {
    std::regex pattern (R"(.*?(\d{2}\/\d{2})\s{1}(\d{2}:\d{2}).*?\d{4}.*?(\d{1,6}).*?)");
    std::smatch match;
    Config config = loadConfig();
    if (std::regex_search(smsText, match, pattern)) {
        std::string date = match[1];
        std::string time = match[2];
        std::string amountStr = match[3];
        std::erase(amountStr, ',');
        double amount = std::stod(amountStr);
        Transaction t = {date, time, "", amount, ""};
        return t;
    }
    std::cout << "Can't resolve the format of message" << std::endl;
    return std::nullopt;
}

void syncWithGoogle() {
    Config config = loadConfig();
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
                Transaction t = resolvingRegex_Mail(item["message"]).value();
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

void runServer (httplib::Server* server) {
    std::cout <<  "server is start, wait for connection from phone" << std::endl;
    server->listen("0.0.0.0", 8080);
}

int main() {

    SetConsoleOutputCP(CP_UTF8);

    const Config config = loadConfig();

    httplib::Server server;
    server.Get("/", [](const httplib::Request&, httplib::Response& res) {
        res.set_content("<h1>Hello World! HJAAH<h1>", "text/html");
    });
    server.Post("/sms", [](const httplib::Request& req, httplib::Response& res) {
        try {
            auto j = json::parse(req.body);
            std::string smsText = j.at("message").get<std::string>();
            std::cout << "\nReceive message: " << smsText << std::endl;

            if (resolvingRegex_Mail(smsText)) {
                res.set_content("Succuss", "text/plain");
            }
            else {
                res.status = 400;
            }
        }catch (const std::exception& e) {
            std::cerr << "JSON resolving fail" << e.what() << std::endl;
            res.status = 400;
        }
    });

    std::thread serverThread(runServer, &server);
    serverThread.detach(); // Run in background

    syncWithGoogle();

    std::vector<Transaction> myBookkeeping = Transaction::loadFromFile(config.csv_filename);

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
                std::string d,c,n,t;
                double a;
                std::cout << "Date (YYYY-MM-DD): "; std::cin >> d;
                std::cout << "Time (HH:mm): "; std::cin >> t;
                std::cout << "Category: "; std::cin >> c;
                std::cout << "Amount: "; std::cin >> a;
                std::cout << "Note: "; std::cin >> n;
                myBookkeeping.emplace_back(d,t,c,a,n);
                std::cout << "Complete"<< std::endl;
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