#include "Deposit.h"
#include "httplib.h"
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
    std::string filter;
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
    ofn.lpstrFilter = filter.c_str();
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = nullptr;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = ".\\Storage"; //Default folder when open
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
        if (transaction->getType() == "POST[Uncategorized]") {
            int diff = Transaction::compareDate(transaction, record);
            if (diff >= 0 && diff <= 7) {
                if (transaction->getAmount() == record->getAmount()) {
                    record->editType("POST[Uncategorized]");
                    return true;

                }
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
                    record->editDate(Tdate);
                    record->editCategory(transaction->getCategory());
                    return true;
                }
            }
            else {
                if (Tdate == record->getDate() && transaction->getAmount() == record->getAmount()) {
                    record->editType("POST[Unmatched]");
                    record->editCategory(transaction->getCategory());
                    return true;

                }
            }
        }
        else if (record->getType() == "POST[Unconfirm]") {
            if (Tdate == record->getDate() && transaction->getAmount() == record->getAmount()) {
                record->editType("POST[Unmatched]");
                record->editCategory(transaction->getCategory());
                return true;
            }
        }
    }
    return false;
}

void POST::mergeOriginal(std::vector<std::shared_ptr<Transaction>>& addInTransactions, std::vector<std::shared_ptr<Transaction>>& originalTransaction) {
    std::vector<std::shared_ptr<Transaction>> tempTransactions;

    for (const auto& transaction : originalTransaction) {
        std::string type = transaction->getType();
        if (type == "POST[Unconfirmed]" || type == "POST[Uncategorized") {
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
    return tempRecord;
}

std::vector<std::shared_ptr<Transaction>> IPass::find_IPass_Record(const std::vector<std::shared_ptr<Transaction>> &transactions) {
    std::vector<std::shared_ptr<Transaction>> tempRecord;
    for (const auto& transaction : transactions) {
        if (transaction->getType() == "IPass") {
            tempRecord.push_back(transaction);
        }
    }
    return tempRecord;
}