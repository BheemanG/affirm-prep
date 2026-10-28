// Hidden tests. Built by `make hidden`, which defines SOLUTION_FILE.
#define TESTING
#include SOLUTION_FILE

#include "check.h"

#include <climits>
#include <sstream>

static std::vector<Transaction> load(const std::string& text, std::vector<std::string>& errors) {
    std::istringstream in(text);
    return load_transactions(in, errors);
}

// ---- parse_amount_cents --------------------------------------------------

TEST(amount_basic_forms) {
    CHECK(parse_amount_cents("0") == 0);
    CHECK(parse_amount_cents("0.00") == 0);
    CHECK(parse_amount_cents("0.05") == 5);
    CHECK(parse_amount_cents("0.5") == 50);
    CHECK(parse_amount_cents("7.50") == 750);
    CHECK(parse_amount_cents("100") == 10000);
    CHECK(parse_amount_cents("007.10") == 710);
}

TEST(amount_surrounding_whitespace_is_ignored) {
    CHECK(parse_amount_cents("  12.34 ") == 1234);
    CHECK(parse_amount_cents("12.34\r") == 1234);
}

TEST(amount_rejects_garbage) {
    CHECK_THROWS(parse_amount_cents(""), std::invalid_argument);
    CHECK_THROWS(parse_amount_cents("   "), std::invalid_argument);
    CHECK_THROWS(parse_amount_cents("12abc"), std::invalid_argument);
    CHECK_THROWS(parse_amount_cents("abc12"), std::invalid_argument);
    CHECK_THROWS(parse_amount_cents("1 2"), std::invalid_argument);
    CHECK_THROWS(parse_amount_cents("1,000.00"), std::invalid_argument);
    CHECK_THROWS(parse_amount_cents("$5.00"), std::invalid_argument);
}

TEST(amount_rejects_bad_decimal_forms) {
    CHECK_THROWS(parse_amount_cents(".5"), std::invalid_argument);
    CHECK_THROWS(parse_amount_cents("12."), std::invalid_argument);
    CHECK_THROWS(parse_amount_cents("."), std::invalid_argument);
    CHECK_THROWS(parse_amount_cents("1.2.3"), std::invalid_argument);
    CHECK_THROWS(parse_amount_cents("1.234"), std::invalid_argument);
    CHECK_THROWS(parse_amount_cents("1.005"), std::invalid_argument);
}

TEST(amount_rejects_signs) {
    CHECK_THROWS(parse_amount_cents("-5.00"), std::invalid_argument);
    CHECK_THROWS(parse_amount_cents("+5.00"), std::invalid_argument);
    CHECK_THROWS(parse_amount_cents("1.-5"), std::invalid_argument);
}

TEST(amount_rejects_overflow_with_invalid_argument) {
    CHECK_THROWS(parse_amount_cents("99999999999999999999"), std::invalid_argument);
    // 92233720368547758.07 dollars is the largest representable amount.
    CHECK(parse_amount_cents("92233720368547758.07") == INT64_MAX);
    CHECK_THROWS(parse_amount_cents("92233720368547758.08"), std::invalid_argument);
    CHECK_THROWS(parse_amount_cents("92233720368547759"), std::invalid_argument);
}

TEST(amount_is_exact_not_floating_point) {
    CHECK(parse_amount_cents("0.10") == 10);
    CHECK(parse_amount_cents("0.29") == 29);
    CHECK(parse_amount_cents("1234567.89") == 123456789);
    CHECK(parse_amount_cents("19.99") == 1999);
}

// ---- format_cents --------------------------------------------------------

TEST(format_boundaries) {
    CHECK(format_cents(99) == "$0.99");
    CHECK(format_cents(100) == "$1.00");
    CHECK(format_cents(101) == "$1.01");
    CHECK(format_cents(1000) == "$10.00");
    CHECK(format_cents(123456789) == "$1234567.89");
}

TEST(format_negatives) {
    CHECK(format_cents(-1) == "-$0.01");
    CHECK(format_cents(-100) == "-$1.00");
    CHECK(format_cents(-12345) == "-$123.45");
}

TEST(format_extreme_values) {
    CHECK(format_cents(INT64_MAX) == "$92233720368547758.07");
    CHECK(format_cents(INT64_MIN + 1) == "-$92233720368547758.07");
}

TEST(format_then_parse_round_trips) {
    for (std::int64_t cents : {0, 1, 9, 10, 99, 100, 101, 12345, 99999999}) {
        const std::string text = format_cents(cents);
        CHECK(parse_amount_cents(text.substr(1)) == cents);
    }
}

// ---- parse_transaction ---------------------------------------------------

TEST(row_trims_whitespace_and_crlf) {
    const Transaction txn = parse_transaction("  t1 , acct-1 , CREDIT , 5.00 \r");
    CHECK(txn.id == "t1");
    CHECK(txn.account == "acct-1");
    CHECK(txn.type == TransactionType::Credit);
    CHECK(txn.amount_cents == 500);
}

TEST(row_wrong_field_count_throws) {
    CHECK_THROWS(parse_transaction(""), std::invalid_argument);
    CHECK_THROWS(parse_transaction("t1"), std::invalid_argument);
    CHECK_THROWS(parse_transaction("t1,a,CREDIT"), std::invalid_argument);
    CHECK_THROWS(parse_transaction("t1,a,CREDIT,5.00,extra"), std::invalid_argument);
}

TEST(row_trailing_comma_is_an_extra_empty_field) {
    // getline(ss, field, ',') silently drops the empty last field.
    CHECK_THROWS(parse_transaction("t1,a,CREDIT,5.00,"), std::invalid_argument);
}

TEST(row_trailing_comma_with_missing_amount_throws) {
    CHECK_THROWS(parse_transaction("t1,a,CREDIT,"), std::invalid_argument);
}

TEST(row_empty_id_or_account_throws) {
    CHECK_THROWS(parse_transaction(",a,CREDIT,5.00"), std::invalid_argument);
    CHECK_THROWS(parse_transaction("t1,,CREDIT,5.00"), std::invalid_argument);
    CHECK_THROWS(parse_transaction("  ,a,CREDIT,5.00"), std::invalid_argument);
}

TEST(row_type_must_be_exact) {
    CHECK_THROWS(parse_transaction("t1,a,credit,5.00"), std::invalid_argument);
    CHECK_THROWS(parse_transaction("t1,a,REFUND,5.00"), std::invalid_argument);
    CHECK_THROWS(parse_transaction("t1,a,,5.00"), std::invalid_argument);
}

TEST(row_zero_amount_throws) {
    CHECK_THROWS(parse_transaction("t1,a,CREDIT,0"), std::invalid_argument);
    CHECK_THROWS(parse_transaction("t1,a,DEBIT,0.00"), std::invalid_argument);
}

TEST(row_bad_amount_throws) {
    CHECK_THROWS(parse_transaction("t1,a,CREDIT,-5.00"), std::invalid_argument);
    CHECK_THROWS(parse_transaction("t1,a,CREDIT,5.001"), std::invalid_argument);
    CHECK_THROWS(parse_transaction("t1,a,CREDIT,five"), std::invalid_argument);
}

// ---- load_transactions ---------------------------------------------------

TEST(load_header_is_optional) {
    std::vector<std::string> errors;
    const auto with_header = load("id,account,type,amount\nt1,a,CREDIT,1.00\n", errors);
    const auto without_header = load("t1,a,CREDIT,1.00\n", errors);
    CHECK(errors.empty());
    CHECK(with_header.size() == 1);
    CHECK(without_header.size() == 1);
}

TEST(load_header_only_in_first_row) {
    std::vector<std::string> errors;
    const auto txns = load("id,account,type,amount\nt1,a,CREDIT,1.00\nid,account,type,amount\n", errors);
    CHECK(txns.size() == 1);
    CHECK(errors.size() == 1);
    if (errors.size() == 1) CHECK(errors[0].rfind("line 3", 0) == 0);
}

TEST(load_empty_input) {
    std::vector<std::string> errors;
    CHECK(load("", errors).empty());
    CHECK(load("\n\n  \n", errors).empty());
    CHECK(errors.empty());
}

TEST(load_blank_lines_are_skipped_but_counted) {
    std::vector<std::string> errors;
    const auto txns = load("t1,a,CREDIT,1.00\n\n\nt2,a,DEBIT,bad\n", errors);
    CHECK(txns.size() == 1);
    CHECK(errors.size() == 1);
    if (errors.size() == 1) CHECK(errors[0].rfind("line 4", 0) == 0);
}

TEST(load_header_after_blank_lines_still_counts_as_header) {
    std::vector<std::string> errors;
    const auto txns = load("\n\nid,account,type,amount\nt1,a,CREDIT,1.00\n", errors);
    CHECK(errors.empty());
    CHECK(txns.size() == 1);
}

TEST(load_windows_line_endings) {
    std::vector<std::string> errors;
    const auto txns = load("id,account,type,amount\r\nt1,a,CREDIT,1.00\r\nt2,a,DEBIT,2.50\r\n", errors);
    CHECK(errors.empty());
    CHECK(txns.size() == 2);
    if (txns.size() == 2) {
        CHECK(txns[1].type == TransactionType::Debit);
        CHECK(txns[1].amount_cents == 250);
    }
}

TEST(load_no_trailing_newline) {
    std::vector<std::string> errors;
    const auto txns = load("t1,a,CREDIT,1.00\nt2,a,CREDIT,2.00", errors);
    CHECK(errors.empty());
    CHECK(txns.size() == 2);
}

TEST(load_duplicate_id_keeps_first_and_reports_second) {
    std::vector<std::string> errors;
    const auto txns = load("t1,a,CREDIT,1.00\nt1,a,CREDIT,9.00\nt2,a,CREDIT,2.00\n", errors);
    CHECK(txns.size() == 2);
    if (txns.size() == 2) {
        CHECK(txns[0].amount_cents == 100);
        CHECK(txns[1].id == "t2");
    }
    CHECK(errors.size() == 1);
    if (errors.size() == 1) CHECK(errors[0].rfind("line 2", 0) == 0);
}

TEST(load_bad_row_does_not_burn_its_id) {
    std::vector<std::string> errors;
    const auto txns = load("t1,a,CREDIT,oops\nt1,a,CREDIT,1.00\n", errors);
    CHECK(txns.size() == 1);
    CHECK(errors.size() == 1);
    if (errors.size() == 1) CHECK(errors[0].rfind("line 1", 0) == 0);
}

TEST(load_reports_every_bad_row_in_order) {
    std::vector<std::string> errors;
    const auto txns = load("t1,a,CREDIT,1.00\nbad\nt2,a,DEBIT,-1\nt3,a,CREDIT,3.00\n,a,CREDIT,1\n", errors);
    CHECK(txns.size() == 2);
    CHECK(errors.size() == 3);
    if (errors.size() == 3) {
        CHECK(errors[0].rfind("line 2", 0) == 0);
        CHECK(errors[1].rfind("line 3", 0) == 0);
        CHECK(errors[2].rfind("line 5", 0) == 0);
    }
}

TEST(load_appends_to_existing_errors) {
    std::vector<std::string> errors = {"earlier"};
    load("bad\n", errors);
    CHECK(errors.size() == 2);
    if (errors.size() == 2) CHECK(errors[0] == "earlier");
}

// ---- balances_by_account -------------------------------------------------

TEST(balances_empty_input_gives_empty_map) {
    CHECK(balances_by_account({}).empty());
}

TEST(balances_can_go_negative) {
    const std::vector<Transaction> txns = {
        {"t1", "a", TransactionType::Debit, 300},
        {"t2", "a", TransactionType::Credit, 100},
    };
    const auto balances = balances_by_account(txns);
    CHECK(balances.size() == 1);
    CHECK(balances.at("a") == -200);
}

TEST(balances_are_ordered_by_account_name) {
    const std::vector<Transaction> txns = {
        {"t1", "zeta", TransactionType::Credit, 1},
        {"t2", "alpha", TransactionType::Credit, 1},
        {"t3", "mid", TransactionType::Credit, 1},
    };
    std::vector<std::string> names;
    for (const auto& entry : balances_by_account(txns)) names.push_back(entry.first);
    CHECK((names == std::vector<std::string>{"alpha", "mid", "zeta"}));
}

TEST(balances_account_that_nets_to_zero_is_still_listed) {
    const std::vector<Transaction> txns = {
        {"t1", "a", TransactionType::Credit, 500},
        {"t2", "a", TransactionType::Debit, 500},
    };
    const auto balances = balances_by_account(txns);
    CHECK(balances.size() == 1);
    CHECK(balances.at("a") == 0);
}

TEST(balances_end_to_end_with_dirty_file) {
    std::vector<std::string> errors;
    const auto txns = load(
        "id,account,type,amount\r\n"
        "t1,acct-1,CREDIT,100.00\r\n"
        "t2,acct-1,DEBIT,25.50\r\n"
        "t3,acct-2,CREDIT,10\r\n"
        "t4,acct-2,DEBIT,abc\r\n"
        "t5,acct-1,DEBIT,0.05\r\n",
        errors);
    const auto balances = balances_by_account(txns);
    CHECK(errors.size() == 1);
    CHECK(balances.at("acct-1") == 7445);
    CHECK(balances.at("acct-2") == 1000);
}

int main() { return check::run_all(); }
