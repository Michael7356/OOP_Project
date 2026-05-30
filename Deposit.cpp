#include "Deposit.h"
#include "httplib.h"
#include "PdfParser.h"

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
            return std::make_shared<PS>(password);
        }
        default :
            std::cerr << "Invalid input" << std::endl;
            break;
    }
    return nullptr;
}

void CTBC::set_password(const std::string& password) {
    this->password = password;
}

std::vector<std::shared_ptr<Transaction>> CTBC::get_Record() const {
    std::vector<std::shared_ptr<Transaction>> tempRecord;
    std::string filePath = getFilePathWithWindow("PDF");
    if (!filePath.empty()) {
        std::vector<Transaction> temp;
        try {
           temp = PdfParser::parseBankStatement(filePath, this->password);
        }catch(std::runtime_error& e) {
            std::cerr << "Failed, check if your .pdf file is correct one [Error code:" << e.what() << "]" << std::endl;
        }
        for (const auto& record : temp) {
            tempRecord.push_back(std::make_shared<Transaction>(record));
        }
    }
    else {
        std::cerr << "Canceled the load" << std::endl;
    }
    return tempRecord;
}

std::vector<std::shared_ptr<Transaction>> PS::get_Record() const {
    std::vector<std::shared_ptr<Transaction>> tempRecord;
    std::string filePath = getFilePathWithWindow("CSV");
    if (!filePath.empty()) {
        try {
            tempRecord = csvParser::loadFromFile_PS(filePath);
        }catch (const std::runtime_error& e) {
            std::cerr << "Failed, check if your .csv file is correct one [Error code:" << e.what() << "]" << std::endl;
        }
    }
    return tempRecord;
}

std::vector<std::shared_ptr<Transaction>> IPass::get_Record() const {
    std::vector<std::shared_ptr<Transaction>> tempRecord;
    return tempRecord;
}
