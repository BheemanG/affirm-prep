# Drill 02 (C++): Transaction Parsing — interviewer notes

Graded on two areas only (drill, not full mock): **Correctness** and **C++ style**, 1-4 each.

## Answers to the intended clarifying questions

Answer these only if asked. Hidden tests assume these answers.

| Question | Answer |
|---|---|
| Is there a header row? | Optional. If the first non-blank line is exactly `id,account,type,amount`, skip it. A header-looking line later in the file is an ordinary (bad) row. |
| Blank lines? | Skip silently, but they still count toward line numbers. |
| Is `line N` physical (counting blanks and the header)? | Yes, 1-based physical line number. |
| Windows line endings / stray spaces? | Handle both: trim spaces, tabs and `\r` around every field and around the whole row. |
| Which amount formats are valid? | Digits, optionally `.` plus **1 or 2** digits. `7`, `7.5`, `7.50` are fine. Reject `.5`, `12.`, `1.234`, signs, `$`, thousands separators, empty. Never round. |
| Negative or zero amounts? | Both are invalid in a row. (`parse_amount_cents("0")` itself returns 0; the zero check is in `parse_transaction`.) |
| Is `type` case-sensitive? | Yes: exactly `CREDIT` or `DEBIT`. |
| Extra or missing columns? | Exactly 4 fields. A trailing comma means a 5th (empty) field, so it's an error. |
| Empty id or account? | Invalid. |
| Duplicate ids? | Keep the first, report the later ones as errors. A row that failed to parse does not claim its id. |
| Overflow? | Throw `std::invalid_argument` (not `out_of_range` leaking out). Largest valid amount is 92233720368547758.07. |
| Thousands separators in `format_cents`? | No: `$1234567.89`. |
| What does `balances_by_account` do with an account whose balance is 0? | Still listed. |

Credit asking about the trailing-comma / header / duplicate-id cases even if they guessed right.

## Correctness (1-4)

- **4**: all visible + hidden tests pass; no `double`; the four row-level gotchas below are handled.
- **3**: misses one or two of the ambiguity cases (usually duplicate ids, header placement, or trailing comma) but the core path is exact and one bad row never stops the file.
- **2**: wrong in the main paths: uses `double`, accepts `12abc`, lets an exception escape `load_transactions`, loses the line number, breaks on `\r\n`, or `"7.5"` becomes 705.
- **1**: doesn't compile against the hidden tests, or core happy path is broken.

Common bugs to look for:
- `std::stoll("12abc")` returns 12 without throwing; the code must check `pos` or validate digits first.
- `std::stoll` throws `std::out_of_range`, which is *not* an `std::invalid_argument`; one huge amount can kill the whole load.
- `"7.5"` -> `7*100 + 5 = 705`. The single-digit cents case needs scaling.
- `getline(ss, field, ',')` drops a trailing empty field, so `"t1,a,CREDIT,5.00,"` looks like 4 fields.
- `\r` left on the last field makes `"CREDIT\r"` / `"5.00\r"` fail on Windows files.
- `format_cents` with `cents % 100` on negatives gives a negative remainder (`-5 % 100 == -5`) and a missing zero pad (`$0.5`).
- Negating `INT64_MIN` is undefined behaviour (bonus; the hidden test only uses `INT64_MIN + 1`... and `INT64_MAX`).
- `balances[txn.account]` on `std::map` is fine here (default 0), but using `operator[]` just to *look up* an account elsewhere would insert it.
- Counting the line number only over *non-blank* rows, or over rows *after the header*.
- Marking an id as seen before the row has been fully validated.

## C++ style (1-4)

Look for:
- `getline` loop over the stream, `stringstream` or `find`-based splitting, in small `static` helpers (`trim`, `split`, digit check) rather than one 80-line function.
- `try/catch` only around the thing that can throw, catching specific exception types; `catch (...)` is a smell.
- Passing `const std::string&`, not by value, for read-only text; `errors` taken by non-const reference as the signature says.
- Constants/units clear: `amount_cents`, `balance_cents`.
- Clear exception messages that include the offending text.
- `std::move` when pushing a local `Transaction`; `reserve` is fine but not needed.
- No `using namespace std;`, no raw `new`/`delete`, no `double`/`atof`/`stod`.
- Includes what it uses (`<sstream>`, `<map>`, `<unordered_set>`/`<set>`, `<limits>`).
- Builds clean as `-std=c++17`.
- Their own `TEST`s for edge cases in `tests/test_parsing.cpp`.

Bonus (mention, don't require):
- Unsigned magnitude in `format_cents` so `INT64_MIN` is safe.
- Overflow check before `dollars * 100 + cents`.
- Splitting on `\r\n` once at the top of `load_transactions`.

Scoring:
- **4**: idiomatic throughout, small helpers, specific exception handling, wrote edge-case tests.
- **3**: correct and readable, one or two nits (one big function, copies, `catch (...)`).
- **2**: several issues: `using namespace std`, vague messages, duplicated trimming, no tests of their own.
- **1**: hard to read or unsafe (raw pointers, floating point money).
