"""Reference solution covering Parts 1-3."""

import calendar
from dataclasses import dataclass
from datetime import date, timedelta
from decimal import ROUND_HALF_UP, Decimal

PERIODS_PER_YEAR = {"biweekly": 26, "monthly": 12}


@dataclass(frozen=True)
class Installment:
    number: int
    due_date: date
    amount_cents: int
    principal_cents: int
    interest_cents: int


def _round_cents(value: Decimal) -> int:
    return int(value.quantize(Decimal(1), rounding=ROUND_HALF_UP))


def _add_months(anchor: date, months: int) -> date:
    index = anchor.year * 12 + (anchor.month - 1) + months
    year, month = divmod(index, 12)
    month += 1
    day = min(anchor.day, calendar.monthrange(year, month)[1])
    return date(year, month, day)


def _due_date(purchase_date: date, frequency: str, k: int) -> date:
    if frequency == "biweekly":
        return purchase_date + timedelta(days=14 * k)
    return _add_months(purchase_date, k)


def _validate(total_cents, num_installments, frequency, apr_bps):
    if total_cents <= 0:
        raise ValueError("total_cents must be positive")
    if num_installments < 1:
        raise ValueError("num_installments must be at least 1")
    if frequency not in PERIODS_PER_YEAR:
        raise ValueError(f"unknown frequency: {frequency}")
    if apr_bps < 0:
        raise ValueError("apr_bps must not be negative")


def build_schedule(
    total_cents: int,
    num_installments: int,
    purchase_date: date,
    frequency: str = "biweekly",
    apr_bps: int = 0,
) -> list[Installment]:
    _validate(total_cents, num_installments, frequency, apr_bps)
    dates = [_due_date(purchase_date, frequency, k) for k in range(1, num_installments + 1)]

    if apr_bps == 0:
        base, extra = divmod(total_cents, num_installments)
        amounts = [base + (1 if i < extra else 0) for i in range(num_installments)]
        return [
            Installment(i + 1, dates[i], amounts[i], amounts[i], 0)
            for i in range(num_installments)
        ]

    rate = Decimal(apr_bps) / Decimal(10000) / PERIODS_PER_YEAR[frequency]
    payment = _round_cents(
        Decimal(total_cents) * rate / (1 - (1 + rate) ** -num_installments)
    )
    balance = total_cents
    schedule = []
    for i in range(num_installments):
        interest = _round_cents(Decimal(balance) * rate)
        if i == num_installments - 1:
            principal = balance
        else:
            principal = min(payment - interest, balance)
        schedule.append(
            Installment(i + 1, dates[i], principal + interest, principal, interest)
        )
        balance -= principal
    return schedule


def amount_due_by(schedule: list[Installment], as_of: date) -> int:
    return sum(i.amount_cents for i in schedule if i.due_date <= as_of)


@dataclass
class InstallmentState:
    number: int
    due_date: date
    amount_cents: int
    paid_cents: int = 0

    @property
    def outstanding_cents(self) -> int:
        return self.amount_cents - self.paid_cents


class PaymentPlan:
    def __init__(self, schedule: list[Installment]):
        self.installments = [
            InstallmentState(i.number, i.due_date, i.amount_cents) for i in schedule
        ]
        self._principal_total = sum(i.principal_cents for i in schedule)
        self._refunded_total = 0
        self._payment_ids: set[str] = set()
        self._refund_results: dict[str, int] = {}

    def outstanding_cents(self) -> int:
        return sum(i.outstanding_cents for i in self.installments)

    def outstanding_due_by(self, as_of: date) -> int:
        return sum(
            i.outstanding_cents for i in self.installments if i.due_date <= as_of
        )

    def record_payment(self, payment_id: str, amount_cents: int) -> None:
        if payment_id in self._payment_ids:
            return
        if amount_cents <= 0:
            raise ValueError("payment must be positive")
        if amount_cents > self.outstanding_cents():
            raise ValueError("payment exceeds outstanding balance")
        self._payment_ids.add(payment_id)
        remaining = amount_cents
        for inst in self.installments:
            applied = min(remaining, inst.outstanding_cents)
            inst.paid_cents += applied
            remaining -= applied
            if remaining == 0:
                break

    def refund(self, refund_id: str, amount_cents: int) -> int:
        if refund_id in self._refund_results:
            return self._refund_results[refund_id]
        if amount_cents <= 0:
            raise ValueError("refund must be positive")
        if self._refunded_total + amount_cents > self._principal_total:
            raise ValueError("refund exceeds purchase total")
        remaining = amount_cents
        for inst in reversed(self.installments):
            reduction = min(remaining, inst.outstanding_cents)
            inst.amount_cents -= reduction
            remaining -= reduction
            if remaining == 0:
                break
        self._refunded_total += amount_cents
        self._refund_results[refund_id] = remaining
        return remaining
