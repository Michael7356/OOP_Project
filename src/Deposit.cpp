#include "Deposit.h"
#include "../include/httplib.h"
#include "PdfParser.h"
#include "DataManager.h"

void Deposit::display() const{
    for (const auto& record : this->transactions) {
        record->display();
    }
}

std::string Deposit::getFilePathWithWindow(const std::string &fileType){
    OPENFILENAMEA ofn;
    char szFile[260] = {0};//Buffer of store the path to the file
    const char* filter = "All Files (*.*)\0*.*\0";
    if (fileType == "CSV") {
        filter = "CSV Files (*.csv)\0*.csv\0All Files (*.*)\0*.*\0";
    }
    else if (fileType == "PDF") {
        filter = "PDF Files (*.pdf)\0*.pdf\0All Files (*.*)\0*.*\0";
    }

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = nullptr;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = filter;
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = nullptr;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = "."; //Default folder when open
    ofn.lpstrTitle = "Choose the records";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetOpenFileName(&ofn) == TRUE) {
        return std::string (ofn.lpstrFile);
    }
    return "";
}

std::shared_ptr<Deposit> Deposit::deposit_ptr(){
    std::cout << "Choose one source to add" << std::endl;
    std::cout << "1.CTBC(中國信託)" << std::endl;
    std::cout << "2.PS(中華郵政)" << std::endl;
    std::cout << "3.IPass(一卡通)" << std::endl;
    int input;
    std::cin >> input;
    switch (input) {
        case 1: {
            std::string password;
            std::cout << "Please insert the file password (Default is your ID)" << std::endl;
            std::cin >> password;
            return std::make_shared<CTBC>(password);
        }
        case 2: {
            std::string password;
            std::cout << "Please insert the file password (Default is your ID)" << std::endl;
            std::cin >> password;
            return std::make_shared<POST>(password);
        }
        case 3: {
            return std::make_shared<IPass>();
        }
        default :
            std::cerr << "Invalid input" << std::endl;
            break;
    }
    return nullptr;
}

bool Deposit::checkUnique(const std::shared_ptr<Transaction> &record, const std::vector<std::shared_ptr<Transaction>> &transactions) {
    for (const auto& transaction : transactions) {
        if (transaction == record) return false;
    }
    return true;
}

bool Deposit::callPython(std::string filename) {
    std::string command = "uv run --with selenium --with beautifulsoup4 --with webdriver-manager --with ddddocr --with requests ..\\Python_auto\\" + filename;
    std::cout << "Hold on a second" << std::endl;

    int result = std::system(command.c_str());
    if (result != 0) {
        std::cerr << "Failed to run script" << std::endl;
        std::cout << "Make sure your account and password are correct" << std::endl;
        std::cout << "Do you want to call it again? [y or n]" << std::endl;
        std::string input;
        while (std::cin >> input) {
            if(input == "y" || input == "Y" || input == "N"|| input == "n") {
                if (input == "y" || input == "Y") {
                    return callPython(filename);
                }
                return false;
            }
            std::cout << "Not a valid choice" << std::endl;
        }
    }
    std::cout << "Success" << std::endl;
    return true;
}

void CTBC::set_password(const std::string& password) {
    this->password = password;
}

std::vector<std::shared_ptr<Transaction>> CTBC::get_Record() const {
    std::vector<std::shared_ptr<Transaction>> tempRecord;
    std::string filePath = getFilePathWithWindow("PDF");
    if (!filePath.empty()) {
        try {
           tempRecord = PdfParser::parseBankStatement(filePath, this->password, "CTBC");
        }catch(std::runtime_error& e) {
            std::cerr << "Failed, check if your .pdf file is correct one [Error code:" << e.what() << "]" << std::endl;
        }
    }
    else {
        std::cerr << "Canceled the load" << std::endl;
    }
    return tempRecord;
}

std::vector<std::shared_ptr<Transaction>> CTBC::find_CTBC_Record(const std::vector<std::shared_ptr<Transaction>> &transactions) {
    std::vector<std::shared_ptr<Transaction>> tempRecord;
    for (const auto& transaction : transactions) {
        if (transaction->getType().find("CTBC") != std::string::npos) {
            tempRecord.push_back(transaction);
        }
    }
    return tempRecord;
}

std::vector<std::shared_ptr<Transaction>> POST::get_Record() const {
    std::vector<std::shared_ptr<Transaction>> tempRecord;
    std::cout << "Please choose the file that going to be added in." << std::endl;
    std::cout << "1.PDF file   2.CSV file" << std::endl;
    std::string input; std::cin >> input;
    if (input == "1") {
        std::string filePath = getFilePathWithWindow("PDF");
        if (!filePath.empty()) {
            try {
                tempRecord = PdfParser::parseBankStatement(filePath, this->password, "POST");
            }catch (const std::runtime_error& e) {
                std::cerr << "Failed, check if your .csv file is correct one [Error code:" << e.what() << "]" << std::endl;
            }
        }
    }
    else if (input == "2") {
        std::string filePath = getFilePathWithWindow("CSV");
        if (!filePath.empty()) {
            try {
                tempRecord = csvParser::loadFromFile_PS(filePath);
            }catch (const std::runtime_error& e) {
                std::cerr << "Failed, check if your .csv file is correct one [Error code:" << e.what() << "]" << std::endl;
            }
        }
    }
    else {
        std::cerr << "Invalid input" << std::endl;
    }
    return tempRecord;
}

std::vector<std::shared_ptr<Transaction>> POST::find_POST_Record(const std::vector<std::shared_ptr<Transaction>> &transactions) {
    std::vector<std::shared_ptr<Transaction>> tempRecord;
    for (const auto& transaction : transactions) {
        if (transaction->getType().find("POST") != std::string::npos) {
            tempRecord.push_back(transaction);
        }
    }
    return tempRecord;
}

bool POST::matchUncategorized(const std::shared_ptr<Transaction> &transaction, const std::vector<std::shared_ptr<Transaction> > &transactions) {
    for (const auto& record : transactions) {
        if (record->getType() == "POST[Unconfirmed]") {
            int diff = Transaction::compareDate(transaction, record);
            if (diff >= 0 && diff <= 7) {
                if (transaction->getAmount() == record->getAmount()) {
                    record->editType("POST[Uncategorized]");
                    return true;

                }
            }
        }
        else if (record->getType() == "POST[Unmatched]") {
            std::string Ddate = record->getDate().substr(9,8); // Deduction date
            if (transaction->getAmount() == record->getAmount() && Ddate == transaction->getDate()) {
                transaction->editType("Deleted");
                return true;
            }
;       }
        else if (record->getType() == "Receipt(POST)") {
            std::string Ddate = record->getDate().substr(9,8); // Deduction date
            if (transaction->getAmount() == record->getAmount() && Ddate == transaction->getDate()) {
                transaction->editType("Deleted");
                return true;
            }
        }
    }
    return false;
}

bool POST::matchUnmatched(const std::shared_ptr<Transaction> &transaction, const std::vector<std::shared_ptr<Transaction>> &transactions) {
    std::string date = transaction->getDate();
    std::string Tdate = date.substr(0,8);// Transaction date
    std::string Ddate = date.substr(9,8);// Deduction date
    for (const auto& record : transactions) {
        if (record->getType() == "POST[Uncategorized]") {
            if (record->getTime() == "No Time") {
                if (Ddate == record->getDate() && transaction->getAmount() == record->getAmount()) {
                    record->editType("POST[Unmatched]");
                    record->editDate(transaction->getDate());
                    record->editCategory(transaction->getCategory());
                    return true;
                }
            }
            else {
                if (Tdate == record->getDate() && transaction->getAmount() == record->getAmount()) {
                    record->editType("POST[Unmatched]");
                    record->editDate(transaction->getDate());
                    record->editCategory(transaction->getCategory());
                    return true;

                }
            }
        }
        else if (record->getType() == "POST[Unconfirmed]") {
            if (Tdate == record->getDate() && transaction->getAmount() == record->getAmount()) {
                record->editType("POST[Unmatched]");
                record->editCategory(transaction->getCategory());
                return true;
            }
        }
        else if (record->getType() == "Receipt(POST)") {
            if (Ddate == record->getDate() && record->getAmount() == transaction->getAmount() && transaction->getCategory() == record->getCategory()) {
                transaction->editType("Deleted");
            }
        }
    }
    return false;
}

void POST::mergeOriginal(std::vector<std::shared_ptr<Transaction>>& addInTransactions, std::vector<std::shared_ptr<Transaction>>& originalTransaction) {
    std::vector<std::shared_ptr<Transaction>> tempTransactions;

    for (const auto& transaction : originalTransaction) {
        std::string type = transaction->getType();
        if (type.find("POST") != std::string::npos) {
            tempTransactions.push_back(transaction);
        }
    }
    std::ranges::sort(addInTransactions, DataManager::compare);
    std::ranges::sort(tempTransactions, DataManager::compare);
    for (const auto& transaction : addInTransactions) {
        bool containOrNot = false;
        if (transaction->getType() == "POST[Uncategorized]") {
            containOrNot = matchUncategorized(transaction, tempTransactions);
        }
        else if (transaction->getType() == "POST[Unmatched]") {
            containOrNot = matchUnmatched(transaction, tempTransactions);
        }
        if (!containOrNot && checkUnique(transaction, originalTransaction)) {
            originalTransaction.push_back(transaction);
            tempTransactions.push_back(transaction);
        }
    }
}

std::vector<std::shared_ptr<Transaction>> IPass::get_Record() const {
    std::vector<std::shared_ptr<Transaction>> tempRecord;
    std::string path;
    if (callPython("iPass.py")) path = PdfParser::getCSVfile("downloads");
    if (!path.empty()) {
        std::ifstream inFile(path);
        if (!inFile.is_open()) {
            std::cerr << "Error at iPass get record" << std::endl;
        }
        std::string line;
        std::getline(inFile, line); // First line is not our target
        while (std::getline(inFile, line)) {
            std::vector<std::string> tokens;
            std::stringstream ss (line);
            std::string input;
            for (int i = 0 ; i < 6 ; i ++) {
                std::getline(ss, input, ',');
                tokens.push_back(input);
            }
            double amount = 0;
            try {
                amount = std::stod(tokens[5]);
            }catch(std::invalid_argument& e) {
                std::cerr << e.what() << std::endl;
            }
            if (tokens[3] != "里程下車" && tokens[3] != "里程上車" && tokens[4].find("客運") == std::string::npos && tokens[4].find("捷運") == std::string::npos) {
                tempRecord.push_back(std::make_shared<Transaction>(tokens[0], tokens[1], tokens[2], tokens[4], amount, tokens[3]));
            }
            else {
                tempRecord.push_back(std::make_shared<Transaction>("IPass(Transportation)",tokens[1], tokens[2], tokens[4], amount, tokens[3]));
            }
        }
    }
    return tempRecord;
}

std::vector<std::shared_ptr<Transaction>> IPass::find_IPass_Record(const std::vector<std::shared_ptr<Transaction>> &transactions) {
    std::cout << "Choose one way to show IPass data" << std::endl;
    std::cout << "1. Show all transaction data (include transportation record)" << std::endl;
    std::cout << "2. Only show transaction data" << std::endl;
    std::cout << "3. Only show transportation data" << std::endl;
    int input; std::cin >> input;
    std::vector<std::shared_ptr<Transaction>> tempRecord;
    switch (input) {
        case 1: {
            for (const auto& transaction : transactions) {
                if (transaction->getType().find("IPass") != std::string::npos ) {
                    tempRecord.push_back(transaction);
                }
            }
            break;
        }
        case 2: {
            for (const auto& transaction : transactions) {
                if (transaction->getType().find("IPass") != std::string::npos && transaction->getType() != "IPass(Transportation)") {
                    tempRecord.push_back(transaction);
                }
            }
            break;
        }
        case 3: {
            for (const auto& transaction : transactions) {
                if (transaction->getType() == "IPass(Transportation)") {
                    tempRecord.push_back(transaction);
                }
            }
            break;
        }
        default:
            std::cerr << "Invalid choice" << std::endl;
    }
    return tempRecord;
}

bool IPass::checkRecord(const std::shared_ptr<Transaction> &record, const std::vector<std::shared_ptr<Transaction> > &transactions) {
    for (const auto& transaction : transactions) {
        if (transaction->getType().find("IPass") != std::string::npos) {
            if (transaction->getTime() == record->getTime() && transaction->getDate() == record->getDate()) {
                record->editType("Deleted");
                return true;
            }
        }
    }
    return false;
}
