# run main.py to test

import math

class Vector:
    def __init__ (self, i:int|float, j:int|float) -> None:
        self.i = i
        self.j = j
    
    @property
    def magnitude(self) -> float:
        return (self.i**2+self.j**2)**(1/2)
    
    @property
    def heading(self) -> float:
        return math.atan2(self.j, self.i)
    
    def unit(self) -> "Vector":
        return Vector(self.i/self.magnitude, self.j/self.magnitude)
    
    def dot(self, vec:"Vector") -> float:
        return self.i*vec.i + self.j*vec.j
    
    def scale(self, scalar:float) -> "Vector":
        return Vector(self.i*scalar, self.j*scalar)
    
    def display(self) -> str:
        return f"<{self.i}, {self.j}>"
    
    @staticmethod
    def from_angle_magnitude(angle:float, magnitude:float) -> "Vector":
        return Vector(math.cos(angle)*magnitude, math.sin(angle)*magnitude)
    
    @staticmethod
    def sum_vectors(vectors:list["Vector"]) -> "Vector":
        sum_vector = Vector(0, 0)
        for vec in vectors:
            sum_vector.i += vec.i
            sum_vector.j += vec.j
        return sum_vector