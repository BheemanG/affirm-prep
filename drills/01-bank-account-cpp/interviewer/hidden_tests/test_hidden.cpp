// Hidden tests. Built by `make hidden`, which defines SOLUTION_FILE.
#define TESTING
#include SOLUTION_FILE

#include "check.h"

#include <type_traits>

static bool same(const Transaction& t, TransactionKind kind, std::int64_t amount_cents) {
    return t.kind == kind && t.amount_cents == amount_cents;
}

// ---- compile-time shape -------------------------------------------------

static_assert(!std::is_convertible_v<std::string, BankAccount>,
              "single-argument constructor should be explicit");
static_assert(std::is_same_v<decltype(std::declval<const BankAccount&>().balance_cents()), std::int64_t>,
              "balance_cents() must be const and return int64_t");
static_assert(std::is_same_v<decltype(std::declval<const BankAccount&>().history()),
                             const std::vector<Transaction>&>,
              "history() must be const and return a const reference");

// ---- construction -------------------------------------------------------

TEST(negative_opening_balance_throws) {
    CHECK_THROWS(BankAccount("Ada", -1), std::invalid_argument);
}

TEST(zero_opening_balance_is_allowed) {
    const BankAccount acct("Ada", 0);
    CHECK(acct.balance_cents() == 0);
}

TEST(opening_balance_is_not_a_history_entry) {
    const BankAccount acct("Ada", 1000);
    CHECK(acct.history().empty());
}

TEST(large_balances_fit_in_int64) {
    BankAccount acct("Ada", 5'000'000'000'000LL);
    acct.deposit(3'000'000'000LL);
    CHECK(acct.balance_cents() == 5'003'000'000'000LL);
}

// ---- deposit ------------------------------------------------------------

TEST(deposit_zero_throws_and_changes_nothing) {
    BankAccount acct("Ada", 1000);
    CHECK_THROWS(acct.deposit(0), std::invalid_argument);
    CHECK(acct.balance_cents() == 1000);
    CHECK(acct.history().empty());
}

TEST(deposit_negative_throws_and_changes_nothing) {
    BankAccount acct("Ada", 1000);
    CHECK_THROWS(acct.deposit(-500), std::invalid_argument);
    CHECK(acct.balance_cents() == 1000);
    CHECK(acct.history().empty());
}

// ---- withdraw -----------------------------------------------------------

TEST(withdraw_zero_throws_and_changes_nothing) {
    BankAccount acct("Ada", 1000);
    CHECK_THROWS(acct.withdraw(0), std::invalid_argument);
    CHECK(acct.balance_cents() == 1000);
    CHECK(acct.history().empty());
}

TEST(withdraw_negative_throws_and_changes_nothing) {
    BankAccount acct("Ada", 1000);
    CHECK_THROWS(acct.withdraw(-1), std::invalid_argument);
    CHECK(acct.balance_cents() == 1000);
    CHECK(acct.history().empty());
}

TEST(overdraft_throws_and_changes_nothing) {
    BankAccount acct("Ada", 1000);
    acct.deposit(150);
    CHECK_THROWS(acct.withdraw(5000), std::invalid_argument);
    CHECK(acct.balance_cents() == 1150);
    CHECK(acct.history().size() == 1);
}

TEST(overdraft_by_one_cent_throws) {
    BankAccount acct("Ada", 1000);
    CHECK_THROWS(acct.withdraw(1001), std::invalid_argument);
    CHECK(acct.balance_cents() == 1000);
}

TEST(withdraw_exact_balance_leaves_zero) {
    BankAccount acct("Ada", 1000);
    acct.withdraw(1000);
    CHECK(acct.balance_cents() == 0);
    CHECK(acct.history().size() == 1 && same(acct.history()[0], TransactionKind::Withdrawal, 1000));
}

TEST(withdraw_from_empty_account_throws) {
    BankAccount acct("Ada");
    CHECK_THROWS(acct.withdraw(1), std::invalid_argument);
    CHECK(acct.balance_cents() == 0);
}

// ---- transfer -----------------------------------------------------------

TEST(transfer_overdraft_changes_neither_account) {
    BankAccount ada("Ada", 100);
    BankAccount bob("Bob", 50);
    CHECK_THROWS(ada.transfer_to(bob, 101), std::invalid_argument);
    CHECK(ada.balance_cents() == 100);
    CHECK(bob.balance_cents() == 50);
    CHECK(ada.history().empty());
    CHECK(bob.history().empty());
}

TEST(transfer_non_positive_changes_neither_account) {
    BankAccount ada("Ada", 100);
    BankAccount bob("Bob", 50);
    CHECK_THROWS(ada.transfer_to(bob, 0), std::invalid_argument);
    CHECK_THROWS(ada.transfer_to(bob, -20), std::invalid_argument);
    CHECK(ada.balance_cents() == 100);
    CHECK(bob.balance_cents() == 50);
    CHECK(ada.history().empty());
    CHECK(bob.history().empty());
}

TEST(transfer_to_self_throws_and_changes_nothing) {
    BankAccount ada("Ada", 100);
    CHECK_THROWS(ada.transfer_to(ada, 10), std::invalid_argument);
    CHECK(ada.balance_cents() == 100);
    CHECK(ada.history().empty());
}

TEST(transfer_entire_balance) {
    BankAccount ada("Ada", 100);
    BankAccount bob("Bob");
    ada.transfer_to(bob, 100);
    CHECK(ada.balance_cents() == 0);
    CHECK(bob.balance_cents() == 100);
}

TEST(transfer_does_not_touch_third_account) {
    BankAccount ada("Ada", 100);
    BankAccount bob("Bob");
    BankAccount cy("Cy", 7);
    ada.transfer_to(bob, 40);
    CHECK(cy.balance_cents() == 7);
    CHECK(cy.history().empty());
}

// ---- history ------------------------------------------------------------

TEST(history_keeps_mixed_operations_in_order) {
    BankAccount ada("Ada", 500);
    BankAccount bob("Bob", 500);
    ada.deposit(10);
    ada.transfer_to(bob, 20);
    bob.transfer_to(ada, 30);
    ada.withdraw(40);
    const std::vector<Transaction>& h = ada.history();
    CHECK(h.size() == 4);
    if (h.size() == 4) {
        CHECK(same(h[0], TransactionKind::Deposit, 10));
        CHECK(same(h[1], TransactionKind::TransferOut, 20));
        CHECK(same(h[2], TransactionKind::TransferIn, 30));
        CHECK(same(h[3], TransactionKind::Withdrawal, 40));
    }
    CHECK(ada.balance_cents() == 480);
    CHECK(bob.balance_cents() == 490);
}

TEST(failed_operations_leave_no_history_gaps) {
    BankAccount acct("Ada", 100);
    acct.deposit(5);
    CHECK_THROWS(acct.withdraw(1000), std::invalid_argument);
    CHECK_THROWS(acct.deposit(0), std::invalid_argument);
    acct.withdraw(5);
    CHECK(acct.history().size() == 2);
    CHECK(acct.balance_cents() == 100);
}

TEST(accounts_do_not_share_state) {
    BankAccount ada("Ada", 100);
    BankAccount bob("Bob", 200);
    ada.deposit(1);
    CHECK(bob.balance_cents() == 200);
    CHECK(bob.history().empty());
}

TEST(copied_account_is_independent) {
    BankAccount ada("Ada", 100);
    BankAccount copy = ada;
    copy.deposit(50);
    CHECK(ada.balance_cents() == 100);
    CHECK(ada.history().empty());
}

int main() { return check::run_all(); }
