"""Starter stub for the BankAccount drill. Fill in the TODOs yourself."""


# TODO: define Transaction here (frozen dataclass with kind and amount_cents).
class Transaction:
    def __init__(self, kind, amount_cents):



class BankAccount:
    def __init__(self, owner, opening_balance_cents=0):
        self.owner = owner
        
        raise NotImplementedError

    @property
    def balance_cents(self):
        raise NotImplementedError

    @property
    def history(self):
        raise NotImplementedError

    def deposit(self, amount_cents):
        raise NotImplementedError

    def withdraw(self, amount_cents):
        raise NotImplementedError

    def transfer_to(self, other, amount_cents):
        raise NotImplementedError
