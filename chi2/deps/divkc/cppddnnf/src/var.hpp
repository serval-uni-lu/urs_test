#ifndef VAR_HPP
#define VAR_HPP

#include<iostream>

struct Literal;
struct Variable;

struct Literal {
    private:
    int l;

    public:
    Literal (Variable v);
    Literal (Variable v, int sign);
    Literal (int i);

    Literal(Literal const& v) = default;
    Literal(Literal && v) = default;

    Literal& operator= (Literal const& p) = default;
    Literal& operator= (Literal && p) = default;

    inline int sign() const { return l & 1 ? -1 : 1; }
    inline int to_int() const { return sign() * ((l >> 1) + 1); }
    inline int get() const { return l; }

    bool operator == (Literal p) const;
    bool operator != (Literal p) const;
    bool operator <  (Literal p) const; // '<' makes p, ~p adjacent in the ordering.  

    friend inline Literal operator ~ (Literal p);
};

inline Literal operator ~ (Literal p) {
    p.l ^= 1;
    return p;
}

inline std::ostream & operator<<(std::ostream & out, Literal const& l) {
    out << l.to_int();
    //out << l.get();
    return out;
}

struct Variable {
    private:
    int v;

    public:
    Variable(int i);
    Variable(Literal l);

    Variable(Variable const& v) = default;
    Variable(Variable && v) = default;

    Variable& operator= (Variable const& p) = default;
    Variable& operator= (Variable && p) = default;

    inline int to_int() const { return v + 1; }
    inline int get() const { return v; }

    bool operator == (Variable p) const;
    bool operator != (Variable p) const;
    bool operator <  (Variable p) const;
};

#endif
