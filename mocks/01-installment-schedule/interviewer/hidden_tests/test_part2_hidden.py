from datetime import date

import pytest

from installments import amount_due_by, build_schedule


def dates(schedule):
    return [i.due_date for i in schedule]


def test_part1_behaviour_unchanged_by_default():
    schedule = build_schedule(10001, 4, date(2026, 3, 1))
    assert [i.amount_cents for i in schedule] == [2501, 2500, 2500, 2500]
    assert all(i.interest_cents == 0 for i in schedule)
    assert all(i.principal_cents == i.amount_cents for i in schedule)


def test_monthly_dates_clamp_to_month_end_and_return_to_anchor_day():
    schedule = build_schedule(40000, 4, date(2026, 1, 31), frequency="monthly")
    assert dates(schedule) == [
        date(2026, 2, 28),
        date(2026, 3, 31),
        date(2026, 4, 30),
        date(2026, 5, 31),
    ]


def test_monthly_dates_leap_year():
    schedule = build_schedule(30000, 3, date(2028, 1, 31), frequency="monthly")
    assert dates(schedule)[0] == date(2028, 2, 29)


def test_monthly_dates_cross_year_boundary():
    schedule = build_schedule(30000, 3, date(2026, 11, 15), frequency="monthly")
    assert dates(schedule) == [
        date(2026, 12, 15),
        date(2027, 1, 15),
        date(2027, 2, 15),
    ]


def test_monthly_zero_apr_still_splits_evenly():
    schedule = build_schedule(10001, 3, date(2026, 1, 1), frequency="monthly")
    assert [i.amount_cents for i in schedule] == [3334, 3334, 3333]


def test_amortized_monthly_known_values():
    # $1,000 at 12% APR over 12 months: standard payment is $88.85.
    schedule = build_schedule(
        100000, 12, date(2026, 1, 31), frequency="monthly", apr_bps=1200
    )
    assert schedule[0].amount_cents == 8885
    assert schedule[0].interest_cents == 1000
    assert schedule[0].principal_cents == 7885
    assert schedule[1].interest_cents == 921
    assert [i.amount_cents for i in schedule[:11]] == [8885] * 11
    last = schedule[-1]
    assert (last.amount_cents, last.principal_cents, last.interest_cents) == (
        8884,
        8796,
        88,
    )


def test_amortized_biweekly_uses_26_periods_per_year():
    schedule = build_schedule(100000, 6, date(2026, 1, 31), apr_bps=1200)
    assert schedule[0].interest_cents == 462
    assert schedule[0].amount_cents == 16937
    assert schedule[-1].interest_cents == 78


@pytest.mark.parametrize("apr_bps", [1, 499, 1200, 2999, 3600])
@pytest.mark.parametrize("n", [1, 2, 3, 6, 12, 24])
@pytest.mark.parametrize("frequency", ["biweekly", "monthly"])
def test_amortized_invariants(apr_bps, n, frequency):
    total = 123457
    schedule = build_schedule(
        total, n, date(2026, 5, 31), frequency=frequency, apr_bps=apr_bps
    )
    assert len(schedule) == n
    assert sum(i.principal_cents for i in schedule) == total
    assert all(i.amount_cents == i.principal_cents + i.interest_cents for i in schedule)
    assert all(i.principal_cents > 0 for i in schedule)
    assert all(i.interest_cents >= 0 for i in schedule)
    regular = schedule[:-1]
    if regular:
        assert len({i.amount_cents for i in regular}) == 1
        assert abs(schedule[-1].amount_cents - regular[0].amount_cents) <= n


def test_amount_due_by_includes_interest():
    schedule = build_schedule(
        100000, 12, date(2026, 1, 31), frequency="monthly", apr_bps=1200
    )
    assert amount_due_by(schedule, date(2026, 3, 31)) == 8885 * 2


def test_single_installment_with_interest_is_principal_plus_one_period_interest():
    schedule = build_schedule(
        100000, 1, date(2026, 1, 31), frequency="monthly", apr_bps=1200
    )
    assert schedule[0].interest_cents == 1000
    assert schedule[0].amount_cents == 101000


def test_invalid_frequency_raises():
    with pytest.raises(ValueError):
        build_schedule(10000, 4, date(2026, 1, 1), frequency="weekly")


def test_negative_apr_raises():
    with pytest.raises(ValueError):
        build_schedule(10000, 4, date(2026, 1, 1), apr_bps=-1)
