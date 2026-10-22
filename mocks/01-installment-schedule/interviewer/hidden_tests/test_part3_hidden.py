from datetime import date

import pytest

from installments import PaymentPlan, build_schedule

PURCHASE = date(2026, 3, 1)


def make_plan(total=10000, n=4, **kwargs):
    return PaymentPlan(build_schedule(total, n, PURCHASE, **kwargs))


def paid(plan):
    return [i.paid_cents for i in plan.installments]


def owed(plan):
    return [i.amount_cents for i in plan.installments]


def test_new_plan_state():
    plan = make_plan()
    assert owed(plan) == [2500] * 4
    assert paid(plan) == [0] * 4
    assert plan.outstanding_cents() == 10000
    assert [i.number for i in plan.installments] == [1, 2, 3, 4]


def test_payment_fills_oldest_installment_first():
    plan = make_plan()
    plan.record_payment("p1", 2500)
    assert paid(plan) == [2500, 0, 0, 0]
    assert plan.outstanding_cents() == 7500


def test_partial_payment_spills_across_installments():
    plan = make_plan()
    plan.record_payment("p1", 1000)
    plan.record_payment("p2", 2000)
    assert paid(plan) == [2500, 500, 0, 0]


def test_payment_larger_than_one_installment():
    plan = make_plan()
    plan.record_payment("p1", 6000)
    assert paid(plan) == [2500, 2500, 1000, 0]


def test_duplicate_payment_id_is_ignored():
    plan = make_plan()
    plan.record_payment("p1", 2500)
    plan.record_payment("p1", 2500)
    assert plan.outstanding_cents() == 7500


def test_overpayment_rejected_and_nothing_applied():
    plan = make_plan()
    plan.record_payment("p1", 9000)
    with pytest.raises(ValueError):
        plan.record_payment("p2", 1001)
    assert plan.outstanding_cents() == 1000


@pytest.mark.parametrize("amount", [0, -5])
def test_non_positive_payment_rejected(amount):
    plan = make_plan()
    with pytest.raises(ValueError):
        plan.record_payment("p1", amount)


def test_outstanding_due_by_only_counts_unpaid_balance():
    plan = make_plan()
    plan.record_payment("p1", 3000)
    second_due = plan.installments[1].due_date
    assert plan.outstanding_due_by(second_due) == 2000
    assert plan.outstanding_due_by(plan.installments[0].due_date) == 0
    assert plan.outstanding_due_by(date(2000, 1, 1)) == 0


def test_refund_reduces_latest_installments_first():
    plan = make_plan()
    cash = plan.refund("r1", 4000)
    assert cash == 0
    assert owed(plan) == [2500, 2500, 1000, 0]
    assert plan.outstanding_cents() == 6000


def test_refund_never_reduces_below_amount_already_paid():
    plan = make_plan()
    plan.record_payment("p1", 3000)
    cash = plan.refund("r1", 8000)
    assert owed(plan) == [2500, 500, 0, 0]
    assert paid(plan) == [2500, 500, 0, 0]
    assert cash == 1000
    assert plan.outstanding_cents() == 0


def test_refund_after_full_payment_is_all_cash():
    plan = make_plan()
    plan.record_payment("p1", 10000)
    assert plan.refund("r1", 3000) == 3000
    assert owed(plan) == [2500] * 4


def test_multiple_refunds_accumulate():
    plan = make_plan()
    assert plan.refund("r1", 2500) == 0
    assert plan.refund("r2", 2500) == 0
    assert owed(plan) == [2500, 2500, 0, 0]


def test_duplicate_refund_id_is_idempotent():
    plan = make_plan()
    plan.record_payment("p1", 10000)
    first = plan.refund("r1", 3000)
    again = plan.refund("r1", 3000)
    assert first == again == 3000
    # The duplicate must not count toward the refund cap either.
    assert plan.refund("r2", 7000) == 7000


def test_refund_cannot_exceed_purchase_total():
    plan = make_plan()
    plan.refund("r1", 6000)
    with pytest.raises(ValueError):
        plan.refund("r2", 4001)
    assert plan.outstanding_cents() == 4000


@pytest.mark.parametrize("amount", [0, -1])
def test_non_positive_refund_rejected(amount):
    plan = make_plan()
    with pytest.raises(ValueError):
        plan.refund("r1", amount)


def test_payment_after_refund_only_needs_reduced_balance():
    plan = make_plan()
    plan.refund("r1", 4000)
    plan.record_payment("p1", 6000)
    assert plan.outstanding_cents() == 0
    with pytest.raises(ValueError):
        plan.record_payment("p2", 1)


def test_interest_bearing_plan_refund_reduces_amount_owed_without_recalculation():
    plan = make_plan(100000, 12, frequency="monthly", apr_bps=1200)
    cash = plan.refund("r1", 10000)
    assert cash == 0
    assert owed(plan)[-1] == 0
    assert owed(plan)[-2] == 8885 - (10000 - 8884)
    assert owed(plan)[:10] == [8885] * 10


def test_refund_cap_uses_principal_not_interest():
    plan = make_plan(100000, 12, frequency="monthly", apr_bps=1200)
    plan.refund("r1", 100000)
    with pytest.raises(ValueError):
        plan.refund("r2", 1)
