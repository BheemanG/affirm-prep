import pytest

from account import BankAccount, Transaction


def test_new_account_defaults_to_zero_balance():
    acct = BankAccount("Ada")
    assert acct.balance_cents == 0


def test_opening_balance_is_used():
    acct = BankAccount("Ada", 1000)
    assert acct.balance_cents == 1000


def test_deposit_increases_balance():
    acct = BankAccount("Ada", 1000)
    acct.deposit(250)
    assert acct.balance_cents == 1250


def test_withdraw_decreases_balance():
    acct = BankAccount("Ada", 1000)
    acct.withdraw(400)
    assert acct.balance_cents == 600


def test_withdraw_more_than_balance_raises_and_changes_nothing():
    acct = BankAccount("Ada", 1000)
    with pytest.raises(ValueError):
        acct.withdraw(1001)
    assert acct.balance_cents == 1000


def test_history_records_deposit_and_withdraw_in_order():
    acct = BankAccount("Ada", 1000)
    acct.deposit(250)
    acct.withdraw(100)
    assert acct.history == [
        Transaction("deposit", 250),
        Transaction("withdraw", 100),
    ]


def test_balance_cents_is_read_only():
    acct = BankAccount("Ada", 1000)
    with pytest.raises(AttributeError):
        acct.balance_cents = 5


def test_transfer_moves_money_between_accounts():
    src = BankAccount("Ada", 1000)
    dst = BankAccount("Bob", 200)
    src.transfer_to(dst, 300)
    assert src.balance_cents == 700
    assert dst.balance_cents == 500
