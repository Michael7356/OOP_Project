#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <string>
#include <vector>
#include <fstream>

class Transaction {
private:
    std::string date;
    std::string time;
    std::string category;
    double amount;
    std::string note;
public:
    Transaction(std::string d, std::string time,  std::string c, double a, std::string n);
    void display() const;
    [[nodiscard]] double getAmount() const {return amount;}
    [[nodiscard]]std::string getTime() const {return time;}
    [[nodiscard]] std::string getCategory() const {return category;}
    [[nodiscard]] std::string getNote() const {return note;}
    [[nodiscard]] std::string getDate() const {return date;}

    static void saveToFile(const std::vector<Transaction> &records, const std::string &filename);

    static std::vector<Transaction> loadFromFile(const std::string &filename);

};

#endif
