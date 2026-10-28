# Drill 01 (C++): Bank Account — interviewer notes

Graded on two areas only (drill, not full mock): **Correctness** and **C++ style**, 1-4 each.

## Answers to the intended clarifying questions

Answer these only if asked. Hidden tests assume these answers.

| Question | Answer |
|---|---|
| Can the opening balance be negative? | No. Throw `std::invalid_argument`. Zero is fine. |
| Does the opening balance appear in `history()`? | No. History starts empty. |
| What happens on `transfer_to` the same account? | Throw `std::invalid_argument`, change nothing. |
| Is withdrawing the exact balance OK? | Yes, balance becomes 0. |
| What about int64 overflow? | Good question. Assume realistic amounts; guarding it is a bonus, not tested. |
| Must the owner name be non-empty? | Not validated. Any string. |
| Should transfers record a counterparty? | No, just kind and amount. |

Credit asking about self-transfer and opening balance in the debrief, even if they guessed right.

## Correctness (1-4)

- **4**: all visible + hidden tests pass; validation happens before any mutation in every method; transfer checks the receiver too (self-transfer).
- **3**: misses one ambiguity case (usually self-transfer or negative opening balance) but every "change nothing on failure" test passes.
- **2**: a failed operation leaves a history entry or partial transfer, or the wrong exception type (`runtime_error`, `logic_error`).
- **1**: doesn't compile against the hidden tests, or core happy path broken.

Common bugs to look for:
- `history_.push_back` before the overdraft check → history gap on failure.
- transfer implemented as `withdraw` + `other.deposit` with no self-transfer guard: works by accident when `this == &other`, but records two entries and should throw.
- `int` instead of `int64_t` for members or locals → `large_balances_fit_in_int64` fails.
- `>=` vs `>` in the overdraft check → exact-balance withdrawal rejected.

## C++ style (1-4)

Look for:
- Member initializer list, `std::move(owner)` into the member.
- `const` on all accessors; `history()` returned by `const&` (signature was given, so check they didn't change it).
- Consistent member naming (`balance_cents_` or `m_balance_cents`), units in names.
- A private helper (e.g. `require_positive`) instead of duplicating the check in three places. `static` if it uses no state.
- Clear exception messages that say what was wrong.
- No `using namespace std;` at file scope, no raw `new`/`delete`, no `double`.
- Includes what it uses (`<utility>` for `std::move`, `<limits>` if they guard overflow).
- Their own `TEST`s for edge cases in `tests/test_account.cpp`.

Bonus (mention, don't require):
- Strong exception guarantee in `transfer_to`: `reserve` both histories before mutating, so a `bad_alloc` from `push_back` can't leave a half-done transfer.
- Overflow check on deposit / transfer receiver.

Scoring:
- **4**: idiomatic throughout, DRY validation, wrote edge-case tests.
- **3**: correct and readable, one or two nits (copy instead of move, duplicated checks).
- **2**: several issues: missing `const`, non-const reference to internals, `using namespace std`, vague messages.
- **1**: hard to read or unsafe (raw pointers, floating point money).
