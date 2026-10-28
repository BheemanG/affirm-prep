#include <cstdint>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

enum class TransactionType { Credit, Debit };

struct Transaction {
    std::string id;
    std::string account;
    TransactionType type;
    std::int64_t amount_cents;
};

// "12.34" -> 1234. Throws std::invalid_argument if the text is not a valid amount.
std::int64_t parse_amount_cents(const std::string& text) {
    throw std::logic_error("parse_amount_cents: not implemented");
}

// 1234 -> "$12.34". Negative values keep the sign in front: -5 -> "-$0.05".
std::string format_cents(std::int64_t cents) {
    throw std::logic_error("format_cents: not implemented");
}

// One CSV row "id,account,type,amount" -> Transaction.
// Throws std::invalid_argument if the row is malformed.
Transaction parse_transaction(const std::string& line) {
    throw std::logic_error("parse_transaction: not implemented");
}

// Reads every row from `in`. Good rows are returned; for each bad row, a
// message is appended to `errors` and parsing continues with the next row.
std::vector<Transaction> load_transactions(std::istream& in, std::vector<std::string>& errors) {
    throw std::logic_error("load_transactions: not implemented");
}

// Net balance per account, in cents. Credits add, debits subtract.
std::map<std::string, std::int64_t> balances_by_account(const std::vector<Transaction>& transactions) {
    throw std::logic_error("balances_by_account: not implemented");
}

#ifndef TESTING
int main() {
    std::vector<std::string> errors;
    const std::vector<Transaction> transactions = load_transactions(std::cin, errors);
    for (const std::string& error : errors) {
        std::cout << "error: " << error << "\n";
    }
    for (const auto& [account, balance_cents] : balances_by_account(transactions)) {
        std::cout << account << " " << format_cents(balance_cents) << "\n";
    }
    return 0;
}
#endif
