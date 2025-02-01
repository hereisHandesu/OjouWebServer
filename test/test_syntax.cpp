//#include <iostream>
//#include <list>
//
//class Damn
//{
//public:
//	Damn(int val_, int val2_) : val(val_), val2(val2_) {}
//public:
//	int val;
//	int val2;
//};
//
//int main()
//{
//	std::list<Damn> lst;
//	std::list<Damn*> lst2;
//	Damn thing = Damn(2, 4);
//	Damn* thing_p = &thing;
//	lst.push_back(thing);
//	//	如果用list<T>的方式，要使用元素会很麻烦，不如直接用list<T*>
//	//std::cout << (*(lst.front())).val << " " << thing.val << std::endl;
//	lst.pop_front();
//	//std::cout << lst.front() << std::endl;
//
//	lst2.push_back(thing_p);
//	std::cout << lst2.front() << " " << thing_p << std::endl;
//	lst2.pop_front();
//	std::cout << lst2.front() << std::endl;
//	return 0;
//}

#include <iostream>
#include <exception>
int main()
{
	int n = 0;
	scanf("%d", &n);
	if(n < 0)
	{
		throw "Damn";
	}
	else
	{
		printf("yes!");
	}
	return 0;
}
