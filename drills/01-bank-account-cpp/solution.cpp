#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

enum class TransactionKind { Deposit, Withdrawal, TransferIn, TransferOut };

struct Transaction {
    TransactionKind kind;
    std::int64_t amount_cents;
};

class BankAccount {
public:
    explicit BankAccount(std::string owner, std::int64_t opening_balance_cents = 0) {
        owner_ = owner;
        balance_ = opening_balance_cents;
    }

    const std::string& owner() const {
        return owner_;
    }

    std::int64_t balance_cents() const {
        return balance_;
    }

    const std::vector<Transaction>& history() const {
        return transactions_;
    }

    void deposit(std::int64_t amount_cents) {
        if (amount_cents <= 0) throw invalid_argument("amount 0 or neg");
        balance_ += amount_cents;
        transactions_.push_back(Transaction(TransactionKind::Deposit, amount_cents));
    }

    void withdraw(std::int64_t amount_cents) {
        if (amount_cents <= 0) throw invalid_argument("amount 0 or neg");
        if (amount_cents > balance_) throw invalid_argument("amount > balance");
        balance_ -= amount_cents;
        transactions_.push_back(Transaction(TransactionKind::Withdrawal, amount_cents));
    }

    void transfer_to(BankAccount& other, std::int64_t amount_cents) {
        if (amount_cents <= 0) throw invalid_argument("amount 0 or neg");
        if (amount_cents > balance_) if (amount_cents > balance_) throw invalid_argument("amount > balance");
        balance_ -= amount_cents;
        other.balance_ += amount_cents;
        transactions_.push_back(Transaction(TransactionKind::TransferOut, amount_cents));
        other.transactions_.push_back(Transaction(TransactionKind::TransferIn, amount_cents));
    }

private:
    string owner_;
    int64_t balance_;
    vector<Transaction> transactions_;
};

#ifndef TESTING
int main() {
    // Scratch space: `make run` builds and runs this.
    return 0;
}
#endif
