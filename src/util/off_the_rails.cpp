namespace otr {
    template<typename T> struct type_identity { using type = T; };


    template<typename T> struct remove_reference { using type = T; };
    template<typename T> struct remove_reference<T&> { using type = T; };
    template<typename T> struct remove_reference<T&&> { using type = T; };


    template<typename T> struct remove_cv { using type = T; };
    template<typename T> struct remove_cv<T const> { using type = T; };
    template<typename T> struct remove_cv<T volatile> { using type = T; };
    template<typename T> struct remove_cv<T const volatile> { using type = T; };
    template<typename T> using remove_cv_t = typename remove_cv<T>::type;


    template<typename T> struct remove_const { using type = T; };
    template<typename T> struct remove_const<T const> { using type = T; };
    template<typename T> using remove_const_t = typename remove_const<T>::type;


    template<typename T> struct remove_volatile { using type = T; };
    template<typename T> struct remove_volatile<T volatile> { using type = T; };
    template<typename T> using remove_volatile_t = typename remove_volatile<T>::type;


	template<typename A, typename B>
	inline constexpr bool type_eq = false;

	template<typename A>
	inline constexpr bool type_eq<A, A> = true;

    template<typename T, typename U>
    concept same_as = type_eq<T, U> && type_eq<U, T>;


    template<typename T, T v>
    struct integral_constant {
        static constexpr T value = v;
        using value_type = T;
        using type = integral_constant;
        constexpr operator value_type() const noexcept { return value; }
        constexpr value_type operator()() const noexcept { return value; }
    };

    
    template<bool B>
    using bool_constant = integral_constant<bool, B>;

    using true_type = bool_constant<true>;
    using false_type = bool_constant<false>;


    template<typename T> struct is_pointer_helper : false_type {};
    template<typename T> struct is_pointer_helper<T*> : true_type {};
    template<typename T> struct is_pointer : is_pointer_helper<typename remove_cv<T>::type> {};
    template<typename T> inline constexpr bool is_pointer_v = is_pointer<T>::value;


    template<typename T> struct is_array : false_type {};
    template<typename T> struct is_array<T[]> : true_type {};
    template<typename T, u64 N> struct is_array<T[N]> : true_type {};
    template<typename T> inline constexpr bool is_array_v = is_array<T>::value;


    template<typename T> struct is_union : bool_constant<__is_union(T)> {};
    template<typename T> inline constexpr auto is_union_v = is_union<T>::value;


    template<typename T> struct is_noncv_floating_point : false_type {};
    template<> struct is_noncv_floating_point<float> : true_type {};
    template<> struct is_noncv_floating_point<double> : true_type {};
    template<> struct is_noncv_floating_point<long double> : true_type {};

    template<typename T> struct is_noncv_integral : false_type {};
    template<> struct is_noncv_integral<bool> : true_type {};
    template<> struct is_noncv_integral<char> : true_type {};
    template<> struct is_noncv_integral<signed char> : true_type {};
    template<> struct is_noncv_integral<unsigned char> : true_type {};
    template<> struct is_noncv_integral<wchar_t> : true_type {};
    template<> struct is_noncv_integral<char16_t> : true_type {};
    template<> struct is_noncv_integral<char32_t> : true_type {};
    template<> struct is_noncv_integral<short> : true_type {};
    template<> struct is_noncv_integral<unsigned short> : true_type {};
    template<> struct is_noncv_integral<int> : true_type {};
    template<> struct is_noncv_integral<unsigned int> : true_type {};
    template<> struct is_noncv_integral<long> : true_type {};
    template<> struct is_noncv_integral<unsigned long> : true_type {};
    template<> struct is_noncv_integral<long long> : true_type {};
    template<> struct is_noncv_integral<unsigned long long> : true_type {};
    
    
    template<typename T> struct is_integral : is_noncv_integral<remove_cv_t<T>> {};
    template<typename T> inline constexpr auto is_integral_v = is_integral<T>::value;


    template<typename T>
    integral_constant<bool, !is_union_v<T>> _is_class(int T::*);
    
    template<typename>
    false_type _is_class(...);
    
    template<typename T>
    struct is_class : decltype(_is_class<T>(nullptr)) {};


    template<typename B>
    true_type _test_pre_ptr_convertible(const volatile B*);
    template<typename>
    false_type _test_pre_ptr_convertible(const volatile void*);
    
    template<typename, typename>
    auto _test_pre_is_base_of(...) -> true_type;
    template<typename B, typename D>
    auto _test_pre_is_base_of(int) -> decltype(_test_pre_ptr_convertible<B>(static_cast<D*>(nullptr)));
    
    template<typename Base, typename Derived>
    struct is_base_of :
        bool_constant<
            is_class<Base>::value && 
            is_class<Derived>::value &&
            decltype(_test_pre_is_base_of<Base, Derived>(0))::value
        > { };

    template<typename Base, typename Derived>
    inline constexpr auto is_base_of_v = is_base_of<Base, Derived>::value;


    template<bool B, typename T, typename F> struct conditional { using type = T; };
    template<typename T, typename F> struct conditional<false, T, F> { using type = F; };
    template<bool B, typename T, typename F> using conditional_t = typename conditional<B, T, F>::type;
}