
#ifndef GENETIC_ALGORITHM_CHROMOSOME_HPP_
#define GENETIC_ALGORITHM_CHROMOSOME_HPP_

#include <vector>

namespace genetic_algorithm {

template <class T = int, class U = std::vector<T>>
class base_chromosome {
public:
	using gene_type = T;
	using container_type = U;
	using container_size_type = typename container_type::size_type;

	using fitness_type = float;

public:
	base_chromosome() : _fitness(0), _gene_container(){}
	base_chromosome(const base_chromosome &other) : _fitness(other.fitness()), _gene_container(other.gene_container()) {}
	base_chromosome(base_chromosome &&other) : _fitness(other.fitness()), _gene_container(std::move(other.gene_container())) {}

	base_chromosome(const container_type &genes) : _fitness(0), _gene_container(genes) {}
	base_chromosome(const container_type &&genes) : _fitness(0), _gene_container(genes) {}

	template <class Function>
	base_chromosome(container_size_type size, Function fn) : base_chromosome() { generate(size, fn); }

public:
	const container_type &gene_container() const { return _gene_container; }
	container_type &gene_container() { return const_cast<container_type &>(static_cast<const base_chromosome *>(this)->gene_container()); }

	fitness_type fitness() const { return _fitness; }

	void set_fitness(fitness_type fitness) { _fitness = fitness; }

public:
	template <class V>
	const gene_type &gene(const V &key) const { return gene_container().at(key); }

	template <class V>
	gene_type &gene(const V &key) { return gene_container().at(key); }

	template <class V>
	const gene_type &operator[](const V &key) const { return _gene_container[key]; }

	template <class V>
	gene_type &operator[](const V &key) { return _gene_container[key]; }

	auto begin() { return gene_container().begin(); }
	auto begin() const { return gene_container().begin(); }
	auto end() { return gene_container().end(); }
	auto end() const { return gene_container().end(); }

	auto cbegin() const { return gene_container().cbegin(); }
	auto cend() const { return gene_container().cend(); }

	auto rbegin() { return gene_container().rbegin(); }
	auto rbegin() const { return gene_container().rbegin(); }
	auto rend() { return gene_container().rend(); }
	auto rend() const { return gene_container().rend(); }

	auto crbegin() const { return gene_container().crbegin(); }
	auto crend() const { return gene_container().crend(); }

	bool empty() const { return gene_container().empty(); }

	container_size_type size() const { return gene_container().size(); }

	void resize(container_size_type size) { gene_container().resize(size); }

	template <class Function>
	void generate(container_size_type size, Function fn) {
		resize(size);
		std::generate(begin(), end(), fn);
	}

	base_chromosome clone() const { return std::move(base_chromosome(*this)); }

private:
	container_type _gene_container;
	fitness_type _fitness;
};

using chromosome = base_chromosome<>;

} // namespace genetic_algorithm

#endif // GENETIC_ALGORITHM_CHROMOSOME_HPP_
