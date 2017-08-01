
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

public:
	base_chromosome() {}
	base_chromosome(const base_chromosome &other) = default;

	const container_type &gene_container() const { return _gene_container; }
	container_type &gene_container() { return const_cast<container_type &>(static_cast<const base_chromosome *>(this)->gene_container()); }

	template <class V>
	const gene_type &gene(const V &key) const { return gene_container()[key]; }

	template <class V>
	gene_type &gene(const V &key) { return const_cast<gene_type &>(static_cast<const base_chromosome *>(this)->gene(key)); }

	template <class V>
	const gene_type &operator[](const V &key) const { return gene(key); }

	template <class V>
	gene_type &operator[](const V &key) { return gene(key); }

	container_size_type size() const { return gene_container().size(); }

private:
	container_type _gene_container;
};

using chromosome = base_chromosome<>;

} // namespace genetic_algorithm

#endif // GENETIC_ALGORITHM_CHROMOSOME_HPP_
