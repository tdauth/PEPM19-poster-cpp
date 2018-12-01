#include <iostream>

#include <folly/init/Init.h>
#include <folly/futures/Future.h>

using namespace folly;
using namespace std;
using namespace std::chrono;

int main(int argc, char *argv[])
{
	init(&argc, &argv);

	// thread 1
	auto p = Promise<int>();
	auto f = p.getFuture().thenValue([] (int v) { return "Got result: " + to_string(v); });

	// thread 2
	thread t2([p = move(p)] () mutable {
		this_thread::sleep_for(chrono::seconds(5));
		p.setValue(10);
	});

	auto r = move(f).get();
	cerr << "Result: " << r << endl;

	t2.join();

	return 0;
}