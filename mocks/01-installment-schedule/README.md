# Installment Schedules

## Context

You're joining the payments team at a buy-now-pay-later company. When a customer checks out, they can choose to split their purchase into a fixed number of equal installments instead of paying all at once. Your first task is to build the piece that decides **how much** the customer owes and **when**.

## Part 1

Implement the two functions in `src/installments.py`.

### `build_schedule(total_cents, num_installments, purchase_date)`

Returns a list of `Installment` objects, ordered by due date.

- `total_cents` is the purchase amount in integer cents (e.g. `$100.00` is `10000`).
- `num_installments` is how many payments the customer will make.
- Payments are made every 14 days.
- The installment amounts must add up exactly to the purchase total.
- `Installment.number` starts at 1.
- Raise `ValueError` if `total_cents` is not positive or `num_installments` is less than 1.

### `amount_due_by(schedule, as_of)`

Returns the total (in cents) of all installments that are due on or before the date `as_of`.

### Example

A `$100.00` purchase split into 4 installments:

| number | amount_cents |
|--------|--------------|
| 1      | 2500         |
| 2      | 2500         |
| 3      | 2500         |
| 4      | 2500         |

## Running the tests

```
pytest
```

You are expected to add your own tests as you go.
