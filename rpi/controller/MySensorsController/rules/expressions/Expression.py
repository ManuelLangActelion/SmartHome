from typing import TypeVar, Generic


T = TypeVar('T')  # Declare type variable


class Expression(Generic[T]):

    def __init__(self, value: T) -> None:
        self.value = value

    def evaluate(self) -> T:
        return self.value
