template<typename T> requires Poolable<T>
Pool<T>::Pool(u64 size) {
    data = cast(elem_type*, xalloc(sizeof(elem_type) * size));
    memset(data, 0, sizeof(data));

    data[0].prev = NULL;
    for(u64 i = 1; i < size; i++) {
        auto last = &data[i - 1];
        data[i].prev = last;
        data[i].value._pooled = &data[i];
    }
    free_list = &data[size - 1];
}

template<typename T> requires Poolable<T>
T* Pool<T>::alloc() {
    count++;
    auto e = free_list;
    assert(e); // TODO
    free_list = e->prev;
    e->prev = NULL;
    return &e->value;
}

template<typename T> requires Poolable<T>
void Pool<T>::free(T *p) {
    assert(p);
    count--;
    auto e = p->_pooled;
    e->prev = free_list;
    free_list = e;
}