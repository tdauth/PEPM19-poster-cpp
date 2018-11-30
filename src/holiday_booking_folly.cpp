#include <sstream>
#include <iostream>
#include <array>

#include <folly/init/Init.h>
#include <folly/futures/Future.h>
#include <folly/executors/GlobalExecutor.h>

using namespace folly;

const auto budgetEUR = 600;

struct HolidayLocation {
	double price;
	std::string name;
	std::string currency;
	std::string familyLetter;
	std::string friendsLetter;

	std::string getFamilyLetter() const {
		if (familyLetter.empty()) {
				return familyLetter;
		}
		return "Dear family, I am going to " + name + ".";
	}

	std::string getFriendsLetter() const {
		if (friendsLetter.empty()) {
			return friendsLetter;
		}

		std::stringstream sstream;
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

Future<HolidayLocationAndRating> currencyRating(HolidayLocation location) {
	if (location.currency == "CHF") {
		return makeFuture(HolidayLocationAndRating { .location = location, .rating = 1.13 });
	} else if (location.currency == "USD") {
		return makeFuture(HolidayLocationAndRating { .location = location, .rating = 1.14 });
	}

	return makeFuture<HolidayLocationAndRating>(std::runtime_error("Unknown currency!"));
}

bool budgetIsSufficient(HolidayLocationAndRating locationAndRating) {
	return locationAndRating.rating * budgetEUR >= locationAndRating.location.price;
}

HolidayLocation bookHoliday(std::pair<std::size_t, HolidayLocationAndRating> &&locationAndRating) {
	return locationAndRating.second.location;
}


HolidayLocation dontBookAnything() {
	return AtHome;
}

HolidayLocation letterToFamily(HolidayLocation location) {
	std::cerr << location.getFamilyLetter() << std::endl;
	return location;
}

HolidayLocation letterToFriends(HolidayLocation location) {
	std::cerr << location.getFriendsLetter() << std::endl;
	return location;
}

int main(int argc, char *argv[])
{
	folly::init(&argc, &argv);
	auto ex = getCPUExecutor().get();

	Future<HolidayLocationAndRating> x1 = holidayLocationSwitzerland().thenValue(currencyRating).filter(budgetIsSufficient);
	Future<HolidayLocationAndRating> x2 = holidayLocationUSA().thenValue(currencyRating).filter(budgetIsSufficient);
	std::array<Future<HolidayLocationAndRating>, 2> both{{ std::move(x1), std::move(x2) }};
	auto x3 = collectAnyWithoutException(std::move(both)).via(ex); // L1
	auto x4 = std::move(x3).thenValue(bookHoliday); // TODO Error Handling:.onError(dontBookAnything); // L2
	auto x5 = std::move(x4).thenValue(letterToFamily);
	auto x6 = std::move(x5).thenValue(letterToFriends); // L3

	x6.wait();

	return 0;
}
