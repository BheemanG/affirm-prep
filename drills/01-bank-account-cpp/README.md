# Drill 01 (C++): Bank Account

**Time:** 25-35 minutes. This is a drill, not a full mock.
**Goal:** get comfortable writing a small, const-correct C++ class from scratch,
in a single file you could paste into HackerRank.

## Context

You're joining the accounts team at a payments company. Before you touch the
real ledger code, they want a small in-memory `BankAccount` model that other
engineers can use in tests and prototypes. All money is stored as **integer
cents** in a `std::int64_t`. No `double`, ever.

## Requirements

Implement `BankAccount` in `solution.cpp`. The stub gives you the
`Transaction` struct, the `TransactionKind` enum, and the public method
signatures. You write every method body and choose the data members.

- `BankAccount(owner, opening_balance_cents = 0)` creates an account.
- `owner()` returns the owner's name.
- `balance_cents()` returns the current balance. It must be callable on a
  `const BankAccount&`.
- `deposit(amount_cents)` adds money to the account.
- `withdraw(amount_cents)` removes money from the account.
- `deposit` and `withdraw` throw `std::invalid_argument` for an amount that
  is zero or negative.
- `withdraw` throws `std::invalid_argument` if the amount is more than the
  balance.
- `transfer_to(other, amount_cents)` moves money from this account to
  `other`. It is all-or-nothing: either both accounts change, or neither does.
  It follows the same amount and overdraft rules as `withdraw`.
- `history()` returns the account's transactions, oldest first, as a
  `std::vector<Transaction>`. Each successful operation adds one entry:
  - deposit: `{TransactionKind::Deposit, amount}`
  - withdraw: `{TransactionKind::Withdrawal, amount}`
  - transfer: `{TransactionKind::TransferOut, amount}` on the sender and
    `{TransactionKind::TransferIn, amount}` on the receiver
- **When any method throws, nothing about any account changes:** not the
  balance and not the history.

## Example

```cpp
BankAccount ada("Ada", 1000);
ada.deposit(250);
ada.withdraw(100);
ada.balance_cents();   // 1150
ada.history();         // {Deposit, 250}, {Withdrawal, 100}

ada.withdraw(5000);    // throws std::invalid_argument; balance still 1150

BankAccount bob("Bob");
ada.transfer_to(bob, 150);
ada.balance_cents();   // 1000
bob.balance_cents();   // 150
bob.history();         // {TransferIn, 150}
```

## Notes

- A couple of details are left for you to decide. If something isn't clear,
  ask me the way you would in a real interview.
- Keep everything in `solution.cpp`. `main` is wrapped in `#ifndef TESTING`
  so the tests can `#include` the file. Leave that guard in place.
- Until you implement the stubs, the compiler will warn about unused
  parameters. That's expected.

## Building and testing

From this folder:

```
make run     # build solution.cpp and run your main (scratch space)
make test    # build and run the visible tests
make test STD=c++17   # same, but as C++17 (HackerRank's version is unknown)
```

The visible tests only cover the happy path. Add your own tests for the edge
cases you think of: put them in `tests/test_account.cpp` using the `CHECK` and
`CHECK_THROWS` macros from `tests/check.h`.

## C++ things to practice here

- constructors and member initializer lists
- `explicit` single-argument constructors
- `const` member functions and `const&` return values
- `std::int64_t` and why `int` isn't enough for money
- `std::move` for constructor parameters you store
- throwing `std::invalid_argument` with a clear message
- validating input before changing any state
- private helpers to avoid repeating the same check
- why returning a `const&` is better than returning a non-const reference
  to internal state
