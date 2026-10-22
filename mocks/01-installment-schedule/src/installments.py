from dataclasses import dataclass
from datetime import date


@dataclass(frozen=True)
class Installment:
    number: int
    due_date: date
    amount_cents: int


def build_schedule(
    total_cents: int, num_installments: int, purchase_date: date
) -> list[Installment]:
    raise NotImplementedError


def amount_due_by(schedule: list[Installment], as_of: date) -> int:
    raise NotImplementedError
