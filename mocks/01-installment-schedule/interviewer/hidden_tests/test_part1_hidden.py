from datetime import date, timedelta

import pytest

from installments import amount_due_by, build_schedule

PURCHASE = date(2026, 1, 31)


def amounts(schedule):
    return [i.amount_cents for i in schedule]


def test_first_installment_due_one_interval_after_purchase():
    schedule = build_schedule(10000, 4, PURCHASE)
    assert schedule[0].due_date == PURCHASE + timedelta(days=14)
    assert schedule[-1].due_date == PURCHASE + timedelta(days=56)


@pytest.mark.parametrize(
    "total, n, expected",
    [
        (10001, 4, [2501, 2500, 2500, 2500]),
        (10003, 4, [2501, 2501, 2501, 2500]),
        (100, 3, [34, 33, 33]),
        (2, 3, [1, 1, 0]),
        (1, 1, [1]),
    ],
)
def test_remainder_cents_go_to_earliest_installments(total, n, expected):
    assert amounts(build_schedule(total, n, PURCHASE)) == expected


@pytest.mark.parametrize("total", [1, 99, 10000, 123457, 99999999])
@pytest.mark.parametrize("n", [1, 2, 3, 4, 6, 7, 12])
def test_sum_is_exact_and_spread_is_at_most_one_cent(total, n):
    schedule = build_schedule(total, n, PURCHASE)
    values = amounts(schedule)
    assert len(values) == n
    assert sum(values) == total
    assert max(values) - min(values) <= 1


def test_amounts_are_ints():
    schedule = build_schedule(10001, 3, PURCHASE)
    assert all(type(i.amount_cents) is int for i in schedule)


@pytest.mark.parametrize(
    "total, n", [(0, 4), (-100, 4), (10000, 0), (10000, -1)]
)
def test_invalid_input_raises_value_error(total, n):
    with pytest.raises(ValueError):
        build_schedule(total, n, PURCHASE)


def test_amount_due_by_boundaries():
    schedule = build_schedule(10001, 4, PURCHASE)
    first, second = schedule[0].due_date, schedule[1].due_date
    assert amount_due_by(schedule, PURCHASE) == 0
    assert amount_due_by(schedule, first - timedelta(days=1)) == 0
    assert amount_due_by(schedule, first) == 2501
    assert amount_due_by(schedule, second - timedelta(days=1)) == 2501
    assert amount_due_by(schedule, second) == 5001
    assert amount_due_by(schedule, schedule[-1].due_date) == 10001


def test_amount_due_by_empty_schedule():
    assert amount_due_by([], PURCHASE) == 0
