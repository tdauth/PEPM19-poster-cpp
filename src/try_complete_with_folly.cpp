#include <iostream>

#include <folly/init/Init.h>
#include <folly/futures/Future.h>
#include <folly/executors/GlobalExecutor.h>

using namespace folly;
using namespace std;


template<typename T>
bool tryComplete(Promise<T> &p, Try<T> &&t)
{
	if (!t.hasValue() && !t.hasException())
	{
		return false;
	}

	try
	{
		p.setTry(move(t));

		return true;
	}
	catch (const PromiseAlreadySatisfied &e)
	{
		return false;
	}

	return true;
}

template<typename T>
void tryCompleteWith(Promise<T> &&p, Future<T> &&f)
{
	auto ctx = make_shared<Future<T>>(move(f));
	// TODO use thenTry?
	ctx->setCallback_([p = move(p), ctx] (Try<T> &&t) mutable
		{
			tryComplete(p, move(t));
		});
}

int main(int argc, char *argv[])
{
	init(&argc, &argv);
	auto x1 = futures::sleep(Duration(5)).thenValue([] (Unit)  { return 10; });
	auto p = Promise<int>();
	auto x2 = p.getFuture();

	tryCompleteWith(move(p), move(x1));

	auto r = move(x2).get();
	cerr << "Result: " << r << endl;

	return 0;
}