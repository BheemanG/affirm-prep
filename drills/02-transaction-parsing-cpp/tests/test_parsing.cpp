#define TESTING
#include "../solution.cpp"

#include "check.h"

#include <sstream>

TEST(amount_with_two_decimals) {
    CHECK(parse_amount_cents("12.34") == 1234);
}

TEST(amount_without_decimals) {
    CHECK(parse_amount_cents("7") == 700);
}

TEST(amount_with_one_decimal) {
    CHECK(parse_amount_cents("7.5") == 750);
}

TEST(amount_not_a_number_throws) {
    CHECK_THROWS(parse_amount_cents("abc"), std::invalid_argument);
}

TEST(format_dollars_and_cents) {
    CHECK(format_cents(1234) == "$12.34");
    CHECK(format_cents(0) == "$0.00");
}

TEST(format_pads_single_digit_cents) {
    CHECK(format_cents(5) == "$0.05");
}

TEST(format_negative) {
    CHECK(format_cents(-5) == "-$0.05");
}

TEST(parse_valid_row) {
    const Transaction txn = parse_transaction("t1,acct-1,CREDIT,100.00");
    CHECK(txn.id == "t1");
    CHECK(txn.account == "acct-1");
    CHECK(txn.type == TransactionType::Credit);
    CHECK(txn.amount_cents == 10000);
}

TEST(parse_debit_row) {
    const Transaction txn = parse_transaction("t2,acct-1,DEBIT,25.50");
    CHECK(txn.type == TransactionType::Debit);
    CHECK(txn.amount_cents == 2550);
}

TEST(parse_row_with_too_few_fields_throws) {
    CHECK_THROWS(parse_transaction("t1,acct-1,CREDIT"), std::invalid_argument);
}

TEST(load_good_rows_in_file_order) {
    std::istringstream in("id,account,type,amount\nt1,a,CREDIT,1.00\nt2,b,DEBIT,2.00\n");
    std::vector<std::string> errors;
    const std::vector<Transaction> txns = load_transactions(in, errors);
    CHECK(errors.empty());
    CHECK(txns.size() == 2);
    if (txns.size() == 2) {
        CHECK(txns[0].id == "t1");
        CHECK(txns[1].id == "t2");
    }
}

TEST(load_reports_bad_row_and_continues) {
    std::istringstream in("id,account,type,amount\nt1,a,CREDIT,1.00\nt2,a,DEBIT,abc\nt3,a,CREDIT,3.00\n");
    std::vector<std::string> errors;
    const std::vector<Transaction> txns = load_transactions(in, errors);
    CHECK(txns.size() == 2);
    CHECK(errors.size() == 1);
    if (errors.size() == 1) CHECK(errors[0].rfind("line 3", 0) == 0);
}

TEST(balances_net_credits_and_debits) {
    const std::vector<Transaction> txns = {
        {"t1", "b", TransactionType::Credit, 1000},
        {"t2", "a", TransactionType::Credit, 500},
        {"t3", "b", TransactionType::Debit, 250},
    };
    const auto balances = balances_by_account(txns);
    CHECK(balances.size() == 2);
    CHECK(balances.at("a") == 500);
    CHECK(balances.at("b") == 750);
}

int main() { return check::run_all(); }
