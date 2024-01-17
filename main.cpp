#include <stdio.h>
#include <type_traits>
#include <cstdint>

template <int BITS, bool SIGNED> struct getbitint {
    static_assert(BITS <= 64);
    typedef typename getbitint<BITS-1, SIGNED>::type type;
};
template <> struct getbitint<  0, false > { typedef unsigned char type; };
template <> struct getbitint<  9, false > { typedef unsigned short int type; };
template <> struct getbitint< 17, false > { typedef unsigned int type; };
template <> struct getbitint< 33, false > { typedef unsigned long int type; };
template <> struct getbitint<  0, true > { typedef signed char type; };
template <> struct getbitint<  9, true > { typedef signed short int type; };
template <> struct getbitint< 17, true > { typedef signed int type; };
template <> struct getbitint< 33, true > { typedef signed long int type; };

constexpr int  oneif(bool sign) { if (sign) return 1; else return 0; }
constexpr int  maxbits(int bits1, int bits2) { if (bits1 < bits2) return bits2; else return bits1; }
constexpr bool sumsign(int sign1, int sign2) { return sign1 || sign2; }
constexpr int  sumbits(int bits1, bool sign1, int bits2, bool sign2) {
    return maxbits(bits1-oneif(sign1),bits2-oneif(sign2)) + 1 + oneif(sumsign(sign1,sign2));
}
constexpr bool mulsign(int sign1, int sign2) { return sign1 || sign2; }
constexpr int  mulbits(int bits1, bool sign1, int bits2, bool sign2) {
    return (bits1-oneif(sign1)) + (bits2-oneif(sign2)) + oneif(sumsign(sign1,sign2));
}
constexpr bool narrowing_bitint(int bits1, bool sign1, int bits2, bool sign2) {
    if (sign1 && (!sign2)) return true;
    if (bits1-oneif(sign1) >= bits2-oneif(sign2)) return true;
    return false;
}

template <int BITS, bool SIGNED = true> class bitint {
public:
    typedef typename getbitint<BITS, SIGNED>::type type;
private:
    type val;
    template <int BITS_, bool SIGNED_>
    struct sum {
        typedef bitint< sumbits(BITS,SIGNED,BITS_,SIGNED_), sumsign(SIGNED,SIGNED_) > type;
    };
    template <int BITS_, bool SIGNED_>
    struct mul {
        typedef bitint< mulbits(BITS,SIGNED,BITS_,SIGNED_), mulsign(SIGNED,SIGNED_) > type;
    };
public:
    static const int bits = BITS;
    static const int typebits = 8*sizeof(type);
    static const bool sign = SIGNED;
    bitint(type val_) : val(val_) {};
    template <int BITS_, bool SIGNED_>
    bitint(const bitint<BITS_, SIGNED_>& other) : val(other.value()) {
        static_assert(!narrowing_bitint(BITS_,SIGNED_,BITS,SIGNED), "Narrowing bitint");
    };
    type value() const {
        return val;
    }
    void print() const {
        printf("bitint< %d (%d), %s >(%ld)\n", bits, typebits, sign ? "signed" : "unsigned", (long int) val);
    }
    template <int BITS_, bool SIGNED_>
    typename sum<BITS_,SIGNED_>::type operator+ (const bitint<BITS_, SIGNED_>& other) const {
        typedef typename sum<BITS_,SIGNED_>::type rettype;
        typedef typename rettype::type rettypetype;
        rettypetype v1 = value();
        rettypetype v2 = other.value();
        rettypetype v3 = v1 + v2;
        rettype ret(v3);
        return ret;
    }
    template <int BITS_, bool SIGNED_>
    typename mul<BITS_,SIGNED_>::type operator* (const bitint<BITS_, SIGNED_>& other) const {
        typedef typename mul<BITS_,SIGNED_>::type rettype;
        typedef typename rettype::type rettypetype;
        rettypetype v1 = value();
        rettypetype v2 = other.value();
        rettypetype v3 = v1 * v2;
        rettype ret(v3);
        return ret;
    }
};

constexpr bool constsign(signed long int val) { return val < 0; }
constexpr int  constbits(signed long int val) {
    int bits = oneif(constsign(val));
    if (val < 0) val = -val;
    while(val) {val = val/2; bits++;}
    return bits;
}

static_assert(std::is_same<bitint< 8>::type,  int8_t>::value);
static_assert(std::is_same<bitint<16>::type, int16_t>::value);
static_assert(std::is_same<bitint<32>::type, int32_t>::value);
static_assert(std::is_same<bitint<64>::type, int64_t>::value);
static_assert(std::is_same<bitint< 8, false>::type, uint8_t>::value);
static_assert(std::is_same<bitint<16, false>::type, uint16_t>::value);
static_assert(std::is_same<bitint<32, false>::type, uint32_t>::value);
static_assert(std::is_same<bitint<64, false>::type, uint64_t>::value);

template < signed long int val >
bitint< constbits(val),constsign(val) > bitint_const () {
    typedef bitint< constbits(val),constsign(val) > rettype;
    typedef typename rettype::type rettypetype;
    rettypetype v = val;
    rettype ret(v);
    return ret;
};

int main() {
    bitint<16,false> x(12), y(11), z(10), nx(50), ny(50), nz(50);
    auto d = bitint_const<4>();
    auto nd = bitint_const<10>();
    auto ret = x + nx*(y + ny*(z + nz*d));
    x.print();
    d.print();
    ret.print();
    size_t v = ret.value();
    bitint<64> g = ret;
    g.print();
    printf("val: %lu\n",v);
}
