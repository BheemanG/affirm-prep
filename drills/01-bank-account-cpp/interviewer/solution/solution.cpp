#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

enum class TransactionKind { Deposit, Withdrawal, TransferIn, TransferOut };

struct Transaction {
    TransactionKind kind;
    std::int64_t amount_cents;
};

class BankAccount {
public:
    explicit BankAccount(std::string owner, std::int64_t opening_balance_cents = 0)
        : owner_(std::move(owner)), balance_cents_(opening_balance_cents) {
        if (opening_balance_cents < 0) {
            throw std::invalid_argument("opening balance cannot be negative");
        }
    }

    const std::string& owner() const { return owner_; }
    std::int64_t balance_cents() const { return balance_cents_; }
    const std::vector<Transaction>& history() const { return history_; }

    void deposit(std::int64_t amount_cents) {
        require_positive(amount_cents);
        require_room_for(amount_cents);
        history_.push_back({TransactionKind::Deposit, amount_cents});
        balance_cents_ += amount_cents;
    }

    void withdraw(std::int64_t amount_cents) {
        require_positive(amount_cents);
        require_funds(amount_cents);
        history_.push_back({TransactionKind::Withdrawal, amount_cents});
        balance_cents_ -= amount_cents;
    }

    void transfer_to(BankAccount& other, std::int64_t amount_cents) {
        if (&other == this) {
            throw std::invalid_argument("cannot transfer to the same account");
        }
        require_positive(amount_cents);
        require_funds(amount_cents);
        other.require_room_for(amount_cents);

        // Reserve first so nothing below can throw once we start mutating.
        history_.reserve(history_.size() + 1);
        other.history_.reserve(other.history_.size() + 1);

        history_.push_back({TransactionKind::TransferOut, amount_cents});
        other.history_.push_back({TransactionKind::TransferIn, amount_cents});
        balance_cents_ -= amount_cents;
        other.balance_cents_ += amount_cents;
    }

private:
    static void require_positive(std::int64_t amount_cents) {
        if (amount_cents <= 0) {
            throw std::invalid_argument("amount must be positive");
        }
    }

    void require_funds(std::int64_t amount_cents) const {
        if (amount_cents > balance_cents_) {
            throw std::invalid_argument("insufficient funds");
        }
    }

    void require_room_for(std::int64_t amount_cents) const {
        if (amount_cents > std::numeric_limits<std::int64_t>::max() - balance_cents_) {
            throw std::invalid_argument("deposit would overflow the balance");
        }
    }

    std::string owner_;
    std::int64_t balance_cents_;
    std::vector<Transaction> history_;
};

#ifndef TESTING
int main() {
    BankAccount ada("Ada", 1000);
    ada.deposit(250);
    ada.withdraw(100);
    std::cout << ada.owner() << ": " << ada.balance_cents() << " cents\n";
    return 0;
}
#endif
