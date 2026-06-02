#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

class Transaction {
protected:
    std::string type;
    std::string date;
    std::string time;
    std::string category;
    double amount;
    std::string note;
public:
    Transaction(std::string type,std::string d, std::string time,  std::string c, double a, std::string n);
    void display() const;
    [[nodiscard]] std::string getType() const {return type;}
    [[nodiscard]] double getAmount() const {return amount;}
    [[nodiscard]]std::string getTime() const {return time;}
    [[nodiscard]] std::string getCategory() const {return category;}
    [[nodiscard]] std::string getNote() const {return note;}
    [[nodiscard]] std::string getDate() const {return date;}
    void editType(const std::string& type);
    void editDate(const std::string& date);
    void editTime(const std::string& time);
    void editDateAndTime(const std::string& date, const std::string& time);
    void editCategory(const std::string& category);
    void editNote(const std::string& note);
    void editAmount(double amount);

    static void saveToFile(const std::vector<Transaction> &records, const std::string &filename);

    static int compareDate(const std::shared_ptr<Transaction>& a, const std::shared_ptr<Transaction>& b);

    bool operator ==(const std::shared_ptr<Transaction>&) const;

    virtual std::shared_ptr<Transaction> clone() const;

    virtual ~Transaction() = default;
};

class receipt : public Transaction {
private:
    std::string receiptNumber;
public:
    ~receipt() override = default;

    receipt(std::string type, std::string date, std::string time, std::string category, double amount, std::string note, std::string receiptNumber);
    [[nodiscard]] std::string getReceiptNumber() const {return this->receiptNumber;}

    static void checkUnique(std::vector<receipt>& records, const std::string& filename);

    std::shared_ptr<Transaction> clone() const override;

    static std::vector<std::shared_ptr<Transaction>> getSimpleRecords(const std::vector<std::shared_ptr<Transaction>>& records);

    static void deleteReceipt(const std::string &receiptNumber, const std::vector<std::shared_ptr<Transaction>>& records);

    static void editMulti(const std::vector<std::string>& receiptNumber, const std::string &type, std::vector<std::shared_ptr<Transaction>>& records);

    static void matchingRecords(std::vector<std::shared_ptr<Transaction>>& records);
};

#endif
