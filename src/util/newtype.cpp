template<typename T, typename Tag>
struct newtype {
    constexpr newtype() {}
    explicit constexpr newtype(T const& _value) : value(_value) {}
    explicit constexpr newtype(T&& _value) : value(_value) {}

    ~newtype() = default;
    constexpr newtype(newtype const&) = default;
    constexpr newtype(newtype&&) = default;
    constexpr newtype& operator=(newtype const&) = default;
    constexpr newtype& operator=(newtype&&) = default;

    //constexpr T& get()                      { return value; }
    //constexpr T const& get() const          { return value; }

    constexpr T& operator*()                { return value; }
    constexpr T const& operator*() const    { return value; }
    
    constexpr T* operator->()               { return &value; }
    constexpr T const* operator->() const   { return &value; }

    /*explicit*/ constexpr operator T() const   { return value; }

    newtype& operator++() {
        value++;
        return *this;
    }

    newtype operator++(int) {
        auto tmp = *this;
        operator++();
        return tmp;
    }

    newtype& operator--() {
        value--;
        return *this;
    }

    newtype operator--(int) {
        auto tmp = *this;
        operator--();
        return tmp;
    }

private:
    T value;
};


#define CMP(op)                                                                             \
    template<typename T, typename Tag>                                                      \
    constexpr bool operator op(newtype<T, Tag> const& lhs, newtype<T, Tag> const& rhs) {    \
        return *lhs op *lhs;                                                                \
    }
CMP(<)
CMP(<=)
CMP(>)
CMP(>=)
CMP(==)
CMP(!=)
#undef CMP