#include <stdio.h>
#include <type_traits>
#include <cstdint>
#include <functional>

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
constexpr bool narrowing_bitint(int bits1, bool sign1, int bits2, bool sign2) {
    if (sign1 && (!sign2)) return true;
    if (bits1-oneif(sign1) >= bits2-oneif(sign2)) return true;
    return false;
}

struct op_sum {
    static constexpr bool sign(int sign1, int sign2) { return sign1 || sign2; }
    static constexpr int  bits(int bits1, int bits2) { return maxbits(bits1,bits2) + 1; }
    template <class T> static T fun(const T& a, const T& b) { return a+b; }
};

struct op_mul {
    static constexpr bool sign(int sign1, int sign2) { return sign1 || sign2; }
    static constexpr int  bits(int bits1, int bits2) { return bits1 + bits2; }
    template <class T> static T fun(const T& a, const T& b) { return a*b; }
};

template <class T>
static constexpr int  sign_bits_wrap(int bits1, bool sign1, int bits2, bool sign2) {
    return T::bits(bits1-oneif(sign1),bits2-oneif(sign2)) + oneif(T::sign(sign1,sign2));
}

template <int BITS, bool SIGNED = true> class bitint;

template <class OP, int BITS1, bool SIGN1, int BITS2, bool SIGN2 >
struct bitint_op {
    typedef bitint< sign_bits_wrap<OP>(BITS1,SIGN1,BITS2,SIGN2), OP::sign(SIGN1,SIGN2) > type;
    static type fun (const bitint<BITS1, SIGN1>& i1, const bitint<BITS2, SIGN2>& i2) {
        typedef typename type::type rettype;
        rettype v1 = i1.value();
        rettype v2 = i2.value();
        rettype v3 = OP::template fun<rettype>(v1,v2);
        type ret(v3);
        return ret;
    }
};

template <int BITS, bool SIGNED> class bitint {
public:
    typedef typename getbitint<BITS, SIGNED>::type type;
private:
    type val;
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
        printf("%s bitint< %d >: %ld (%d bits)\n", sign ? "signed" : "unsigned", bits, (long int) val, typebits);
    }
    template <class OP, int BITS_, bool SIGNED_>
    using op = bitint_op< OP, BITS, SIGNED, BITS_, SIGNED_>;
    template <int BITS_, bool SIGNED_>
    typename op<op_sum,BITS_,SIGNED_>::type operator+ (const bitint<BITS_, SIGNED_>& other) const {
        return op<op_sum,BITS_,SIGNED_>::fun(*this, other);
    }
    template <int BITS_, bool SIGNED_>
    typename op<op_mul,BITS_,SIGNED_>::type operator* (const bitint<BITS_, SIGNED_>& other) const {
        return op<op_mul,BITS_,SIGNED_>::fun(*this, other);
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
