//
// the radix tree
// kuchynskiandrei@gmail.com
// 2022
//

#include <iostream>
#include <array>
#include <map>
#include <unordered_map>
#include <chrono>
#include <random>

#include "radix_storage.hpp"

constexpr auto map_capacity = 1'000'000;

using key_type = std::string;//int
using argument_type = int;
using test_type = std::array<std::pair<key_type, argument_type> , map_capacity>;

template<typename T>	
void test(test_type *tests_cases, test_type *wrong_tests_cases)
{
	T map;

	auto stat_time = std::chrono::high_resolution_clock::now();
	for (auto &test: *tests_cases) {
		map.insert(test);
	}
	auto duration_init = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - stat_time).count();

	stat_time = std::chrono::high_resolution_clock::now();
	for(auto i = 1; i; i--) {
		for (auto &test: *tests_cases) {
			const auto res = map.find(test.first);
			if (res == map.end())
				std::cout << "Error: " << test.first << " not found" << std::endl;
		}
	}
	auto duration_access = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - stat_time).count();

	stat_time = std::chrono::high_resolution_clock::now();
	for(auto i = 1; i; i--) {
		for (auto &test: *wrong_tests_cases) {
			const auto res = map.find(test.first);
			//if (res != map.end())
			//	std::cout << "Error: " << test.first << " is not wrong" << std::endl;
		}
	}
	auto duration_wrong_access = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - stat_time).count();

	stat_time = std::chrono::high_resolution_clock::now();
	for (auto &test: *tests_cases) {
		map.erase(test.first);
	}
	auto duration_erase = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - stat_time).count();
	
	std::cout <<	"insert time " << duration_init << 
					" ms, access time " << duration_access << 
					" ms, wrong access time " << duration_wrong_access << 
					" ms, erase time " << duration_erase << 
					" ms " << std::endl;
}

int main(int argc, char** argv)
{
	test_type *tests_cases = new test_type();
	test_type *wrong_tests_cases = new test_type();
	std::default_random_engine random_engine;
	std::uniform_int_distribution<int> RandNumber(10, 20);
	std::uniform_int_distribution<int> RandBigNumber(0, 1000000000);
	std::uniform_int_distribution<unsigned char> RandChar(1, 255);
	int value = 0;

		// Init
	for (auto tc: {tests_cases, wrong_tests_cases}) {
		for (auto &test: *tc) {
			const auto size = RandNumber(random_engine);
			std::string str;// = std::to_string(value++);
			for (auto i = size; i; i--) {
				const auto p = RandChar(random_engine);
				str += RandChar(random_engine);
			}

			// if key_type == std::string
			test = {str, RandBigNumber(random_engine)};
			// if key_type == int
			//test = {RandBigNumber(random_engine), RandBigNumber(random_engine)};
		}
	}

	// print test elements
	//for (auto &test: *tests_cases) {
	//	std::cout << test.first << " = " << test.second << std::endl;
	//}

	std::cout << "std::map\n";
	test<std::map<key_type, argument_type>>(tests_cases, wrong_tests_cases);

	std::cout << "std::unordered_map\n";
	test<std::unordered_map<key_type, argument_type>>(tests_cases, wrong_tests_cases);

	std::cout << "Radix tree\n";
			// if key_type == std::string
	test<radix_tree<argument_type>>(tests_cases, wrong_tests_cases);
	//test<AMemory<argument_type, 1>>(tests_cases, wrong_tests_cases);
	//test<AMemory<argument_type, 2>>(tests_cases, wrong_tests_cases);
	//test<AMemory<argument_type, 4>>(tests_cases, wrong_tests_cases);
	//test<AMemory<argument_type, 8>>(tests_cases, wrong_tests_cases);
			// if key_type == int
	//test<radix_tree_multi<key_type, argument_type>>(tests_cases, wrong_tests_cases);

	delete tests_cases;
	delete wrong_tests_cases;

	return 0;
}
