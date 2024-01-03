template<typename T>
struct Pool_Elem {
    Pool_Elem *prev;
    T value;
};

template<typename T>
struct Pooled {
    Pool_Elem<T> *_pooled;
};

template<typename T>
concept Poolable = requires(T *p) {
    requires otr::is_base_of_v<Pooled<T>, T>;
    { p->_pooled } -> otr::same_as<Pool_Elem<T>*&>;
};

template<typename T> requires Poolable<T>
struct Pool {
    using value_type = T;
    using elem_type = Pool_Elem<T>;

    Pool(u64 size);
    T* alloc();
    void free(T *p);
    u64 allocated() { return count; }

private:
    u64 size;
    Pool_Elem<T> *data;
    Pool_Elem<T> *free_list;
    u64 count;

    void expand();
};