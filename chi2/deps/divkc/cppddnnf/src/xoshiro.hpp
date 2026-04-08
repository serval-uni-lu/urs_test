/*  Oridinally written in 2019 by David Blackman and Sebastiano Vigna (vigna@acm.org)
    Adapted to C++ in 2025 by Olivier Zeyen

To the extent possible under law, the author has dedicated all copyright
and related and neighboring rights to this software to the public domain
worldwide.

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. */

#ifndef XOSHIRO_HPP
#define XOSHIRO_HPP

#include <random>
#include <limits>

#include <stdint.h>
#include <string.h>

static inline uint64_t rotl(const uint64_t x, int k) {
    return (x << k) | (x >> (64 - k));
}

class xoshiro512starstar {
    private:
        static uint64_t constexpr state_size = 8;
        uint64_t s[state_size];

    public:
        using result_type = uint64_t;

        inline static result_type constexpr min() { return std::numeric_limits<result_type>::min(); }
        inline static result_type constexpr max() { return std::numeric_limits<result_type>::max(); }

        xoshiro512starstar() { this->seed(); }

        xoshiro512starstar(uint64_t const* seed) { this->seed(seed); }

        xoshiro512starstar(std::random_device & dev) { this->seed(dev); }

        void seed() { memset(s, 0, sizeof(uint64_t) * state_size); }

        void seed(uint64_t const* seed) { memcpy(s, seed, sizeof(uint64_t) * state_size); }

        void seed(std::random_device & dev) {
            std::size_t c = (sizeof(uint64_t) / sizeof(std::random_device::result_type));

            if(c >= 1) {
                for(std::size_t i = 0; i < state_size; i++) {
                    for(std::size_t j = 0; j < c; j++) {
                        s[i] <<= sizeof(std::random_device::result_type) * 8;
                        s[i] = s[i] | dev();
                    }
                }
            }
            else {
                for(std::size_t i = 0; i < state_size; i++) {
                    s[i] = static_cast<uint64_t>(dev());
                }
            }
        }

        uint64_t operator()(void) {
            const uint64_t result = rotl(s[1] * 5, 7) * 9;

            const uint64_t t = s[1] << 11;

            s[2] ^= s[0];
            s[5] ^= s[1];
            s[1] ^= s[2];
            s[7] ^= s[3];
            s[3] ^= s[4];
            s[4] ^= s[5];
            s[0] ^= s[6];
            s[6] ^= s[7];

            s[6] ^= t;

            s[7] = rotl(s[7], 21);

            return result;
        }

        void jump(void) {
            static const uint64_t JUMP[] = { 0x33ed89b6e7a353f9, 0x760083d7955323be, 0x2837f2fbb5f22fae, 0x4b8c5674d309511c, 0xb11ac47a7ba28c25, 0xf1be7667092bcc1c, 0x53851efdb6df0aaf, 0x1ebbc8b23eaf25db };

            uint64_t t[sizeof s / sizeof *s];
            memset(t, 0, sizeof t);
            for(std::size_t i = 0; i < sizeof JUMP / sizeof *JUMP; i++)
                for(int b = 0; b < 64; b++) {
                    if (JUMP[i] & UINT64_C(1) << b)
                        for(std::size_t w = 0; w < sizeof s / sizeof *s; w++)
                            t[w] ^= s[w];
                    (*this)();
                }

            memcpy(s, t, sizeof s);	
        }

        void long_jump(void) {
            static const uint64_t LONG_JUMP[] = { 0x11467fef8f921d28, 0xa2a819f2e79c8ea8, 0xa8299fc284b3959a, 0xb4d347340ca63ee1, 0x1cb0940bedbff6ce, 0xd956c5c4fa1f8e17, 0x915e38fd4eda93bc, 0x5b3ccdfa5d7daca5 };

            uint64_t t[sizeof s / sizeof *s];
            memset(t, 0, sizeof t);
            for(std::size_t i = 0; i < sizeof LONG_JUMP / sizeof *LONG_JUMP; i++)
                for(int b = 0; b < 64; b++) {
                    if (LONG_JUMP[i] & UINT64_C(1) << b)
                        for(std::size_t w = 0; w < sizeof s / sizeof *s; w++)
                            t[w] ^= s[w];
                    (*this)();
                }

            memcpy(s, t, sizeof s);	
        }
};

class xoshiro512plusplus {
    private:
        static uint64_t constexpr state_size = 8;
        uint64_t s[state_size];

    public:
        using result_type = uint64_t;

        inline static result_type constexpr min() { return std::numeric_limits<result_type>::min(); }
        inline static result_type constexpr max() { return std::numeric_limits<result_type>::max(); }

        xoshiro512plusplus() { this->seed(); }

        xoshiro512plusplus(uint64_t const* seed) { this->seed(seed); }

        xoshiro512plusplus(std::random_device & dev) { this->seed(dev); }

        void seed() { memset(s, 0, sizeof(uint64_t) * state_size); }

        void seed(uint64_t const* seed) { memcpy(s, seed, sizeof(uint64_t) * state_size); }

        void seed(std::random_device & dev) {
            std::size_t c = (sizeof(uint64_t) / sizeof(std::random_device::result_type));

            if(c >= 1) {
                for(std::size_t i = 0; i < state_size; i++) {
                    for(std::size_t j = 0; j < c; j++) {
                        s[i] <<= sizeof(std::random_device::result_type) * 8;
                        s[i] = s[i] | dev();
                    }
                }
            }
            else {
                for(std::size_t i = 0; i < state_size; i++) {
                    s[i] = static_cast<uint64_t>(dev());
                }
            }
        }

        uint64_t operator()(void) {
            const uint64_t result = rotl(s[0] + s[2], 17) + s[2];

            const uint64_t t = s[1] << 11;

            s[2] ^= s[0];
            s[5] ^= s[1];
            s[1] ^= s[2];
            s[7] ^= s[3];
            s[3] ^= s[4];
            s[4] ^= s[5];
            s[0] ^= s[6];
            s[6] ^= s[7];

            s[6] ^= t;

            s[7] = rotl(s[7], 21);

            return result;
        }

        void jump(void) {
            static const uint64_t JUMP[] = { 0x33ed89b6e7a353f9, 0x760083d7955323be, 0x2837f2fbb5f22fae, 0x4b8c5674d309511c, 0xb11ac47a7ba28c25, 0xf1be7667092bcc1c, 0x53851efdb6df0aaf, 0x1ebbc8b23eaf25db };

            uint64_t t[sizeof s / sizeof *s];
            memset(t, 0, sizeof t);
            for(std::size_t i = 0; i < sizeof JUMP / sizeof *JUMP; i++)
                for(int b = 0; b < 64; b++) {
                    if (JUMP[i] & UINT64_C(1) << b)
                        for(std::size_t w = 0; w < sizeof s / sizeof *s; w++)
                            t[w] ^= s[w];
                    (*this)();
                }

            memcpy(s, t, sizeof s);	
        }

        void long_jump(void) {
            static const uint64_t LONG_JUMP[] = { 0x11467fef8f921d28, 0xa2a819f2e79c8ea8, 0xa8299fc284b3959a, 0xb4d347340ca63ee1, 0x1cb0940bedbff6ce, 0xd956c5c4fa1f8e17, 0x915e38fd4eda93bc, 0x5b3ccdfa5d7daca5 };

            uint64_t t[sizeof s / sizeof *s];
            memset(t, 0, sizeof t);
            for(std::size_t i = 0; i < sizeof LONG_JUMP / sizeof *LONG_JUMP; i++)
                for(int b = 0; b < 64; b++) {
                    if (LONG_JUMP[i] & UINT64_C(1) << b)
                        for(std::size_t w = 0; w < sizeof s / sizeof *s; w++)
                            t[w] ^= s[w];
                    (*this)();
                }

            memcpy(s, t, sizeof s);	
        }

};

class xoshiro512plus {
    private:
        static uint64_t constexpr state_size = 8;
        uint64_t s[state_size];

    public:
        using result_type = uint64_t;

        inline static result_type constexpr min() { return std::numeric_limits<result_type>::min(); }
        inline static result_type constexpr max() { return std::numeric_limits<result_type>::max(); }

        xoshiro512plus() { this->seed(); }

        xoshiro512plus(uint64_t const* seed) { this->seed(seed); }

        xoshiro512plus(std::random_device & dev) { this->seed(dev); }

        void seed() { memset(s, 0, sizeof(uint64_t) * state_size); }

        void seed(uint64_t const* seed) { memcpy(s, seed, sizeof(uint64_t) * state_size); }

        void seed(std::random_device & dev) {
            std::size_t c = (sizeof(uint64_t) / sizeof(std::random_device::result_type));

            if(c >= 1) {
                for(std::size_t i = 0; i < state_size; i++) {
                    for(std::size_t j = 0; j < c; j++) {
                        s[i] <<= sizeof(std::random_device::result_type) * 8;
                        s[i] = s[i] | dev();
                    }
                }
            }
            else {
                for(std::size_t i = 0; i < state_size; i++) {
                    s[i] = static_cast<uint64_t>(dev());
                }
            }
        }

        uint64_t operator()(void) {
            const uint64_t result = s[0] + s[2];

            const uint64_t t = s[1] << 11;

            s[2] ^= s[0];
            s[5] ^= s[1];
            s[1] ^= s[2];
            s[7] ^= s[3];
            s[3] ^= s[4];
            s[4] ^= s[5];
            s[0] ^= s[6];
            s[6] ^= s[7];

            s[6] ^= t;

            s[7] = rotl(s[7], 21);

            return result;
        }

        void jump(void) {
            static const uint64_t JUMP[] = { 0x33ed89b6e7a353f9, 0x760083d7955323be, 0x2837f2fbb5f22fae, 0x4b8c5674d309511c, 0xb11ac47a7ba28c25, 0xf1be7667092bcc1c, 0x53851efdb6df0aaf, 0x1ebbc8b23eaf25db };

            uint64_t t[sizeof s / sizeof *s];
            memset(t, 0, sizeof t);
            for(std::size_t i = 0; i < sizeof JUMP / sizeof *JUMP; i++)
                for(int b = 0; b < 64; b++) {
                    if (JUMP[i] & UINT64_C(1) << b)
                        for(std::size_t w = 0; w < sizeof s / sizeof *s; w++)
                            t[w] ^= s[w];
                    (*this)();
                }

            memcpy(s, t, sizeof s);
        }

        void long_jump(void) {
            static const uint64_t LONG_JUMP[] = { 0x11467fef8f921d28, 0xa2a819f2e79c8ea8, 0xa8299fc284b3959a, 0xb4d347340ca63ee1, 0x1cb0940bedbff6ce, 0xd956c5c4fa1f8e17, 0x915e38fd4eda93bc, 0x5b3ccdfa5d7daca5 };

            uint64_t t[sizeof s / sizeof *s];
            memset(t, 0, sizeof t);
            for(std::size_t i = 0; i < sizeof LONG_JUMP / sizeof *LONG_JUMP; i++)
                for(int b = 0; b < 64; b++) {
                    if (LONG_JUMP[i] & UINT64_C(1) << b)
                        for(std::size_t w = 0; w < sizeof s / sizeof *s; w++)
                            t[w] ^= s[w];
                    (*this)();
                }

            memcpy(s, t, sizeof s);
        }
};

class xoshiro256starstar {
    private:
        static uint64_t constexpr state_size = 4;
        uint64_t s[state_size];

    public:
        using result_type = uint64_t;

        inline static result_type constexpr min() { return std::numeric_limits<result_type>::min(); }
        inline static result_type constexpr max() { return std::numeric_limits<result_type>::max(); }

        xoshiro256starstar() { this->seed(); }

        xoshiro256starstar(uint64_t const* seed) { this->seed(seed); }

        xoshiro256starstar(std::random_device & dev) { this->seed(dev); }

        void seed() { memset(s, 0, sizeof(uint64_t) * state_size); }

        void seed(uint64_t const* seed) { memcpy(s, seed, sizeof(uint64_t) * state_size); }

        void seed(std::random_device & dev) {
            std::size_t c = (sizeof(uint64_t) / sizeof(std::random_device::result_type));

            if(c >= 1) {
                for(std::size_t i = 0; i < state_size; i++) {
                    for(std::size_t j = 0; j < c; j++) {
                        s[i] <<= sizeof(std::random_device::result_type) * 8;
                        s[i] = s[i] | dev();
                    }
                }
            }
            else {
                for(std::size_t i = 0; i < state_size; i++) {
                    s[i] = static_cast<uint64_t>(dev());
                }
            }
        }

        uint64_t operator()(void) {
            const uint64_t result = rotl(s[1] * 5, 7) * 9;

            const uint64_t t = s[1] << 17;

            s[2] ^= s[0];
            s[3] ^= s[1];
            s[1] ^= s[2];
            s[0] ^= s[3];

            s[2] ^= t;

            s[3] = rotl(s[3], 45);

            return result;
        }

        void jump(void) {
            static const uint64_t JUMP[] = { 0x180ec6d33cfd0aba, 0xd5a61266f0c9392c, 0xa9582618e03fc9aa, 0x39abdc4529b1661c };

            uint64_t s0 = 0;
            uint64_t s1 = 0;
            uint64_t s2 = 0;
            uint64_t s3 = 0;
            for(std::size_t i = 0; i < sizeof JUMP / sizeof *JUMP; i++)
                for(int b = 0; b < 64; b++) {
                    if (JUMP[i] & UINT64_C(1) << b) {
                        s0 ^= s[0];
                        s1 ^= s[1];
                        s2 ^= s[2];
                        s3 ^= s[3];
                    }
                    (*this)();	
                }

            s[0] = s0;
            s[1] = s1;
            s[2] = s2;
            s[3] = s3;
        }

        void long_jump(void) {
            static const uint64_t LONG_JUMP[] = { 0x76e15d3efefdcbbf, 0xc5004e441c522fb3, 0x77710069854ee241, 0x39109bb02acbe635 };

            uint64_t s0 = 0;
            uint64_t s1 = 0;
            uint64_t s2 = 0;
            uint64_t s3 = 0;
            for(std::size_t i = 0; i < sizeof LONG_JUMP / sizeof *LONG_JUMP; i++)
                for(int b = 0; b < 64; b++) {
                    if (LONG_JUMP[i] & UINT64_C(1) << b) {
                        s0 ^= s[0];
                        s1 ^= s[1];
                        s2 ^= s[2];
                        s3 ^= s[3];
                    }
                    (*this)();	
                }

            s[0] = s0;
            s[1] = s1;
            s[2] = s2;
            s[3] = s3;
        }
};

class xoshiro256plusplus {
    private:
        static uint64_t constexpr state_size = 4;
        uint64_t s[state_size];

    public:
        using result_type = uint64_t;

        inline static result_type constexpr min() { return std::numeric_limits<result_type>::min(); }
        inline static result_type constexpr max() { return std::numeric_limits<result_type>::max(); }

        xoshiro256plusplus() { this->seed(); }

        xoshiro256plusplus(uint64_t const* seed) { this->seed(seed); }

        xoshiro256plusplus(std::random_device & dev) { this->seed(dev); }

        void seed() { memset(s, 0, sizeof(uint64_t) * state_size); }

        void seed(uint64_t const* seed) { memcpy(s, seed, sizeof(uint64_t) * state_size); }

        void seed(std::random_device & dev) {
            std::size_t c = (sizeof(uint64_t) / sizeof(std::random_device::result_type));

            if(c >= 1) {
                for(std::size_t i = 0; i < state_size; i++) {
                    for(std::size_t j = 0; j < c; j++) {
                        s[i] <<= sizeof(std::random_device::result_type) * 8;
                        s[i] = s[i] | dev();
                    }
                }
            }
            else {
                for(std::size_t i = 0; i < state_size; i++) {
                    s[i] = static_cast<uint64_t>(dev());
                }
            }
        }

        uint64_t operator()(void) {
            const uint64_t result = rotl(s[0] + s[3], 23) + s[0];

            const uint64_t t = s[1] << 17;

            s[2] ^= s[0];
            s[3] ^= s[1];
            s[1] ^= s[2];
            s[0] ^= s[3];

            s[2] ^= t;

            s[3] = rotl(s[3], 45);

            return result;
        }

        void jump(void) {
            static const uint64_t JUMP[] = { 0x180ec6d33cfd0aba, 0xd5a61266f0c9392c, 0xa9582618e03fc9aa, 0x39abdc4529b1661c };

            uint64_t s0 = 0;
            uint64_t s1 = 0;
            uint64_t s2 = 0;
            uint64_t s3 = 0;
            for(std::size_t i = 0; i < sizeof JUMP / sizeof *JUMP; i++)
                for(int b = 0; b < 64; b++) {
                    if (JUMP[i] & UINT64_C(1) << b) {
                        s0 ^= s[0];
                        s1 ^= s[1];
                        s2 ^= s[2];
                        s3 ^= s[3];
                    }
                    (*this)();	
                }

            s[0] = s0;
            s[1] = s1;
            s[2] = s2;
            s[3] = s3;
        }

        void long_jump(void) {
            static const uint64_t LONG_JUMP[] = { 0x76e15d3efefdcbbf, 0xc5004e441c522fb3, 0x77710069854ee241, 0x39109bb02acbe635 };

            uint64_t s0 = 0;
            uint64_t s1 = 0;
            uint64_t s2 = 0;
            uint64_t s3 = 0;
            for(std::size_t i = 0; i < sizeof LONG_JUMP / sizeof *LONG_JUMP; i++)
                for(int b = 0; b < 64; b++) {
                    if (LONG_JUMP[i] & UINT64_C(1) << b) {
                        s0 ^= s[0];
                        s1 ^= s[1];
                        s2 ^= s[2];
                        s3 ^= s[3];
                    }
                    (*this)();	
                }

            s[0] = s0;
            s[1] = s1;
            s[2] = s2;
            s[3] = s3;
        }
};

class xoshiro256plus {
    private:
        static uint64_t constexpr state_size = 4;
        uint64_t s[state_size];

    public:
        using result_type = uint64_t;

        inline static result_type constexpr min() { return std::numeric_limits<result_type>::min(); }
        inline static result_type constexpr max() { return std::numeric_limits<result_type>::max(); }

        xoshiro256plus() { this->seed(); }

        xoshiro256plus(uint64_t const* seed) { this->seed(seed); }

        xoshiro256plus(std::random_device & dev) { this->seed(dev); }

        void seed() { memset(s, 0, sizeof(uint64_t) * state_size); }

        void seed(uint64_t const* seed) { memcpy(s, seed, sizeof(uint64_t) * state_size); }

        void seed(std::random_device & dev) {
            std::size_t c = (sizeof(uint64_t) / sizeof(std::random_device::result_type));

            if(c >= 1) {
                for(std::size_t i = 0; i < state_size; i++) {
                    for(std::size_t j = 0; j < c; j++) {
                        s[i] <<= sizeof(std::random_device::result_type) * 8;
                        s[i] = s[i] | dev();
                    }
                }
            }
            else {
                for(std::size_t i = 0; i < state_size; i++) {
                    s[i] = static_cast<uint64_t>(dev());
                }
            }
        }

        uint64_t operator()(void) {
            const uint64_t result = s[0] + s[3];

            const uint64_t t = s[1] << 17;

            s[2] ^= s[0];
            s[3] ^= s[1];
            s[1] ^= s[2];
            s[0] ^= s[3];

            s[2] ^= t;

            s[3] = rotl(s[3], 45);

            return result;
        }

        void jump(void) {
            static const uint64_t JUMP[] = { 0x180ec6d33cfd0aba, 0xd5a61266f0c9392c, 0xa9582618e03fc9aa, 0x39abdc4529b1661c };

            uint64_t s0 = 0;
            uint64_t s1 = 0;
            uint64_t s2 = 0;
            uint64_t s3 = 0;
            for(std::size_t i = 0; i < sizeof JUMP / sizeof *JUMP; i++)
                for(int b = 0; b < 64; b++) {
                    if (JUMP[i] & UINT64_C(1) << b) {
                        s0 ^= s[0];
                        s1 ^= s[1];
                        s2 ^= s[2];
                        s3 ^= s[3];
                    }
                    (*this)();	
                }

            s[0] = s0;
            s[1] = s1;
            s[2] = s2;
            s[3] = s3;
        }

        void long_jump(void) {
            static const uint64_t LONG_JUMP[] = { 0x76e15d3efefdcbbf, 0xc5004e441c522fb3, 0x77710069854ee241, 0x39109bb02acbe635 };

            uint64_t s0 = 0;
            uint64_t s1 = 0;
            uint64_t s2 = 0;
            uint64_t s3 = 0;
            for(std::size_t i = 0; i < sizeof LONG_JUMP / sizeof *LONG_JUMP; i++)
                for(int b = 0; b < 64; b++) {
                    if (LONG_JUMP[i] & UINT64_C(1) << b) {
                        s0 ^= s[0];
                        s1 ^= s[1];
                        s2 ^= s[2];
                        s3 ^= s[3];
                    }
                    (*this)();	
                }

            s[0] = s0;
            s[1] = s1;
            s[2] = s2;
            s[3] = s3;
        }
};

#endif
