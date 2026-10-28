# Parts 2 and 3 (release one at a time, on `next`)

When releasing a part, show only the "Candidate-facing text" section, and add the matching visible tests to `tests/` (draw a representative handful from `hidden_tests/`; keep the nastiest edge cases hidden).

## Answers to Part 1 clarifying questions (only if asked)

| Question | Answer |
|---|---|
| Which installments get the leftover cents? | Spread them one cent at a time starting from installment 1. Amounts differ by at most 1 cent. |
| When is the first payment due? | One interval (14 days) after the purchase date. Not on the purchase date. |
| Validation / error type? | Already in the README. `ValueError`. |

If they never ask about either ambiguity, let hidden tests expose it and call it out in grading.

---

## Part 2: Candidate-facing text

The business now offers monthly plans and plans that charge interest.

Extend `build_schedule`:

```python
build_schedule(total_cents, num_installments, purchase_date,
               frequency="biweekly", apr_bps=0)
```

- `frequency` is `"biweekly"` (every 14 days, as before) or `"monthly"`. Anything else raises `ValueError`.
- `apr_bps` is the annual percentage rate in basis points (`1200` means 12.00%). Negative raises `ValueError`.
- Each `Installment` now also has `principal_cents` and `interest_cents`, and `amount_cents == principal_cents + interest_cents`. With `apr_bps=0`, behavior is exactly the same as Part 1 and interest is 0.
- Monthly installments fall on the same day of the month as the purchase date. If a month is too short, use its last day, but go back to the original day afterwards (purchase on Jan 31 gives Feb 28, Mar 31, Apr 30, ...).
- When `apr_bps > 0`, the plan is a standard amortizing loan:
  - The periodic rate is `apr / periods per year`, with 12 periods per year for monthly and 26 for biweekly.
  - Every installment except the last is the same level payment, rounded to the nearest cent (half up).
  - The interest in each installment is the outstanding principal balance times the periodic rate, rounded to the nearest cent (half up). The rest of the payment is principal.
  - The last installment pays off whatever principal remains, plus that period's interest, so it may differ by a few cents.
- `amount_due_by` keeps working and should include interest.

Example: $1,000.00 over 12 monthly installments at 12% APR has a level payment of $88.85, of which $10.00 is interest in the first installment.

### Interviewer notes

- Hidden tests live in `hidden_tests/test_part2_hidden.py`.
- The point: a plain `total // n` approach no longer works, the "extra cents to the earliest installments" rule applies only to the 0% case, and rounding drift gets absorbed by the final payment. Dates need real calendar math. `Installment` grows new fields.
- If asked: "Is interest charged on the first period?" Yes, the first installment is due one period after purchase, so a full period of interest has accrued.
- If asked about rounding mode: half up, not banker's rounding. Using `float` for the formula is acceptable if the known test values pass, but Decimal or integer math is better.
- If asked whether `apr_bps=0` should use the amortization formula: it can't (division by zero), so it keeps the Part 1 split.

---

## Part 3: Candidate-facing text

Customers actually pay, and sometimes they return part of what they bought. Add a class that tracks one customer's plan over time:

```python
plan = PaymentPlan(schedule)   # schedule from build_schedule
```

- `plan.installments` is a list (same order as the schedule) of objects with `number`, `due_date`, `amount_cents` (the amount currently owed for that installment) and `paid_cents`.
- `plan.outstanding_cents()` returns the total still owed across all installments.
- `plan.outstanding_due_by(as_of)` returns the unpaid amount on installments due on or before `as_of`.
- `plan.record_payment(payment_id, amount_cents)` records a payment.
  - It is applied to the earliest installments that still have a balance, and a payment may cover part of an installment or span several.
  - Payment events can be delivered more than once: repeating a `payment_id` must have no effect.
  - A payment that is not positive, or larger than the total outstanding, raises `ValueError` and changes nothing.
- `plan.refund(refund_id, amount_cents)` handles a partial refund of the purchase.
  - It reduces what the customer still owes, starting from the last installment and working backwards. An installment can never go below what has already been paid on it.
  - If the refund is more than the customer still owes, the leftover is returned as cash. `refund` returns the number of cents to pay back to the customer (0 if none).
  - Repeating a `refund_id` has no effect and returns the same value as the first time.
  - Total refunds can never exceed the original purchase amount (the sum of principal). Violations, or a refund that is not positive, raise `ValueError` and change nothing.

### Interviewer notes

- Hidden tests live in `hidden_tests/test_part3_hidden.py`.
- The point: the immutable schedule from Parts 1 and 2 is not enough; the candidate needs mutable per-installment state, idempotency (a set/dict of seen ids), allocation order, and atomic validation (check before mutating). Good candidates keep the schedule untouched and build state alongside it.
- If asked about interest on refunds: no recalculation. Refunds simply reduce amounts owed; the cap uses principal only.
- If asked about payment order: oldest first.
- If asked whether a refund can reduce the balance of an installment due in the past: yes, any installment, latest first.
- If asked about a duplicate id with a different amount: treat as a duplicate, ignore it.
- If asked about dates on payments/refunds: not needed.
