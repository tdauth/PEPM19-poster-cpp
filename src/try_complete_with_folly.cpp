#include <iostream>

#include <folly/init/Init.h>
#include <folly/futures/Future.h>
#include <folly/executors/GlobalExecutor.h>

using namespace folly;

template<typename T>
bool tryComplete(folly::Promise<T> &p, folly::Try<T> &&t)
{
	if (!t.hasValue() && !t.hasException())
	{
		return false;
	}

	try
	{
		p.setTry(std::move(t));

		return true;
	}
	catch (const folly::PromiseAlreadySatisfied &e)
	{
		return false;
	}

	return true;
}

template<typename T>
void tryCompleteWith(Promise<T> &&p, Future<T> &&f)
{
	auto ctx = std::make_shared<Future<T>>(std::move(f));
	// TODO use thenTry?
	ctx->setCallback_([p = std::move(p), ctx] (Try<T> &&t) mutable
		{
			tryComplete(p, std::move(t));
		});
}

int main() {
	auto x1 = futures::sleep(Duration(5)).thenValue([] (Unit)  { return 10; });
	auto p = Promise<int>();
	auto x2 = p.getFuture();

	tryCompleteWith(std::move(p), std::move(x1));

	std::cerr << "Result: " << std::move(x2).get() << std::endl;

	return 0;
}