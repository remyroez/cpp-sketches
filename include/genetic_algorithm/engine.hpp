
#ifndef GENETIC_ALGORITHM_ENGINE_HPP_
#define GENETIC_ALGORITHM_ENGINE_HPP_

#include "chromosome.hpp"

#include <memory>
#include <vector>
#include <functional>
#include <algorithm>

namespace genetic_algorithm {

template <class T = chromosome>
class base_engine {
public:
	using chromosome_type = T;
	using chromosome_pointer = std::shared_ptr<chromosome_type>;

	using container_type = std::vector<chromosome_pointer>;
	
	using population_size_type = size_t;
	using generation_size_type = size_t;
	using crossover_rate_type = float;
	using mutation_rate_type = float;

	using evaluation_value_type = float;

	using initializer_type = std::function<chromosome_pointer()>;
	using evaluator_type = std::function<evaluation_value_type(const chromosome_pointer &)>;
	using selector_type = std::function<void(container_type &)>;
	using crossover_type = std::function<void(const chromosome_pointer &, const chromosome_pointer &, chromosome_pointer &, chromosome_pointer &)>;
	using mutator_type = std::function<void(const chromosome_pointer &)>;

	using randomizer_type = std::function<float()>;

public:
	base_engine() :
		_chromosome_container(),
		_population_size(0),
		_crossover_rate(1.0f),
		_mutation_rate(0.0f),
		_initializer(),
		_crossover(),
		_mutator(),
		_selector(),
		_randomizer()
	{}

	const container_type &chromosome_container() const { return _chromosome_container; }

	population_size_type population_size() const { return _population_size; }
	crossover_rate_type crossover_rate() const { return _crossover_rate; }
	mutation_rate_type mutation_rate() const { return _mutation_rate; }

	void set_population_size(population_size_type size) { _population_size = size; }
	void set_crossover_rate(crossover_rate_type rate) { _crossover_rate = rate; }
	void set_mutation_rate(mutation_rate_type rate) { _mutation_rate = rate; }

	void set_initializer(const initializer_type &fn) { _initializer = fn; }
	void set_evaluator(const evaluator_type &fn) { _evaluator = fn; }
	void set_selector(const selector_type &fn) { _selector = fn; }
	void set_crossover(const crossover_type &fn) { _crossover = fn; }
	void set_mutator(const mutator_type &fn) { _mutator = fn; }

	void set_randomizer(const randomizer_type &fn) { _randomizer = fn; }

public:
	void reset() {
		auto &container = chromosome_container();

		// コンテナの初期化
		container.clear();
		container.resize(population_size());

		// 染色体に初期値設定
		std::generate(container.begin(), container.end(), _initializer);

		// 評価
		for (auto chromosome : container) {
			chromosome->set_fitness(evaluate(chromosome));
		}
	}

	void step() {
		auto &container = chromosome_container();

		// 親の選出
		select(container);

		// 親リスト
		auto parents = container;

		// 世代のリセット
		container.clear();

		// 次世代の子を作る
		for (size_t i = 0; i < parents.size(); i += 2) {
			if ((i + 1) >= parents.size()) break;
			if (container.size() >= population_size()) break;

			// 生成
			if (randomize() < crossover_rate()) {
				// 交叉
				chromosome_pointer a, b;
				crossover(parents[i], parents[i + 1], a, b);

				// 登録
				container.emplace_back(a);
				container.emplace_back(b);

				// 突然変異
				if (randomize() < mutation_rate()) mutate(a);
				if (randomize() < mutation_rate()) mutate(b);

				// 評価
				evaluate_chromosome(a);
				evaluate_chromosome(b);

			} else {
				// 親
				container.emplace_back(parents[i]);
				container.emplace_back(parents[i + 1]);
			}
		}

		// 調節
		if (container.size() > population_size()) {
			// 多すぎたら丸める
			container.resize(population_size());
			container.shrink_to_fit();

		} else if (container.size() < population_size()) {
			// 少なすぎたら埋める
			auto size = container.size();
			container.resize(population_size());
			std::generate(
				container.begin() + size,
				container.end(),
				[this]() {
					auto child = this->initialize();
					this->evaluate_chromosome(child);
					return child;
				}
			);
		}
	}

	void evolve(generation_size_type generation = 0) {
		reset();

		for (generation_size_type i = 0; i < generation; ++i) {
			step();
		}
	}

protected:
	container_type &chromosome_container() { return const_cast<container_type &>(static_cast<const base_engine *>(this)->chromosome_container()); }

	void evaluate_chromosome(chromosome_pointer &chromosome) {
		chromosome->set_fitness(evaluate(chromosome));
	}

protected:
	// 初期化
	chromosome_pointer initialize() const {
		return _initializer ? _initializer() : std::make_shared<chromosome_type>();
	}

	// 評価
	evaluation_value_type evaluate(const chromosome_pointer &chromosome) const {
		return _evaluator ? _evaluator(chromosome) : 0;
	}

	// 選択
	void select(container_type &container) const {
		if (_selector) {
			_selector(container);
		}
	}

	// 交叉
	void crossover(const chromosome_pointer &a, const chromosome_pointer &b, chromosome_pointer &na, chromosome_pointer &nb) const {
		if (_crossover) {
			_crossover(a, b, na, nb);

		} else {
			na = a;
			nb = b;
		}
	}

	// 突然変異
	void mutate(const chromosome_pointer &chromosome) const {
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
	crossover_rate_type _crossover_rate;
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
