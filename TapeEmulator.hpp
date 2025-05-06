#ifndef _TAPE_EMULATOR_
#define _TAPE_EMULATOR_
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <stdexcept>


class Tape {
	char* filename;
	int* elements;
	size_t N;
	size_t head;
public:
	static std::chrono::milliseconds rw_delay;
	static std::chrono::milliseconds rewind_delay;
	static std::chrono::milliseconds move_delay;
	Tape(char* fname, size_t n) :filename(fname), N(n), head(0ULL) {
		if (N > 0ULL) {
			elements = new int[N];
		}
		else {
			elements = nullptr;
			throw std::invalid_argument("Trying to create zero-sized tape.");
		}
	}
	void read() {
		std::ifstream inFile(filename);
		if (inFile.is_open()) {
			size_t i = 0ULL;
			for (; i < N && inFile.good(); ++i) {
				inFile >> elements[i];
			}
			inFile.close();
			if (i < N) { throw std::length_error("The file does not contain enough elements."); }
		}
		else { throw std::exception("Could not open file to read."); }
	}
	void write() {
		std::ofstream outFile(filename);
		if (outFile.is_open()) {
			for (size_t i = 0ULL; i < N; ++i) {
				outFile << elements[i];
			}
			outFile.close();
		}
		else { throw std::exception("Could not open file to write."); }
	}
	int get()const {
		std::this_thread::sleep_for(rw_delay);
		return elements[head];
	}
	void put(int x) {
		elements[head] = x;
		std::this_thread::sleep_for(rw_delay);
	}
	void forward(size_t n = 1ULL) {
		if ((n %= N) != 0ULL) {
			head = (N - head <= n) ? n - (N - head) : head + n;
			if (n == 1ULL) {
				std::this_thread::sleep_for(move_delay);
			}
			else {
				std::this_thread::sleep_for(rewind_delay);
			}
		}
	}
	void backward(size_t n = 1ULL) {
		if ((n %= N) != 0ULL) {
			head = (head < n) ? head + (N - n) : head - n;
			if (n == 1ULL) {
				std::this_thread::sleep_for(move_delay);
			}
			else {
				std::this_thread::sleep_for(rewind_delay);
			}
		}
	}
	void change(char* fname) {
		filename = fname;
		head = 0ULL;
	}
	void MTF() { backward(head); }
	~Tape() {
		if (elements != nullptr) {
			delete[] elements;
		}
	}
	static void prepare(size_t& N, size_t& M) {
		std::ifstream conf("conf.txt");
		if (conf.is_open()) {
			long long delay1, delay2, delay3;
			conf >> N >> M >> delay1 >> delay2 >> delay3;
			conf.close();
			if (!conf.fail()) {
				rw_delay = std::chrono::milliseconds{ delay1 };
				rewind_delay = std::chrono::milliseconds{ delay2 };
				move_delay = std::chrono::milliseconds{ delay3 };
			}
			else {
				throw std::exception("Could not get parameters from \"conf.txt\".");
			}
		}
		else {
			throw std::exception("Could not open \"conf.txt\".");
		}
	}
};

#endif