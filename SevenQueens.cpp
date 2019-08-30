#include <iostream>
#include <array>

using namespace std;

template <int N>
struct Factorial
{
	constexpr static int Value = N * Factorial<N - 1>::Value;
};

template <>
struct Factorial<0>
{
	constexpr static int Value = 1;
};

template <
	size_t Index,
	size_t N>
struct RemoveElementAt
{
private:
	template <
		size_t... Fis,
		size_t... Bis>

	constexpr array<int, N - 1> RemoveElementImpl(
		array<int, N> arr,
		index_sequence<Fis...>,
		index_sequence<Bis...>)
	{
		return array<int, N - 1>
		{
			get<Fis>(arr)...,
			get<Index + 1 + Bis>(arr)...
		};
	}

public:
	constexpr array<int,N-1> operator()(array<int, N> arr)
	{
		return RemoveElementImpl(
				arr,
				make_index_sequence<Index>{},
				make_index_sequence<N - Index - 1>{});
	}
};

template <
	int N,
	typename T,
	size_t... is>
constexpr array<T, N> PushFrontArray(T a, array<T, N - 1> arr, index_sequence<is...>)
{
	return array<T, N>
	{
		a,
		get<is>(arr)...
	};
}

template <
	int N,
	int M,
	size_t... Fis,
	size_t... Bis,
	typename T>
constexpr array<T, N + M> UnionArray(
	array<T, N> a1,
	array<T, M> a2,
	index_sequence<Fis...>,
	index_sequence<Bis...>)
{
	return array<T, N + M>
	{
		get<Fis>(a1)...,
		get<Bis>(a2)...
	};
}

template <
	int Index,
	int N>
constexpr array<int, N> InvCantor(array<int, N> indics)
{

	constexpr auto curIndex = Index / Factorial<N - 1>::Value;
	const auto curNumber = get<curIndex>(indics);

	if constexpr(N - 1 > 0)
	{
		const auto newIndics = RemoveElementAt<curIndex, indics.size()>{}(indics);
		const auto child = InvCantor<Index % Factorial<N - 1>::Value, N - 1>(newIndics);

		return PushFrontArray<N>(curNumber, child, make_index_sequence<child.size()>{});
	}
	else
	{
		return array<int, 1>{curNumber };
	}
}

template <int N,int... is>
constexpr array<int, N> BuildMainIndicis(integer_sequence<int, is...>)
{
	return array<int, N>
	{
		is...
	};
}


constexpr int Abs(int N)
{
	return N < 0 ? -N : N;
}

template <int N>
constexpr bool Check(array<int, N> queens)
{
	for (int i = 0; i < N; ++i)
	{
		for (int j = 0; j < N; ++j)
		{
			if (i != j)
			{
				const int p = queens[i] - queens[j];
				const int q = i - j;
				if (Abs(p) == Abs(q))
				{
					return false;
				}
			}
		}
	}
	return true;
}

constexpr int QueenCount = 7;

template <int C>
constexpr auto BuildLine(int index)
{
	const array<char,C-1> child = BuildLine<C - 1>(index);

	if(index == C - 1)
		return PushFrontArray<C, char>('*', child, make_index_sequence<C - 1>{});
	else
		return PushFrontArray<C, char>('0', child, make_index_sequence<C - 1>{});
}

template <>
constexpr auto BuildLine<0>(int)
{
	return array<char, 0>{};
}

template <int C,int L>
constexpr auto BuildGraph(array<int, C> arr)
{
	if constexpr(L >= 1)
	{
		const auto line = BuildLine<C>(get<L - 1>(arr));
		const auto endLined = PushFrontArray<C + 1, char>('\n', line, make_index_sequence<C>{});
		const auto child = BuildGraph<C, L - 1>(arr);
		return UnionArray<endLined.size(),child.size()>(
			endLined,
			child,
			make_index_sequence<endLined.size()>{},
			make_index_sequence<child.size()>{});
	}
	else
	{
		return array<char, 0>{};
	}
}

template <int N>
void PrintGraph(array<char, N> arr)
{
	for (int i = 0; i < N; ++i)
		cout << arr[i];
	cin.get();
}

template <int Top,int Bottom>
constexpr void MainFor()
{
	if constexpr (Top >= Bottom)
	{
		constexpr array<int, QueenCount> t =
			BuildMainIndicis<QueenCount>(make_integer_sequence<int, QueenCount>{});
		constexpr auto current = InvCantor<Top, t.size()>(t);

		constexpr bool check = Check(current);

		if constexpr(check)
		{
			constexpr auto graph = BuildGraph<QueenCount,QueenCount>(current);
			PrintGraph<graph.size()>(graph);
		}

		MainFor<Top - 1,Bottom>();
	}
}

template <int A,int B,int Layer>
constexpr void MainForSplited()
{
	constexpr int space = A - B;
	constexpr int layerA = space / 2;
	constexpr int layerB = space - layerA;

	constexpr int spaceAB = B;
	constexpr int spaceAA = B + layerB;
	constexpr int spaceBB = spaceAA + 1;
	constexpr int spaceBA = spaceBB + layerA - 1;

	if constexpr(Layer == 0)
	{
		MainFor<spaceAA, spaceAB>();
		MainFor<spaceBA, spaceBB>();
	}
	else 
	{
		MainForSplited<spaceAA, spaceAB, Layer - 1>();
		MainForSplited<spaceBA, spaceBB, Layer - 1>();
	}
}

int main()
{
	constexpr auto all = Factorial<QueenCount>::Value - 1;

	MainForSplited<all, 0, 1>();

	return 0;
}
