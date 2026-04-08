//
// Created by oz on 2/3/22.
//

#ifndef CNNF_NNF_HPP
#define CNNF_NNF_HPP

#include<string>
#include<vector>
#include<set>
#include<memory_resource>

#include <boost/multiprecision/gmp.hpp>

#include "var.hpp"

using boost::multiprecision::mpz_int;

extern std::pmr::unsynchronized_pool_resource MEM_POOL;

std::size_t constexpr ROOT = 1;

enum NodeType {
    And,
    Or,
    Unary,
    True,
    False
};

struct Edge {
    std::size_t id_dst;
    std::pmr::vector<Variable> freeVars{&MEM_POOL};
    std::pmr::vector<Literal> units{&MEM_POOL};
};

struct Node {
    NodeType type;
    std::vector<Edge> children;

    Node() {}
};


class NNF {
private:
    std::size_t nb_vars;
    std::vector<Node> nodes;
    std::vector<std::size_t> ordering;

    void compute_ordering();
public:
    NNF() = default;
    NNF(std::string const& path);
    NNF(std::string const& path, std::set<Variable> const& pvar);

    void init(std::string const& path, bool project, std::set<Variable> const& pvar);

    std::size_t nb_nodes() const;
    std::size_t nb_edges() const;

    Node& operator[](std::size_t i) {
        return nodes[i];
    }

    Node const& operator[](std::size_t i) const {
        return nodes[i];
    }

    inline std::vector<std::size_t> const& get_ordering() const { return ordering; }
};

class ANNF {
private:
    std::vector<mpz_int> mcv;
    NNF const& nnf;
    std::set<Literal> assumps;

    mpz_int get_mc(Edge const& e) const;
    mpz_int get_pc(Edge const& e) const;
public:
    ANNF(NNF const& f);
    // ANNF(NNF const& f, std::set<Literal> const& a);

    void annotate_mc();
    void annotate_pc();
    void set_assumps(std::set<Literal> const& a);
    void set_assumps(std::vector<Literal> const& a);

    mpz_int mc(std::size_t id) const;

    void get_path(mpz_int const& id, std::set<Literal> & s) const;
    void get_solution(mpz_int const& id, std::set<Literal> & s) const;
};


#endif //CNNF_NNF_HPP
