
#ifndef GENETIC_ALGORITHM_ENGINE_HPP_
#define GENETIC_ALGORITHM_ENGINE_HPP_

#include "chromosome.hpp"

#include <vector>
#include <functional>
#include <algorithm>

namespace genetic_algorithm {

template <class T = chromosome>
class base_engine {
public:
	using chromosome_type = T;
	using container_type = std::vector<T>;
	
	using population_size_type = size_t;
	using generation_size_type = size_t;
	using mutation_rate_type = float;

	using evaluation_value_type = int;

	using initializer_type = std::function<void(chromosome_type &)>;
	using evaluator_type = std::function<evaluation_value_type(const chromosome_type &)>;
	using selector_type = std::function<void(container_type &)>;
	using crossover_type = std::function<chromosome_type(const chromosome_type &, const chromosome_type &)>;
	using mutator_type = std::function<void(chromosome_type &)>;

	using randomizer_type = std::function<float()>;

public:
	base_engine() :
		_chromosome_container(),
		_population_size(0),
		_num_generations(0),
		_mutation_rate(0.0f),
		_initializer(),
		_crossover(),
		_mutator(),
		_selector(),
		_randomizer()
	{}

	const container_type &chromosome_container() const { return _chromosome_container; }

	population_size_type population_size() const { return _population_size; }
	generation_size_type num_generations() const { return _num_generations; }
	mutation_rate_type mutation_rate() const { return _mutation_rate; }

	void set_population_size(population_size_type size) { _population_size = size; }
	void set_num_generations(generation_size_type size) { _num_generations = size; }
	void set_mutation_rate(mutation_rate_type rate) { _mutation_rate = rate; }

	void set_initializer(const initializer_type &fn) { _initializer = fn; }
	void set_evaluator(const evaluator_type &fn) { _evaluator = fn; }
	void set_selector(const selector_type &fn) { _selector = fn; }
	void set_crossover(const crossover_type &fn) { _crossover = fn; }
	void set_mutator(const mutator_type &fn) { _mutator = fn; }

	void set_randomizer(const randomizer_type &fn) { _randomizer = fn; }

public:
	void evolve() {
		auto &container = chromosome_container();

		// コンテナの初期化
		container.clear();
		container.resize(population_size());

		// 染色体に初期値設定
		for (auto &chromosome : container) {
			initialize(chromosome);
		}

		// 世代交代
		for (generation_size_type generation = 0; generation < num_generations(); ++generation) {
			// 次世代の選出
			select(container);
			container_type &next_generations = container;

			// 次世代の親の数
			auto parents = next_generations.size();

			// 次世代の子を作る
			{
				for (size_t i = 0; i < parents; ++i) {
					for (size_t j = i + 1; j < parents; ++j) {
						// 交叉
						next_generations.emplace_back(crossover(next_generations[i], next_generations[j]));

						// 突然変異
						if (randomize() >= mutation_rate()) {
							mutate(next_generations.back());
						}
					}
				}
			}

			// 調節
			if (next_generations.size() > population_size()) {
				next_generations.resize(population_size());
				next_generations.shrink_to_fit();

			} else if (next_generations.size() < population_size()) {
				auto size = next_generations.size();
				next_generations.resize(population_size());
				for (auto i = size; i < next_generations.size(); ++i) {
					initialize(next_generations[i]);
				}
			}

			// 次世代に差し替える
			container = next_generations;
		}

	}

protected:
	container_type &chromosome_container() { return const_cast<container_type &>(static_cast<const base_engine *>(this)->chromosome_container()); }

protected:
	// 初期化
	void initialize(chromosome_type &chromosome) {
		if (_initializer) {
			_initializer(chromosome);
		}
	}

	// 評価
	evaluation_value_type evaluate(const chromosome_type &chromosome) const {
		return _evaluator ? _evaluator(chromosome) : 0;
	}

	// 選択
	void select(container_type &container) const {
		if (_selector) {
			_selector(container);
		}
	}

	// 交叉
	chromosome_type crossover(const chromosome_type &lhs, const chromosome_type &rhs) const {
		return _crossover ? _crossover(lhs, rhs) : chromosome_type();
	}

	// 突然変異
	void mutate(chromosome_type &chromosome) const {
		if (_mutator) {
			_mutator(chromosome);
		}
	}

	// 乱数
	float randomize() const {
		return _randomizer ? _randomizer() : 0.0f;
	}

private:
	container_type _chromosome_container;

	population_size_type _population_size;
	generation_size_type _num_generations;
	mutation_rate_type _mutation_rate;

	initializer_type _initializer;
	evaluator_type _evaluator;
	crossover_type _crossover;
	mutator_type _mutator;
	selector_type _selector;

	randomizer_type _randomizer;
};

using engine = base_engine<>;

} // namespace genetic_algorithm

#endif // GENETIC_ALGORITHM_ENGINE_HPP_
