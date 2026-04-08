#include "var.hpp"

Literal::Literal (Variable v) : l((v.get() << 1)) {
}

Literal::Literal (Variable v, int sign) : l((v.get() << 1) | (sign < 0 ? 1 : 0)) {
}

Literal::Literal (int i) : l(((std::abs(i) - 1) << 1) ^ (i < 0 ? 1 : 0)) {
}

bool Literal::operator == (Literal p) const {
    return l == p.get();
}

bool Literal::operator != (Literal p) const {
    return l != p.get();
}

// '<' makes p, ~p adjacent in the ordering.
bool Literal::operator <  (Literal p) const {
    return l < p.get();
}  

Variable::Variable(int i) : v(std::abs(i) - 1) {
}

Variable::Variable(Literal l) : v(l.get() >> 1) {
}

bool Variable::operator == (Variable p) const {
    return v == p.v;
}

bool Variable::operator != (Variable p) const {
    return v != p.v;
}

bool Variable::operator <  (Variable p) const {
    return v < p.v;
}
