#include "TapeEmulator.hpp"

std::chrono::milliseconds Tape::rw_delay{ 0LL };
std::chrono::milliseconds Tape::rewind_delay{ 0LL };
std::chrono::milliseconds Tape::move_delay{ 0LL };


int main(int argc, char* argv[]) {
	if (argc > 2) {
		size_t N, M;
		try {
			Tape::prepare(N, M);
			if (N == 0ULL) {
				throw std::invalid_argument("Empty tape.");
			}
			if (M < 64ULL) { throw std::exception("Not enough RAM."); };
			Tape itape(argv[1], N);
			Tape otape(argv[2], N);
			itape.read();
			if (N == 1ULL) {
				otape.put(itape.get());
			}
			else if ((M >> 2) > N && (M >> 2) - N >= 24ULL) {
				int* elems = new int[N];
				elems[0] = itape.get();
				size_t i;
				size_t ind;
				size_t start = N >> 1;
				{
					size_t child;
					std::chrono::milliseconds est_delay(Tape::move_delay);
					bool sup;
					for (i = 1ULL; i < start + 2ULL; ++i) {
						itape.forward();
						elems[i] = itape.get();
					}
					std::thread t([&i, &itape, elems, N]() {
						for (; i < N; ++i) {
							itape.forward();
							elems[i] = itape.get();
						}
						});
					--N;
					do {
						sup = true;
						ind = --start;
						while (i <= N - ind) {
							std::this_thread::sleep_for(est_delay);
						}
						while (sup && ind < (N >> 1)) {
							child = N - (ind << 1) - 2U;
							if (elems[child] < elems[child + 1U]) {
								++child;
							}
							if ((sup = elems[N - ind] < elems[child])) {
								std::swap(elems[N - ind], elems[child]);
								ind = N - child;
							}
						}
						if (sup && ind == ((N - 1U) >> 1) && elems[N - ind] < elems[0]) {
							std::swap(elems[N - ind], elems[0]);
						}
					} while (start > 0U);
					start = (N + 1U) >> 1;
					do {
						sup = true;
						ind = --start;
						while (sup && ind < (N >> 1)) {
							child = (ind << 1) + 2U;
							if (elems[child - 1U] < elems[child]) {
								--child;
							}
							if ((sup = elems[child] < elems[ind])) {
								std::swap(elems[child], elems[ind]);
								ind = child;
							}
						}
					} while (start > 0U);
				}

				for (start = 2U; start < N; ++start) {//insertion sort with binary search.
					if (elems[start] < elems[start - 1U]) {
						int tmp = elems[start];
						i = 0ULL;
						ind = start - 1ULL;
						while (ind - i > 1ULL) {
							if (elems[i + ((ind - i) >> 1)] <= tmp) {
								i += ((ind - i) >> 1);
							}
							else { ind = i + ((ind - i) >> 1); }
						}
						for (i = start; i > ind; --i) {
							elems[i] = elems[i - 1ULL];
						}
						elems[ind] = tmp;
					}
				}
				otape.put(elems[0]);
				for (size_t j = 1ULL; j <= N; ++j) {
					otape.forward();
					otape.put(elems[j]);
				}
			}
			else {
				char f1[] = "tmp/tape1.txt\0tmp/tape2.txt\0tmp/tape3.txt";
				Tape t1(f1, N);
				Tape t2(f1 + 14, N);
				size_t cnt = 0ULL;
				size_t i = 1ULL;
				int base = 1;
				int j = itape.get();
				if ((j & base) == 0) {
					t1.put(j);
					do {
						itape.forward();
						j = itape.get();
						if ((j & base) == 0) {
							t1.forward();
							t1.put(j);
						}
						else {
							t2.put(j);
							++cnt;
						}
						++i;
					} while (i < N && cnt == 0ULL);
				}
				else {
					t2.put(j);
					++cnt;
					do {
						itape.forward();
						j = itape.get();
						if ((j & base) == 0) {
							t1.put(j);
						}
						else {
							t2.forward();
							t2.put(j);
							++cnt;
						}
						++i;
					} while (i < N && cnt == i);
				}
				for (; i < N; ++i) {
					itape.forward();
					j = itape.get();
					if ((j & base) == 0) {
						t1.forward();
						t1.put(j);
					}
					else {
						t2.forward();
						t2.put(j);
						++cnt;
					}
				}
				itape.change(f1 + 28);
				Tape* from;
				Tape* a;
				Tape* b = &itape;
				if (cnt != N) {
					from = &t1;
					a = &t2;
					if (cnt > 0ULL) {
						t2.MTF();
						t1.forward();
						t1.put(t2.get());
						for (i = 1ULL; i < cnt; ++i) {
							t1.forward();
							t2.forward();
							t1.put(t2.get());
						}
					}
				}
				else {
					from = &t2;
					a = &t1;
				}
				for (base <<= 1; base > 0; base <<= 1) {
					cnt = 0ULL;
					i = 1ULL;
					from->forward();
					j = from->get();
					if ((j & base) == 0) {
						a->put(j);
						do {
							from->forward();
							j = from->get();
							if ((j & base) == 0) {
								a->forward();
								a->put(j);
							}
							else {
								b->put(j);
								++cnt;
							}
							++i;
						} while (i < N && cnt == 0ULL);
					}
					else {
						b->put(j);
						++cnt;
						do {
							from->forward();
							j = from->get();
							if ((j & base) == 0) {
								a->put(j);
							}
							else {
								b->forward();
								b->put(j);
								++cnt;
							}
							++i;
						} while (i < N && cnt == i);
					}
					for (; i < N; ++i) {
						from->forward();
						j = from->get();
						if ((j & base) == 0) {
							a->forward();
							a->put(j);
						}
						else {
							b->forward();
							b->put(j);
							++cnt;
						}
					}
					if (cnt != N) {
						if (cnt > 0ULL) {
							b->backward(cnt - 1ULL);
							a->forward();
							a->put(b->get());
							for (i = 1ULL; i < cnt; ++i) {
								a->forward();
								b->forward();
								a->put(b->get());
							}
						}
						std::swap(from, a);
					}
					else {
						std::swap(from, b);
					}
				}
				cnt = 0ULL;
				i = 1ULL;
				from->forward();
				if ((j = from->get()) < 0) {
					a->put(j);
					do {
						from->forward();
						if ((j = from->get()) < 0) {
							a->forward();
							a->put(j);
						}
						else {
							b->put(j);
							++cnt;
						}
						++i;
					} while (i < N && cnt == 0ULL);
				}
				else {
					b->put(j);
					++cnt;
					do {
						from->forward();
						if ((j = from->get()) < 0) {
							a->put(j);
						}
						else {
							b->forward();
							b->put(j);
							++cnt;
						}
						++i;
					} while (i < N && cnt == i);
				}
				for (; i < N; ++i) {
					from->forward();
					if ((j = from->get()) < 0) {
						a->forward();
						a->put(j);
					}
					else {
						b->forward();
						b->put(j);
						++cnt;
					}
				}
				if (cnt != N) {
					if (cnt > 0ULL) {
						b->backward(cnt - 1ULL);
						a->forward();
						a->put(b->get());
						for (i = 1ULL; i < cnt; ++i) {
							a->forward();
							b->forward();
							a->put(b->get());
						}
					}
					std::swap(from, a);
				}
				else {
					std::swap(from, b);
				}
				from->forward();
				otape.put(from->get());
				for (i = 1ULL; i < N; ++i) {
					from->forward();
					otape.forward();
					otape.put(from->get());
				}
			}
			otape.write();
		}
		catch (std::exception err) {
			std::cout << err.what() << std::endl;
		}
	}
	else {
		std::cout << "Please use the program as follows:" << std::endl << '\'' << argv[0] <<
			" fname1 fname2'" << std::endl << "Where 'fname1' is the name of the file that contains input tape, " <<
			"whereas 'fname2' is the name of the file that will contain elements of the output tape" << std::endl;
	}
	return 0;
}