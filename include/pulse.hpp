#pragma once
enum class Basis { Rectilinear, Diagonal };
enum class Bit { Zero, One };

struct Pulse {
    Basis basis;
    Bit bit;
    Pulse(Basis b, Bit v, double t = 0.0) : basis(b), bit(v) {}
};
