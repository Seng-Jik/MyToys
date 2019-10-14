#include <iostream>
#include <fstream>
#include <algorithm>

using namespace std;

bool check(const char* name)
{
	for (int i = 0; i < 9; ++i)
	{
		const auto c = count(name, name + 9, name[i]);
		if (c > 3) return false;
	}
	return true;
}

template <typename TFunc>
void generate(TFunc&& func)
{
	char name[10] = { 0 };
#define F(t) for(name[t] = 'A';name[t] <= 'Z';++name[t])
	F(0)
	F(1)
	F(2)
	F(3)
	F(4)
	F(5)
	F(6)
	F(7)
	F(8)
	if(check(name)) func(name);
}



int main()
{
	ofstream o("GodsName.txt");
	generate([&o](const char* name)
	{
		o << name << endl;
		cout << name << endl;
	});
	o.close();
}
