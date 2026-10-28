#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

enum class TransactionKind { Deposit, Withdrawal, TransferIn, TransferOut };

struct Transaction {
    TransactionKind kind;
    std::int64_t amount_cents;
};

class BankAccount {
public:
    explicit BankAccount(std::string owner, std::int64_t opening_balance_cents = 0) {
        throw std::logic_error("BankAccount: not implemented");
    }

    const std::string& owner() const {
        throw std::logic_error("owner: not implemented");
    }

    std::int64_t balance_cents() const {
        throw std::logic_error("balance_cents: not implemented");
    }

    const std::vector<Transaction>& history() const {
        throw std::logic_error("history: not implemented");
    }

    void deposit(std::int64_t amount_cents) {
        throw std::logic_error("deposit: not implemented");
    }

    void withdraw(std::int64_t amount_cents) {
        throw std::logic_error("withdraw: not implemented");
    }

    void transfer_to(BankAccount& other, std::int64_t amount_cents) {
        throw std::logic_error("transfer_to: not implemented");
    }

private:
    // Your data members (and any helpers) go here.
};

#ifndef TESTING
int main() {
    // Scratch space: `make run` builds and runs this.
    return 0;
}
#endif
