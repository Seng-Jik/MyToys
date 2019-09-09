#include <iostream>
#include <utility>
template <typename TResult,typename...TArgs>
class base_func_wrapper
{
public:
    virtual TResult operator() (TArgs...) = 0;
    virtual ~base_func_wrapper() {}
};

template <typename TResult,typename...TArgs>
class func_pointer_wrapper final : public base_func_wrapper<TResult,TArgs...>
{
private:
    using func_pointer = TResult (*) (TArgs...);
    func_pointer p_;
public:
    func_pointer_wrapper(func_pointer p):p_{p}{}
    TResult operator() (TArgs...args) override { return p_(args...); }
};

template <typename TFunc,typename TResult,typename...TArgs>
class function_object_wrapper final : public base_func_wrapper<TResult,TArgs...>
{
private:
    TFunc func_;
public:
    template <typename TFuncObj>
    function_object_wrapper(TFuncObj&& obj) : func_{ std::forward<TFuncObj>(obj) } {}
    TResult operator() (TArgs...args) override { return func_(args...); }
};

template <typename TResult,typename...TArgs>
class function;

template <typename TResult,typename...TArgs>
class function <TResult(TArgs...)> final
{
private:
    base_func_wrapper<TResult,TArgs...> * func_ = nullptr;
public:
    function(TResult(*p)(TArgs...)): func_{ new func_pointer_wrapper(p) }{}
    
    template <typename TFuncObj>
    function(TFuncObj&& obj): func_ { new function_object_wrapper<TFuncObj,TResult,TArgs...>(std::forward<TFuncObj>(obj)) } {}
    
    ~function() { delete func_; }
    function(const function&) = delete;
    function& operator= (const function&) = delete;
    
    TResult operator () (TArgs...args) { return (*func_)(args...); }
};

int hello(int a,int b)
{
    return a + b;
}

void hello2()
{
    std::cout<<"Hello";
}

int main()
{
    function<int(int,int)> f(hello);
    std::cout<<f(1,2)<<std::endl;
    function<void()> f2(hello2);
    f2();
    
    int i = 1,j = 2,k = 3;
    function<int()> f3 = [i,j,k]{
        return i + j + k;
    };
    std::cout<<f3()<<std::endl;
    
    return 0;
}
