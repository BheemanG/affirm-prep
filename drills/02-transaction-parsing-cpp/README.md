# Drill 02 (C++): Transaction Parsing

**Time:** 30-40 minutes. This is a drill, not a full mock.
**Goal:** practice the parsing habits that show up in almost every practical
fintech interview: `getline`, splitting, `stoll` pitfalls, bad input, and
turning cents back into dollars.

## Context

The payments team receives a nightly export of card transactions as CSV text.
Your job is to turn that text into `Transaction` values and produce a balance
per account. The export comes from several systems, so don't assume it is
clean. One bad row must never stop the rest of the file from being processed.

All money is **integer cents** in a `std::int64_t`. No `double`, ever. That
includes the parsing step: `"12.34"` must become `1234` without passing
through a floating-point number.

## Row format

```
id,account,type,amount
t1,acct-1,CREDIT,100.00
t2,acct-1,DEBIT,25.50
```

- `type` is `CREDIT` or `DEBIT`.
- `amount` is in dollars, e.g. `12.34`.

## Requirements

Implement the five functions in `solution.cpp`. The stub gives you the
`Transaction` struct and every signature.

1. `parse_amount_cents(text)`: dollars text to cents. `"12.34"` is `1234`,
   `"7"` is `700`, `"7.5"` is `750`. Throws `std::invalid_argument` for text
   that is not a valid amount.
2. `format_cents(cents)`: cents to text with a dollar sign and two decimals.
   `1234` is `"$12.34"`, `5` is `"$0.05"`, `0` is `"$0.00"`, and `-5` is
   `"-$0.05"`.
3. `parse_transaction(line)`: one row to one `Transaction`. Throws
   `std::invalid_argument` for a malformed row.
4. `load_transactions(in, errors)`: reads a whole stream. Valid rows are
   returned in file order. For each row that can't be used, append a message
   to `errors` that starts with the line number, like `"line 3: ..."`, and
   keep going.
5. `balances_by_account(transactions)`: net balance per account, in cents,
   ordered by account name. Credits add; debits subtract.

## Example

`cases/sample.in`:

```
id,account,type,amount
t1,acct-1,CREDIT,100.00
t2,acct-1,DEBIT,25.50
t3,acct-2,CREDIT,10
t4,acct-2,DEBIT,abc
t5,acct-1,DEBIT,0.05
```

`make run` prints:

```
error: line 5: ...
acct-1 $74.45
acct-2 $10.00
```

(The wording of the error message is yours; it must start with `line 5`.)

## Notes

- Some details are deliberately left open: what counts as a valid amount, what
  a "malformed row" is, what to do about the header line, and so on. If
  something isn't clear, ask me the way you would in a real interview.
- Keep everything in `solution.cpp`. `main` is wrapped in `#ifndef TESTING`
  so the tests can `#include` the file. Leave that guard in place.
- Until you implement the stubs, the compiler will warn about unused
  parameters. That's expected.

## Building and testing

From this folder:

```
make run                # build solution.cpp and feed it cases/sample.in
make test               # build and run the visible tests
make test STD=c++17     # same, but as C++17 (HackerRank's version is unknown)
```

The visible tests only cover the happy path. Add your own tests for the edge
cases you think of: put them in `tests/test_parsing.cpp` using the `CHECK` and
`CHECK_THROWS` macros from `tests/check.h`.

## C++ things to practice here

- `std::getline` on a stream, and on a `std::stringstream` with a delimiter
- what `getline(ss, field, ',')` does with an empty last field
- `std::stoll` and why it can succeed on `"12abc"` (check `pos`)
- catching `std::out_of_range` / `std::invalid_argument` from `stoll`
- Windows line endings (`\r\n`) when reading text
- `std::map::operator[]` inserts a default value on lookup
- integer division and `%` for splitting cents into dollars and cents
- `const&` parameters, `static` helpers in a single file
