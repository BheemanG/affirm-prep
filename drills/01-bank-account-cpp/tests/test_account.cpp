#define TESTING
#include "../solution.cpp"

#include "check.h"

static bool same(const Transaction& t, TransactionKind kind, std::int64_t amount_cents) {
    return t.kind == kind && t.amount_cents == amount_cents;
}

TEST(new_account_has_owner_and_opening_balance) {
    const BankAccount acct("Ada", 1000);
    CHECK(acct.owner() == "Ada");
    CHECK(acct.balance_cents() == 1000);
}

TEST(opening_balance_defaults_to_zero) {
    const BankAccount acct("Ada");
    CHECK(acct.balance_cents() == 0);
}

TEST(deposit_increases_balance) {
    BankAccount acct("Ada", 1000);
    acct.deposit(250);
    CHECK(acct.balance_cents() == 1250);
}

TEST(withdraw_decreases_balance) {
    BankAccount acct("Ada", 1000);
    acct.withdraw(300);
    CHECK(acct.balance_cents() == 700);
}

TEST(history_records_operations_oldest_first) {
    BankAccount acct("Ada", 1000);
    acct.deposit(250);
    acct.withdraw(100);
    const std::vector<Transaction>& history = acct.history();
    CHECK(history.size() == 2);
    if (history.size() == 2) {
        CHECK(same(history[0], TransactionKind::Deposit, 250));
        CHECK(same(history[1], TransactionKind::Withdrawal, 100));
    }
}

TEST(transfer_moves_money_and_records_both_sides) {
    BankAccount ada("Ada", 1000);
    BankAccount bob("Bob");
    ada.transfer_to(bob, 150);
    CHECK(ada.balance_cents() == 850);
    CHECK(bob.balance_cents() == 150);
    CHECK(ada.history().size() == 1 && same(ada.history().back(), TransactionKind::TransferOut, 150));
    CHECK(bob.history().size() == 1 && same(bob.history().back(), TransactionKind::TransferIn, 150));
}

int main() { return check::run_all(); }
