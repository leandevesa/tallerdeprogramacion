#include "Globales.h"

std::string IntAString(int UnInt) {
	std::stringstream ss;
	ss << UnInt;
	std::string str = ss.str();

	return str;
}
