
class unsafe_ptr final {
private:
    void *ptr_;
    
public:
    template <typename T>
    unsafe_ptr(T *p): ptr_{ p } { }
    unsafe_ptr(const unsafe_ptr&) = default;
    
    void *get() const { return ptr_; }
};

template <typename T>
T unsafe_cast(const unsafe_ptr& p) {
    return reinterpret_cast<T>(p.get());
}

int main() {
    int *p = nullptr;
    unsafe_ptr ptr = p;
    int *q = unsafe_cast<int*>(ptr);
    return 0;
}
