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
		sstream
		<< "Lets book "
		<< name
		<< " for "
		<< price
		<< " "
		<< currency
		<< ". Join me in my holidays!";
		return sstream.str();
	}

	string toString() const {
		stringstream sstream;
		sstream
		<< "HolidayLocation("
		<< price
		<< ','
		<< name
		<< ','
		<< currency
		<< ")";
		return sstream.str();
	}
};

ostream& operator<<(ostream &out, const HolidayLocation &location)
{
	return out << location.toString();
}

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

// Increase both prices to stay at home.
Future<HolidayLocation> holidayLocationSwitzerland() {
	return makeFuture(HolidayLocation {
		.price = 600.0, // Increase to choose the USA over Switzerland.
		.name = "Switzerland",
		.currency = "CHF"
	});
};

Future<HolidayLocation> holidayLocationUSA(){
	return makeFuture(HolidayLocation {
		.price = 600.0, // Increase to choose Switzerland over the USA.
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

HolidayLocation dontBookAnything(exception_wrapper&&) {
	return AtHome;
}

HolidayLocation letterToFamily(HolidayLocation &&location) {
	cout << "Send a letter to family: " << location.getFamilyLetter() << endl;
	return location;
}

HolidayLocation letterToFriends(HolidayLocation &&location) {
	cout << "Send a letter to friends: " << location.getFriendsLetter() << endl;
	return location;
}

/*
 * Issues:
 * I1: We have to move futures.
 * I2: We cannot give priority to Switzerland. There is no fallbackTo like in Scala FP.
 * I3: We have to convert the SemiFuture into a Future to use callbacks.
 * I4: We have to create a new future for every chained callback. We could use thenMulti for L3 but it would lead to the same code internally.
 * I5: We cannot call get multiple times.
 */
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
	decltype(x1) both[] = { move(x1), move(x2) }; // I1
	auto x3 = collectAnyWithoutException(begin(both), end(both)) // I2
		.via(ex); // I3
	auto x4 = move(x3).thenValue(bookHoliday).thenError(dontBookAnything);
	auto x5 = move(x4).thenValue(letterToFamily);
	auto x6 = move(x5).thenValue(letterToFriends); // I4
	auto r = move(x6).get(); // I5

	cout << r << endl;

	return 0;
}
