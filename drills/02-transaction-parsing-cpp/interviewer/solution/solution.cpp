#include <cstdint>
#include <iostream>
#include <limits>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

enum class TransactionType { Credit, Debit };

struct Transaction {
    std::string id;
    std::string account;
    TransactionType type;
    std::int64_t amount_cents;
};

static std::string trim(const std::string& text) {
    const char* whitespace = " \t\r\n";
    const std::size_t first = text.find_first_not_of(whitespace);
    if (first == std::string::npos) return "";
    const std::size_t last = text.find_last_not_of(whitespace);
    return text.substr(first, last - first + 1);
}

static bool all_digits(const std::string& text) {
    if (text.empty()) return false;
    for (char c : text) {
        if (c < '0' || c > '9') return false;
    }
    return true;
}

// Splits on ',' and keeps empty fields, including a trailing one.
// (getline alone would drop the empty field after a final comma.)
static std::vector<std::string> split_csv(const std::string& line) {
    std::vector<std::string> fields;
    std::size_t start = 0;
    while (true) {
        const std::size_t comma = line.find(',', start);
        if (comma == std::string::npos) {
            fields.push_back(line.substr(start));
            return fields;
        }
        fields.push_back(line.substr(start, comma - start));
        start = comma + 1;
    }
}

std::int64_t parse_amount_cents(const std::string& text) {
    const std::string amount = trim(text);
    const std::size_t dot = amount.find('.');
    const std::string dollars_text = amount.substr(0, dot);
    const std::string cents_text = dot == std::string::npos ? "" : amount.substr(dot + 1);

    if (!all_digits(dollars_text)) throw std::invalid_argument("invalid amount: '" + amount + "'");
    if (dot != std::string::npos && (cents_text.empty() || cents_text.size() > 2 || !all_digits(cents_text))) {
        throw std::invalid_argument("invalid amount: '" + amount + "'");
    }

    std::int64_t dollars = 0;
    try {
        dollars = std::stoll(dollars_text);
    } catch (const std::out_of_range&) {
        throw std::invalid_argument("amount too large: '" + amount + "'");
    }

    std::int64_t cents = 0;
    if (!cents_text.empty()) {
        cents = std::stoll(cents_text);
        if (cents_text.size() == 1) cents *= 10;  // "7.5" is 50 cents, not 5
    }

    if (dollars > (std::numeric_limits<std::int64_t>::max() - cents) / 100) {
        throw std::invalid_argument("amount too large: '" + amount + "'");
    }
    return dollars * 100 + cents;
}

std::string format_cents(std::int64_t cents) {
    const bool negative = cents < 0;
    // Work in unsigned so INT64_MIN doesn't overflow when negated.
    const std::uint64_t magnitude =
        negative ? ~static_cast<std::uint64_t>(cents) + 1 : static_cast<std::uint64_t>(cents);
    const std::uint64_t dollars = magnitude / 100;
    const std::uint64_t remainder = magnitude % 100;

    std::string result = negative ? "-$" : "$";
    result += std::to_string(dollars);
    result += '.';
    if (remainder < 10) result += '0';
    result += std::to_string(remainder);
    return result;
}

Transaction parse_transaction(const std::string& line) {
    const std::vector<std::string> fields = split_csv(line);
    if (fields.size() != 4) {
        throw std::invalid_argument("expected 4 fields but found " + std::to_string(fields.size()));
    }

    const std::string id = trim(fields[0]);
    const std::string account = trim(fields[1]);
    const std::string type_text = trim(fields[2]);

    if (id.empty()) throw std::invalid_argument("missing id");
    if (account.empty()) throw std::invalid_argument("missing account");

    TransactionType type;
    if (type_text == "CREDIT") {
        type = TransactionType::Credit;
    } else if (type_text == "DEBIT") {
        type = TransactionType::Debit;
    } else {
        throw std::invalid_argument("unknown type: '" + type_text + "'");
    }

    const std::int64_t amount_cents = parse_amount_cents(fields[3]);
    if (amount_cents == 0) throw std::invalid_argument("amount must be positive");

    return Transaction{id, account, type, amount_cents};
}

std::vector<Transaction> load_transactions(std::istream& in, std::vector<std::string>& errors) {
    std::vector<Transaction> transactions;
    std::unordered_set<std::string> seen_ids;

    std::string line;
    int line_number = 0;
    bool seen_first_row = false;
    while (std::getline(in, line)) {
        ++line_number;
        const std::string row = trim(line);
        if (row.empty()) continue;

        if (!seen_first_row) {
            seen_first_row = true;
            if (row == "id,account,type,amount") continue;
        }

        try {
            Transaction txn = parse_transaction(row);
            if (!seen_ids.insert(txn.id).second) {
                throw std::invalid_argument("duplicate id: '" + txn.id + "'");
            }
            transactions.push_back(std::move(txn));
        } catch (const std::invalid_argument& e) {
            errors.push_back("line " + std::to_string(line_number) + ": " + e.what());
        }
    }
    return transactions;
}

std::map<std::string, std::int64_t> balances_by_account(const std::vector<Transaction>& transactions) {
    std::map<std::string, std::int64_t> balances;
    for (const Transaction& txn : transactions) {
        // operator[] value-initializes a missing key to 0, which is what we want here.
        const std::int64_t signed_cents =
            txn.type == TransactionType::Credit ? txn.amount_cents : -txn.amount_cents;
        balances[txn.account] += signed_cents;
    }
    return balances;
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
