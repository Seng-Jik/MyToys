#include <tuple>
#include <utility>
#include <functional>
#include <iostream>

using namespace std;

template <size_t I>
struct placeholder {};

template <typename TArg,typename TFromOutterArgsTuple>
struct select_arg
{
	decltype(auto) operator() (TArg arg, TFromOutterArgsTuple&)
	{
		return arg;
	}
};

template <size_t I,typename TFromOutterArgsTuple>
struct select_arg<placeholder<I>, TFromOutterArgsTuple>
{
	decltype(auto) operator() (placeholder<I>, TFromOutterArgsTuple& args)
	{
		return get<I>(args);
	}
};

template <typename TFunc,typename ... TArgs>
class binder final
{
private:
	using args = tuple<TArgs...>;
	using func = decay_t<TFunc>;

	func func_;
	args args_;
public:
	template <typename func,typename ... args>
	binder(func&& f,args&& ... a):
		func_{ std::forward<func>(f) },
		args_{ std::forward<args>(a)... }
	{}

private:
	template <typename TArgsTuple,int...se>
	decltype(auto) inv (func& f, args& a, TArgsTuple& t, index_sequence<se...> x)
	{
		return invoke(f, 
			select_arg<typename tuple_element<se,args>::type, TArgsTuple>{}(get<se>(a), t)...);
	}
	
public:

	template <typename ... TFromOutterArgs>
	decltype(auto) operator() (TFromOutterArgs&& ... outterArgs)
	{
		auto argTuple = make_tuple(std::forward<TFromOutterArgs>(outterArgs)...);
		auto seq = make_index_sequence<tuple_size<args>::value>();
		return inv(func_,args_,argTuple,seq);
	}
};

template <typename TFunc,typename ... TArgs>
auto mybind(TFunc&& f, TArgs&&...args)
{
	return binder<TFunc,TArgs...> { 
		std::forward<TFunc>(f),
		std::forward<TArgs>(args)... 
	};
}

int main()
{
	auto f = mybind(
		[](int a, float b, float c) {
			return a + b + c; 
		}, 
		placeholder<1>{}, 
		1.5f, 
		placeholder<0>{});

	const auto result = f(2.0f, 1);
	cout << result << endl;

	return 0;
}

