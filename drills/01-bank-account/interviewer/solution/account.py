"""Reference solution for the BankAccount drill."""

from dataclasses import dataclass


@dataclass(frozen=True)
class Transaction:
    kind: str
    amount_cents: int


def _validate_amount(amount_cents):
    if isinstance(amount_cents, bool) or not isinstance(amount_cents, int):
        raise ValueError(f"amount must be an int number of cents, got {amount_cents!r}")
    if amount_cents <= 0:
        raise ValueError(f"amount must be positive, got {amount_cents}")


class BankAccount:
    def __init__(self, owner, opening_balance_cents=0):
        if opening_balance_cents < 0:
            raise ValueError("opening balance cannot be negative")
        self.owner = owner
        self._balance_cents = opening_balance_cents
        self._history = []

    @property
    def balance_cents(self):
        return self._balance_cents

    @property
    def history(self):
        # Copy so callers cannot mutate the account's records.
        return list(self._history)

    def deposit(self, amount_cents):
        _validate_amount(amount_cents)
        self._apply("deposit", amount_cents, amount_cents)

    def withdraw(self, amount_cents):
        _validate_amount(amount_cents)
        self._check_funds(amount_cents)
        self._apply("withdraw", amount_cents, -amount_cents)

    def transfer_to(self, other, amount_cents):
        if other is self:
            raise ValueError("cannot transfer to the same account")
        _validate_amount(amount_cents)
        self._check_funds(amount_cents)
        # All validation is done; nothing below can fail.
        self._apply("transfer_out", amount_cents, -amount_cents)
        other._apply("transfer_in", amount_cents, amount_cents)

    def _check_funds(self, amount_cents):
        if amount_cents > self._balance_cents:
            raise ValueError("insufficient funds")

    def _apply(self, kind, amount_cents, signed_delta):
        self._balance_cents += signed_delta
        self._history.append(Transaction(kind, amount_cents))
