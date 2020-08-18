#include "test.h"
#include"../inc/index_mgr.hpp"
#include"../inc/record_mgr.hpp"
#include"../inc/index.hpp"
#include"../inc/files.hpp"
vector<pair<string, void(*)()>> test_cases;

using namespace std;

int main() {
	int cur = 1;
	int len = test_cases.size();
	for (auto& p : test_cases) {
		auto& test = p.first;
		cout << "[" << cur << "/" << len << "] " << test << " : ";
		void (*fp)() = p.second;
		try {
			fp();
			cout << "Passed" << endl;
		}
		catch (AssertionError & ex) {
			cout << "Assertion failed! " << ex.what() << endl;
		}
		catch (exception & ex) {
			cout << "Unhandled error! " << ex.what() << endl;
		}
		cur++;
	}
	system("pause");
	return 0;

}