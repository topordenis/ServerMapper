#pragma once
namespace Utils {
	std::vector<unsigned char> OTPKey ( int ping );
}
#define throw_line(msg) \
    throw std::exception(msg " " __FILE__ ":" __LINE__)