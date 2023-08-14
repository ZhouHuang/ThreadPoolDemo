// Compiler: MSVC 19.29.30038.1
// C++ Standard: C++17
#include <iostream>
#include <thread>
#include <vector>
using namespace std;
void doit() { cout << "World!" << endl; }
int main() {
	thread a([]{
		cout << "Hello, " << flush;
	}), b(doit);
	a.join();
	b.join();

	vector<int> va{4};
	cout << va.size() << endl;
	return 0;
}
