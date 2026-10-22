from datetime import date, timedelta

from installments import amount_due_by, build_schedule

PURCHASE = date(2026, 3, 1)


def test_even_split():
    schedule = build_schedule(10000, 4, PURCHASE)
    assert [i.amount_cents for i in schedule] == [2500, 2500, 2500, 2500]


def test_numbers_start_at_one_and_increase():
    schedule = build_schedule(10000, 4, PURCHASE)
    assert [i.number for i in schedule] == [1, 2, 3, 4]


def test_installments_are_14_days_apart():
    schedule = build_schedule(10000, 4, PURCHASE)
    for earlier, later in zip(schedule, schedule[1:]):
        assert later.due_date - earlier.due_date == timedelta(days=14)


def test_amounts_always_add_up_to_total():
    schedule = build_schedule(10001, 4, PURCHASE)
    assert sum(i.amount_cents for i in schedule) == 10001


def test_single_installment():
    schedule = build_schedule(5000, 1, PURCHASE)
    assert len(schedule) == 1
    assert schedule[0].amount_cents == 5000


def test_amount_due_by_before_first_due_date_is_zero():
    schedule = build_schedule(10000, 4, PURCHASE)
    assert amount_due_by(schedule, schedule[0].due_date - timedelta(days=1)) == 0


def test_amount_due_by_includes_installments_due_on_that_date():
    schedule = build_schedule(10000, 4, PURCHASE)
    assert amount_due_by(schedule, schedule[1].due_date) == 5000


def test_amount_due_by_after_last_due_date_is_total():
    schedule = build_schedule(10000, 4, PURCHASE)
    assert amount_due_by(schedule, schedule[-1].due_date + timedelta(days=30)) == 10000
