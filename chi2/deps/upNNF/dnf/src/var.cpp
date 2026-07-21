#include "var.hpp"

Literal::Literal (Variable v) : l((v.get() << 1)) {
}

Literal::Literal (Variable v, int sign) : l((v.get() << 1) | (sign < 0 ? 1 : 0)) {
}

Literal::Literal (int i) : l(((std::abs(i) - 1) << 1) ^ (i < 0 ? 1 : 0)) {
}

std::size_t to_size_t(Literal const& l) {
    return l.get();
}

Literal from_size_t(std::size_t const& l) {
    Literal res(0);
    res.l = l;
    return res;
}

std::ostream& operator<<(std::ostream & out, BitSet<Literal> const& b) {
    for(Literal const l : b) {
        out << l << " ";
    }
    out << "0";
    return out;
}

Variable::Variable(int i) : v(std::abs(i) - 1) {
}

Variable::Variable(Literal l) : v(l.get() >> 1) {
}

bool Variable::operator == (Variable const& p) const {
    return v == p.v;
}

bool Variable::operator != (Variable const& p) const {
    return v != p.v;
}

bool Variable::operator <  (Variable const& p) const {
    return v < p.v;
}

std::ostream& operator<<(std::ostream & out, BitSet<Variable> const& b) {
    for(Variable const v : b) {
        out << v << " ";
    }
    out << "0";
    return out;
}
