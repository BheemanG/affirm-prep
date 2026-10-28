import dataclasses

import pytest

from account import BankAccount, Transaction


# Interviewer answers to the README's open questions:
#   - failed operations are NOT recorded in history
#   - transfers record "transfer_out" (source) and "transfer_in" (destination)
#   - transferring to the same account raises ValueError
#   - a negative opening balance raises ValueError


# --- amount validation ---------------------------------------------------

@pytest.mark.parametrize("amount", [0, -1, -500])
def test_deposit_rejects_non_positive(amount):
    acct = BankAccount("Ada", 1000)
    with pytest.raises(ValueError):
        acct.deposit(amount)
    assert acct.balance_cents == 1000
    assert acct.history == []


@pytest.mark.parametrize("amount", [0, -1, -500])
def test_withdraw_rejects_non_positive(amount):
    acct = BankAccount("Ada", 1000)
    with pytest.raises(ValueError):
        acct.withdraw(amount)
    assert acct.balance_cents == 1000
    assert acct.history == []


def test_withdraw_exact_balance_is_allowed():
    acct = BankAccount("Ada", 1000)
    acct.withdraw(1000)
    assert acct.balance_cents == 0


def test_withdraw_one_over_balance_is_rejected():
    acct = BankAccount("Ada", 1000)
    with pytest.raises(ValueError):
        acct.withdraw(1001)
    assert acct.balance_cents == 1000
    assert acct.history == []


def test_negative_opening_balance_rejected():
    with pytest.raises(ValueError):
        BankAccount("Ada", -1)


# --- history -------------------------------------------------------------

def test_new_account_has_empty_history():
    assert BankAccount("Ada", 1000).history == []


def test_opening_balance_is_not_a_transaction():
    assert BankAccount("Ada", 1000).history == []


def test_mutating_returned_history_does_not_change_account():
    acct = BankAccount("Ada", 1000)
    acct.deposit(100)
    acct.history.append(Transaction("deposit", 999999))
    assert acct.history == [Transaction("deposit", 100)]


def test_histories_are_not_shared_between_accounts():
    a = BankAccount("Ada")
    b = BankAccount("Bob")
    a.deposit(100)
    assert b.history == []


def test_transaction_is_frozen_dataclass():
    assert dataclasses.is_dataclass(Transaction)
    txn = Transaction("deposit", 100)
    with pytest.raises(dataclasses.FrozenInstanceError):
        txn.amount_cents = 5


def test_transaction_fields_and_equality():
    assert Transaction("deposit", 100) == Transaction(kind="deposit", amount_cents=100)
    assert Transaction("deposit", 100) != Transaction("withdraw", 100)
    assert [f.name for f in dataclasses.fields(Transaction)] == ["kind", "amount_cents"]


# --- read-only balance ---------------------------------------------------

def test_balance_cents_cannot_be_assigned():
    acct = BankAccount("Ada", 1000)
    with pytest.raises(AttributeError):
        acct.balance_cents = 0
    assert acct.balance_cents == 1000


def test_history_cannot_be_reassigned():
    acct = BankAccount("Ada", 1000)
    with pytest.raises(AttributeError):
        acct.history = []


# --- transfer ------------------------------------------------------------

def test_transfer_records_history_on_both_accounts():
    src = BankAccount("Ada", 1000)
    dst = BankAccount("Bob", 0)
    src.transfer_to(dst, 300)
    assert src.history == [Transaction("transfer_out", 300)]
    assert dst.history == [Transaction("transfer_in", 300)]


def test_transfer_entire_balance():
    src = BankAccount("Ada", 1000)
    dst = BankAccount("Bob", 0)
    src.transfer_to(dst, 1000)
    assert src.balance_cents == 0
    assert dst.balance_cents == 1000


def test_transfer_insufficient_funds_changes_nothing():
    src = BankAccount("Ada", 100)
    dst = BankAccount("Bob", 50)
    with pytest.raises(ValueError):
        src.transfer_to(dst, 101)
    assert src.balance_cents == 100
    assert dst.balance_cents == 50
    assert src.history == []
    assert dst.history == []


@pytest.mark.parametrize("amount", [0, -1, -500])
def test_transfer_rejects_non_positive_and_changes_nothing(amount):
    src = BankAccount("Ada", 1000)
    dst = BankAccount("Bob", 50)
    with pytest.raises(ValueError):
        src.transfer_to(dst, amount)
    assert src.balance_cents == 1000
    assert dst.balance_cents == 50
    assert src.history == []
    assert dst.history == []


def test_transfer_to_self_rejected_and_changes_nothing():
    acct = BankAccount("Ada", 1000)
    with pytest.raises(ValueError):
        acct.transfer_to(acct, 100)
    assert acct.balance_cents == 1000
    assert acct.history == []


def test_transfer_preserves_total_money():
    a = BankAccount("Ada", 1000)
    b = BankAccount("Bob", 700)
    a.transfer_to(b, 250)
    b.transfer_to(a, 100)
    assert a.balance_cents + b.balance_cents == 1700


# --- mixed sequences -----------------------------------------------------

def test_mixed_sequence_balance_and_history():
    acct = BankAccount("Ada")
    acct.deposit(500)
    acct.deposit(250)
    acct.withdraw(100)
    with pytest.raises(ValueError):
        acct.withdraw(10_000)
    assert acct.balance_cents == 650
    assert acct.history == [
        Transaction("deposit", 500),
        Transaction("deposit", 250),
        Transaction("withdraw", 100),
    ]
