# Interview Prep Progress

## Mock log

| # | Date | Mock | Type | Time taken | Correctness | Clarity | Structure | Testing | Speed vs. quality |
|---|------|------|------|------------|-------------|---------|-----------|---------|-------------------|
| 01 | 2026-10-22 | installment-schedule | Build from spec | in progress | | | | | |

## Drills

Short language-fundamentals exercises (not full mocks). Graded on correctness and language style (Python or C++) only.

| # | Date | Drill | Language | Focus | Time taken | Correctness | Style | Notes |
|---|------|-------|----------|-------|------------|-------------|-------|-------|
| 01 | 2026-10-27 | bank-account | Python | classes, properties, frozen dataclasses, validation | not started | | | |
| 01 | 2026-10-28 | bank-account-cpp | C++ | classes, const correctness, int64 cents, exceptions, validate-before-mutate | not recorded | 2/4 | 2/4 | Visible 6/6, hidden 19/21 (missed negative opening balance, self-transfer). Doesn't compile as C++17 (`Transaction(...)` paren aggregate init). No own tests. Validate-before-mutate was correct everywhere. |
| 02 | 2026-10-28 | transaction-parsing-cpp | C++ | getline/splitting, stoll pitfalls, CRLF, malformed rows, cents formatting | not started | | | |

## Recurring weaknesses

- **Doesn't ask about boundary/aliasing cases** (drill 01 C++): missed negative opening balance and transfer-to-self.
- **Writes no own edge-case tests** (drill 01 C++): left `tests/test_account.cpp` untouched.
- **C++17 portability** (drill 01 C++): used C++20-only paren aggregate init; always run `make test STD=c++17`.
- **C++ style basics** (drill 01 C++): `using namespace std;`, assignment in ctor body instead of init list + `std::move`, duplicated validation, vague exception messages, no units in member names.

## Topics covered

- Installment / payment schedules (01)

## Next up

_Chosen from the weaknesses above when you ask for the next mock._
