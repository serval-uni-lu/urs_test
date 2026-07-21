#ifndef BITSET_HPP
#define BITSET_HPP

#include <boost/dynamic_bitset.hpp>

template <typename T>
struct BitSetTraits {
    static std::size_t to_size_t(const T& val);
    static T from_size_t(std::size_t const idx);
};

template<typename T>
class BitSet {
private:
    boost::dynamic_bitset<> set;

public:
    void insert(T const& e) {
        std::size_t p = BitSetTraits<T>::to_size_t(e);

        if(p >= set.size()) {
            set.resize(p + 1);
        }
        set.set(p, true);
    }

    void erase(T const& e) {
        std::size_t p = BitSetTraits<T>::to_size_t(e);

        if(p < set.size()) {
            set.set(p, false);
        }
    }

    bool contains(T const& e) const {
        std::size_t p = BitSetTraits<T>::to_size_t(e);

        return p < set.size() && set.test(p);
    }

    /**
      * @returns The number of bits in this bitset that are set to true.
      */
    inline std::size_t size() const {
        return set.count();
    }

    void intersect(BitSet & b) {
        //if(set.size() > b.set.size()) {
        //    set.resize(b.set.size());
        //}
        auto const sz = std::max(set.size(), b.set.size());
        set.resize(sz);
        b.set.resize(sz);
        set &= b.set;
    }

    bool empty() const {
        return set.none();
    }

    void clear() {
        set.reset();
    }

    class Iterator {
        private:
            boost::dynamic_bitset<> const& set;
            std::size_t pos;
        public:
            Iterator(boost::dynamic_bitset<> const& s, std::size_t const p) : set(s), pos(p) {}

            /**
              * @returns the current element according to the mapping defined in BitSetTraits
              *          if T == std::size_t, it returns the position
              */
            T operator*() const {
                return BitSetTraits<T>::from_size_t(pos);
            }

            bool operator!=(Iterator const& i) const {
                return pos != i.pos;
            }

            Iterator& operator++() {
                pos = set.find_next(pos);
                return *this;
            }
    };

    Iterator begin() const {
        return Iterator(set, set.find_first());
    }

    Iterator end() const {
        return Iterator(set, boost::dynamic_bitset<>::npos);
    }

    friend bool equals(BitSet<T> & a, BitSet<T> & b) {
        std::size_t const sz = std::max(a.set.size(), b.set.size());

        a.set.resize(sz);
        b.set.resize(sz);

        return a.set == b.set;
    }
};

template <>
struct BitSetTraits<std::size_t> {
    static std::size_t to_size_t(std::size_t const& v) {
        return v;
    }

    static std::size_t from_size_t(std::size_t const v) {
        return v;
    }
};

#endif
