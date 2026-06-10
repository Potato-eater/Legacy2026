# test algorithm for triangulation
# finding ball location using robot location and ball direction

import math

class Vector:
    def __init__(self, i: float, j: float):
        self.i: float = i
        self.j: float = j
    def magnitude(self):
        return (self.i ** 2 + self.j ** 2) ** 0.5
    def heading(self):
        return math.atan2(self.j, self.i)
    def unit(self):
        magnitude = self.magnitude()
        if magnitude == 0:
            return Vector(0, 0)
        return Vector(self.i/magnitude, self.j/magnitude)
def dot(vec1: Vector, vec2: Vector):
    return vec1.i*vec2.i + vec1.j*vec2.j
def scale(vec1: Vector, scalar: float):
    return Vector(vec1.i * scalar, vec1.j * scalar)
def relative_to(vec1: Vector, vec2: Vector):
    return Vector(vec1.i-vec2.i, vec1.j-vec2.j)
def from_angle(angle: float, magnitude: float) -> Vector:
    return Vector(math.cos(angle)*magnitude, math.sin(angle)*magnitude);
    


def find_triangulated_pos(posv1: Vector, angle1: float, posv2: Vector, angle2: float):
    ballv1 = from_angle(angle1, 1)
    ballv2 = from_angle(angle2, 1)
    time1 = ((posv1.i-posv2.i)/ballv2.i - (posv1.j-posv2.j)/ballv2.j)/(ballv1.j/ballv2.j - ballv1.i/ballv2.i)
    ball_posv = Vector(posv1.i + time1 * ballv1.i, posv1.j + time1 * ballv1.j)
    return ball_posv




# change these.
pos1 = Vector(1, 2);
pos2 = Vector(2, 1);
angle1 = 0
angle2 = math.pi / 8



ball_pos: Vector = find_triangulated_pos(pos1, angle1, pos2, angle2)

print(ball_pos.i, ball_pos.j)