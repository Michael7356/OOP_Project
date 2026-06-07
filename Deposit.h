

#ifndef OOP_PROJECT_DEPOSIT_H
#define OOP_PROJECT_DEPOSIT_H
#include <memory>
#include <vector>

#include "Transaction.h"


class Deposit {
protected:
    std::vector<std::shared_ptr<Transaction>> transactions;
public:
    Deposit() = default;


    void display() const;

    virtual std::vector<std::shared_ptr<Transaction>> get_Record() const = 0;

    static std::shared_ptr<Deposit> deposit_ptr();

    static bool checkUnique(const std::shared_ptr<Transaction> &record, const std::vector<std::shared_ptr<Transaction>> &transactions);

    static bool callPython(const std::string filename);

    static std::string getFilePathWithWindow(const std::string &fileType);
    virtual ~Deposit() = default;
};

class CTBC : public Deposit{
private:
    std::string password;
public:
    CTBC(std::string password): password(std::move(password)){}

    void set_password(const std::string& password);
    std::vector<std::shared_ptr<Transaction>> get_Record() const override;
    static std::vector<std::shared_ptr<Transaction>> find_CTBC_Record(const std::vector<std::shared_ptr<Transaction>> &transactions);
};

class POST : public Deposit{
private:
    std::string password;
public:
    POST(std::string password): password(std::move(password)){}

    std::vector<std::shared_ptr<Transaction>> get_Record() const override;
    static std::vector<std::shared_ptr<Transaction>> find_POST_Record(const std::vector<std::shared_ptr<Transaction>> &transactions);

    static bool matchUncategorized(const std::shared_ptr<Transaction> &transaction, const std::vector<std::shared_ptr<Transaction>> &transactions);

    static bool matchUnmatched(const std::shared_ptr<Transaction> &transaction, const std::vector<std::shared_ptr<Transaction>> &transactions);
    static void mergeOriginal(std::vector<std::shared_ptr<Transaction>>& a, std::vector<std::shared_ptr<Transaction>>& b);
};

class IPass : public Deposit{
public:
    static void setBus(const std::vector<std::shared_ptr<Transaction>> &transactions);
    std::vector<std::shared_ptr<Transaction>> get_Record() const override;
    static std::vector<std::shared_ptr<Transaction>> find_IPass_Record(const std::vector<std::shared_ptr<Transaction>> &transactions);
    static bool checkRecord(const std::shared_ptr<Transaction> &transaction, const std::vector<std::shared_ptr<Transaction>> &transactions);
};


#endif //OOP_PROJECT_DEPOSIT_H
