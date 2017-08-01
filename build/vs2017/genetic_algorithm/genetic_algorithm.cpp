
#include "genetic_algorithm/genetic_algorithm.hpp"

#include <iostream>
#include <random>
#include <numeric>
#include <algorithm>

namespace ga = genetic_algorithm;

namespace {

constexpr int chromosome_size = 8;

using engine = ga::engine;
using chromosome_type = engine::chromosome_type;
using container_type = engine::container_type;

void print_chromosome(const chromosome_type &chromosome) {
	std::cout << "  [ ";
	bool first = true;
	for (const auto &gene : chromosome.gene_container()) {
		if (!first) std::cout << ", ";
		std::cout << gene;
		first = false;
	}
	std::cout << " ]" << std::endl;
}

void print_chromosomes(const container_type &chromosomes) {
	std::cout << "{" << std::endl;
	for (const auto &chromosome : chromosomes) {
		print_chromosome(chromosome);
	}
	std::cout << "}" << std::endl;
}

} // namespace

int main()
{
	// 乱数器
	std::random_device random_device;
	std::mt19937 mt(random_device());

	// ランダマイザ
	std::uniform_real_distribution<float> randomizer(0.0f, 1.0f);

	// 初期化用
	std::uniform_int_distribution<int> distribution(0, 100);

	// ＧＡエンジン
	::engine engine;

	// パラメータ
	engine.set_population_size(100);
	engine.set_num_generations(100);
	engine.set_mutation_rate(0.01f);

	// ランダマイザの設定
	engine.set_randomizer(
		[&]() {
			return randomizer(mt);
		}
	);

	// 初期化の設定
	engine.set_initializer(
		[&](auto &chromosome) {
			chromosome.gene_container().resize(::chromosome_size);
			std::generate(
				chromosome.gene_container().begin(),
				chromosome.gene_container().end(),
				[&]() { return distribution(mt); }
			);
		}
	);

	// 評価関数の設定
	auto evaluator = [&](const auto &chromosome) {
		return std::accumulate(chromosome.gene_container().begin(), chromosome.gene_container().end(), 0);
	};

	engine.set_evaluator(evaluator);

	// 選出関数の設定
	engine.set_selector(
		[&](auto &container) {
			std::sort(
				container.begin(),
				container.end(),
				[&](const auto &a, const auto &b) {
					return evaluator(a) > evaluator(b);
				}
			);
			if (container.size() > 10) {
				container.resize(10);
			}
		}
	);

	// 交叉関数の設定
	engine.set_crossover(
		[&](const auto &a, const auto &b) {
			::chromosome_type chromosome;
			auto size = std::min(a.size(), b.size());
			bool target = false;
			for (decltype(size) i = 0; i < size; ++i) {
				chromosome.gene_container().push_back((target ? a : b)[i]);
				target = !target;
			}
			return chromosome;
		}
	);

	// 突然変異の設定
	engine.set_mutator(
		[&](auto &chromosome) {
			std::uniform_int_distribution<int> index_selector(0, chromosome.size() - 1);
			chromosome[index_selector(mt)] = distribution(mt);
		}
	);

	// 実行
	engine.evolve();

	// ゲノムの出力
	::print_chromosomes(static_cast<const ::engine &>(engine).chromosome_container());

#if _DEBUG
	system("pause");
#endif

    return 0;
}

