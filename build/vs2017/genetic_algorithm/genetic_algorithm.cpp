
#include "genetic_algorithm/genetic_algorithm.hpp"

#include <iostream>
#include <random>
#include <numeric>
#include <algorithm>
#include <functional>

namespace ga = genetic_algorithm;

namespace {

constexpr int chromosome_size = 8;

using engine = ga::engine;
using chromosome_type = engine::chromosome_type;
using chromosome_pointer = engine::chromosome_pointer;
using container_type = engine::container_type;

void print_chromosome(const chromosome_pointer &chromosome) {
	std::cout << "  [ ";
	bool first = true;
	for (const auto &gene : chromosome->gene_container()) {
		if (!first) std::cout << ", ";
		std::cout << gene;
		first = false;
	}
	std::cout << " ] = " << chromosome->fitness() << std::endl;
}

void print_chromosomes(const container_type &chromosomes) {
	container_type container = chromosomes;
	std::sort(
		container.begin(),
		container.end(),
		[](const auto &a, const auto &b) {
			return a->fitness() > b->fitness();
		}
	);

	std::cout << "{" << std::endl;
	for (const auto &chromosome : container) {
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

	// ＧＡエンジン
	::engine engine;

	// パラメータ
	engine.set_population_size(100);
	engine.set_crossover_rate(0.60f);
	engine.set_mutation_rate(0.05f);

	// ランダマイザの設定
	engine.set_randomizer(std::bind(std::uniform_real_distribution<float>(0.0f, 1.0f), std::ref(mt)));

	// 初期化用
	auto random_initializer = std::bind(std::uniform_int_distribution<int>(0, 100), std::ref(mt));

	// 初期化の設定
	engine.set_initializer(
		[&]() {
			return std::make_shared<::chromosome_type>(::chromosome_size, random_initializer);
		}
	);

	// 評価関数の設定
	engine.set_evaluator(
		[&](const auto &chromosome) {
			return std::accumulate(chromosome->begin(), chromosome->end(), 0.0f);
		}
	);

	// 選出関数の設定
	engine.set_selector(
		[&](auto &container) {
			std::sort(
				container.begin(),
				container.end(),
				[&](const auto &a, const auto &b) {
					return a->fitness() > b->fitness();
				}
			);
			if (container.size() > 10) {
				container.resize(10);
			}
		}
	);

	// 交叉関数の設定
	engine.set_crossover(
		[&](const auto &parent1, const auto &parent2, auto &child1, auto &child2) {
			// 染色体の長さ
			auto size = std::min(parent1->size(), parent2->size());

			// 一点交叉
			std::uniform_int_distribution<decltype(size)> index_selector(1, size - 1);
			auto split = index_selector(mt);
			auto generator = [&](bool b) {
				size_t index = 0;
				return std::make_shared<::chromosome_type>(
					size,
					[&]() {
						return ((index >= split) ? (b ? parent1 : parent2) : (b ? parent2 : parent1))->gene(index++);
					}
				);
			};
			child1 = generator(false);
			child2 = generator(true);
		}
	);

	// 突然変異の設定
	engine.set_mutator(
		[&](const auto &chromosome) {
			std::uniform_int_distribution<int> index_selector(0, chromosome->size() - 1);
			chromosome->gene(index_selector(mt)) = random_initializer();
		}
	);

	// 実行
	engine.evolve(100);

	// ゲノムの出力
	::print_chromosomes(static_cast<const ::engine &>(engine).chromosome_container());

#if _DEBUG
	system("pause");
#endif

    return 0;
}

