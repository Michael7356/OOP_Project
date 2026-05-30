

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

};

class PS : public Deposit{
private:
    std::string password;
public:
    PS(std::string password): password(std::move(password)){}

    std::vector<std::shared_ptr<Transaction>> get_Record() const override;
};

class IPass : public Deposit{
public:
    std::vector<std::shared_ptr<Transaction>> get_Record() const override;
};


#endif //OOP_PROJECT_DEPOSIT_H
