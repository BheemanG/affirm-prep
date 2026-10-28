# Drill 01: Bank Account

**Time:** 20-30 minutes. This is a drill, not a full mock.
**Goal:** get comfortable writing a small Python class from scratch.

## Context

You're joining the accounts team at a payments company. Before you touch the
real ledger code, they want a small in-memory `BankAccount` model that other
engineers can use in tests and prototypes. All money is stored as **integer
cents**. No floats, ever.

## Requirements

Implement `BankAccount` in `src/account.py`. The stub is already there.

- `BankAccount(owner, opening_balance_cents=0)` creates an account.
- `deposit(amount_cents)` adds money to the account.
- `withdraw(amount_cents)` removes money from the account.
- `deposit` and `withdraw` raise `ValueError` for a non-positive amount.
- `withdraw` raises `ValueError` if the amount is more than the balance. When
  it raises, nothing about the account changes.
- `balance_cents` is a **read-only** property.
- `history` is a list of transactions, oldest first. Each transaction is a
  frozen dataclass called `Transaction` with two fields: `kind` and
  `amount_cents`. Successful deposits have kind `"deposit"` and successful
  withdrawals have kind `"withdraw"`.
- `transfer_to(other, amount_cents)` moves money from this account to
  `other`. It must be atomic: either both accounts change, or neither does.
  Validate before you mutate anything.

## Examples

```
acct = BankAccount("Ada", 1000)
acct.deposit(250)
acct.withdraw(100)
acct.balance_cents      # 1150
acct.history            # [Transaction("deposit", 250), Transaction("withdraw", 100)]

acct.withdraw(5000)     # ValueError, balance still 1150
```

## Notes

- A few details are left for you to decide. If something isn't clear, ask me
  the way you would in a real interview.
- The starter stub only gives you signatures. You write the body of every
  method, and you write `Transaction` yourself.

## Running the tests

From this folder:

```
pytest
```

The visible tests only cover the happy path. Write your own tests for the
edge cases you can think of.

## Python things to practice here

- classes and `__init__`
- instance attributes vs. local variables
- properties (`@property`) and read-only attributes
- dataclasses and the `frozen` option
- raising exceptions with clear messages
- validating input before changing state
- naming conventions (snake_case, leading underscore for internals)
- mutable default arguments and why they bite
- exposing internal lists safely
- docstrings and type hints
