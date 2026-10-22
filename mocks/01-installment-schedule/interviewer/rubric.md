# Rubric: Installment Schedules

Scores are 1 to 4 per area: 1 = major gaps, 2 = works partly, 3 = solid new-grad, 4 = strong.

## Correctness and edge cases

- Part 1: amounts are int cents and sum exactly; remainder cents handled (one cent each, earliest first); dates are `purchase + 14*k` for k starting at 1; `amount_due_by` is inclusive of the as-of date; `ValueError` validation.
- Part 2: month-end clamping that returns to the anchor day (computed from the purchase date, not chained from the previous due date); leap years and year rollover; half-up rounding; last installment absorbs drift; 0% handled without dividing by zero.
- Part 3: oldest-first payment allocation; latest-first refund reduction; never below paid; cash-back computed correctly; idempotency; validation before mutation.
- 4 means all hidden tests pass with no money handled as float in Part 1 and a defensible numeric approach in Part 2.

## Code clarity and naming

- Money named with units (`_cents`), small helpers (for example `_add_months`), no deeply nested loops, no magic numbers (14, 26, 12 named).

## Structure and extensibility

- Did Part 2 slot in without a rewrite of Part 1? Signals of a good Part 1: due-date generation separated from amount calculation, `Installment` easy to extend.
- Part 3: separation of immutable schedule from mutable state, one place for allocation logic rather than copy-pasted loops.
- Red flags: hard-coded 14 or `timedelta` scattered through logic, interest logic tangled with split logic, mutating the frozen schedule.

## Testing

- Did they write their own tests beyond the visible ones? Look for: remainder case, single installment, invalid input, month-end dates, and for Part 3 a duplicate-id test and a refund-larger-than-owed test.
- Bonus for parametrized tests and tests written before or alongside code.

## Speed vs. quality trade-offs

- Part 1 target is about 15 minutes. Parts 2 and 3 should land inside the overall 45 to 60 minutes.
- Did they ask clarifying questions early (remainder placement, first due date) rather than guessing silently?
- Did they get a working version before polishing? Did they avoid over-engineering (no need for abstract base classes or a framework)?

## Things to call out in feedback

- Whether they asked about the two README ambiguities.
- Float usage for money.
- Chaining month additions (Jan 31 to Feb 28 to Mar 28) instead of anchoring on the original day.
- Applying a payment partially and then raising (non-atomic).
- Idempotency stored per call instead of per instance.
