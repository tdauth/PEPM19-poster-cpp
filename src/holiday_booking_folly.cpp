#include <sstream>
#include <iostream>
#include <array>

#include <folly/init/Init.h>
#include <folly/futures/Future.h>
#include <folly/executors/GlobalExecutor.h>

using namespace folly;
using namespace std;

const auto budgetEUR = 600;

struct HolidayLocation {
	double price;
	string name;
	string currency;
	string familyLetter;
	string friendsLetter;

	string getFamilyLetter() const {
		if (!familyLetter.empty()) {
				return familyLetter;
		}
		return "Dear family, I am going to " + name + ".";
	}

	string getFriendsLetter() const {
		if (!friendsLetter.empty()) {
			return friendsLetter;
		}

		stringstream sstream;
		sstream <<"Lets book ";
		sstream << name;
		sstream << " for ";
		sstream << price
		<< " " << currency << ". Join me in my holidays!";
		return sstream.str();
	}
};

const auto AtHome = HolidayLocation {
	.price = 0.0,
	.name = "at home",
	.currency = "EUR",
	.familyLetter = "Dear family, please send me more money.",
	.friendsLetter = "Don't book anything, I am staying at home."
};

struct HolidayLocationAndRating {
	HolidayLocation location;
	double rating;
};

Future<HolidayLocation> holidayLocationSwitzerland() {
	return makeFuture(HolidayLocation {
		.price = 600.0,
		.name = "Switzerland",
		.currency = "CHF"
	});
};

Future<HolidayLocation> holidayLocationUSA(){
	return makeFuture(HolidayLocation {
		.price = 600.0,
		.name = "the USA",
		.currency = "USD"
	});
};

Future<HolidayLocationAndRating> currencyRating(HolidayLocation &&location) {
	if (location.currency == "CHF") {
		return makeFuture(HolidayLocationAndRating { .location = location, .rating = 1.13 });
	} else if (location.currency == "USD") {
		return makeFuture(HolidayLocationAndRating { .location = location, .rating = 1.14 });
	}

	return makeFuture<HolidayLocationAndRating>(runtime_error("Unknown currency!"));
}

bool budgetIsSufficient(const HolidayLocationAndRating &locationAndRating) {
	return locationAndRating.rating * budgetEUR >= locationAndRating.location.price;
}

HolidayLocation bookHoliday(pair<size_t, HolidayLocationAndRating> &&locationAndRating) {
	return locationAndRating.second.location;
}

HolidayLocation dontBookAnything() {
	return AtHome;
}

HolidayLocation letterToFamily(HolidayLocation &&location) {
	cerr << "Send letter to family: " << location.getFamilyLetter() << endl;
	return location;
}

HolidayLocation letterToFriends(HolidayLocation &&location) {
	cerr << "Send letter to friends: " << location.getFriendsLetter() << endl;
	return location;
}

int main(int argc, char *argv[])
{
	init(&argc, &argv);
	auto ex = getCPUExecutor().get();

	auto x1 = holidayLocationSwitzerland()
		.thenValue(currencyRating)
		.filter(budgetIsSufficient);
	auto x2 = holidayLocationUSA()
		.thenValue(currencyRating)
		.filter(budgetIsSufficient);
	auto x3 = collectAnyWithoutException(array<Future<HolidayLocationAndRating>, 2>{{ move(x1), move(x2) }}).via(ex); // L1
	auto x4 = move(x3).thenValue(bookHoliday); // TODO Error Handling:.onError(dontBookAnything); // L2
	// TODO Move the results, so we can see the text.
	auto x5 = move(x4).thenValue(letterToFamily);
	auto x6 = move(x5).thenValue(letterToFriends); // L3

	x6.wait();

	return 0;
}
