#pragma once
#include <iostream>
#include <random>
#include <vector>

class RandomManager
{
public:

    static RandomManager& get() {
        static RandomManager instance;
        return instance;
    }

	bool probability(float val) {
		rd_probability.param(std::bernoulli_distribution::param_type(val));
		return rd_probability(random);
	}

	int weight(std::vector<int> list) {
		rd_weight.param(std::discrete_distribution<>::param_type(list.begin(), list.end()));
		return rd_weight(random);
	}

	int range(int x, int y) {
		rd_range.param(std::uniform_int_distribution<int>::param_type(x, y));
		return rd_range(random);
	}

private:
	std::random_device rd;
	std::mt19937 random = std::mt19937(rd());

	std::bernoulli_distribution rd_probability;
	std::uniform_int_distribution<int> rd_range;
	std::discrete_distribution<int> rd_weight;
};