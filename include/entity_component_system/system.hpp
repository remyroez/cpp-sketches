
#ifndef ENTITY_COMPONENT_SYSTEM_SYSTEM_HPP_
#define ENTITY_COMPONENT_SYSTEM_SYSTEM_HPP_

#include <tuple>
#include <vector>
#include <unordered_map>
#include <deque>

#include "utility/id_pool.hpp"
#include "utility/for_each.hpp"

namespace entity_component_system {

template <typename... Args>
class system {
public:
	using data_type = std::tuple<std::vector<Args>...>;

	using component = std::tuple<Args...>;
	using component_view = std::tuple<Args &...>;
	using component_index_type = std::size_t;
	using component_index_pool = utility::id_pool<component_index_type>;

	using entity_map_type = std::unordered_map<entity_id, component_index_type>;

public:
	static component make_component(Args&&... args) {
		return std::make_tuple(std::forward<Args>(args)...);
	}

public:
	system() {}

public:
	template <std::size_t Index>
	decltype(auto) get_members() {
		return std::get<Index>(data());
	}

	const data_type &data() const { return _data; }
	data_type &data() { return _data; }

public:
	void add_component(entity_id id, component &&initializer) {
		const auto index = allocate_component_index();
		if (register_entity(id, index)) {
			get_component_from_index(index) = initializer;
		}
	}

	void emplace(entity_id id, Args&&... args) {
		add_component(id, make_component(std::forward<Args>(args)...));
	}

	void remove_component(entity_id id) {
		auto index = get_component_index(id);
		free_component_index(index);
		deregister_entity(id);
	}

	decltype(auto) get_component(entity_id id) const {
		return get_component_from_index(get_component_index(id));
	}

	decltype(auto) get_component(entity_id id) {
		return get_component_from_index(get_component_index(id));
	}

	void clear() {
		utility::for_each_in_tuple(
			data(),
			[](auto &members) {
				members.clear();
			}
		);
		entity_map().clear();
		_component_index_pool.clear();
	}

protected:
	template <std::size_t Index, typename Tuple>
	static decltype(auto) get_element(Tuple &tuple, component_index_type index) {
		auto &container = std::get<Index>(tuple);
		if (index >= container.size()) {
			container.resize(index + 1);
		}
		return container[index];
	}

	template <std::size_t Index, typename Tuple>
	static decltype(auto) get_element(const Tuple &tuple, component_index_type index) {
		auto &container = std::get<Index>(tuple);
#if 0
		if (index >= container.size()) {
			container.resize(index + 1);
		}
#endif
		return container[index];
	}

	template <typename T, std::size_t... Indices>
	static auto make_component_handle(T &tuple, component_index_type index, std::index_sequence<Indices...>) {
		return std::tie(get_element<Indices>(tuple, index)...);
	}

	template <typename T, std::size_t... Indices>
	static auto make_component_handle(const T &tuple, component_index_type index, std::index_sequence<Indices...>) {
		return std::tie(get_element<Indices>(tuple, index)...);
	}

protected:
	decltype(auto) get_component_from_index(component_index_type index) const {
		return make_component_handle(data(), index, std::index_sequence_for<Args...>());
	}

	decltype(auto) get_component_from_index(component_index_type index) {
		return make_component_handle(data(), index, std::index_sequence_for<Args...>());
	}

	bool register_entity(entity_id id, component_index_type index) {
		return entity_map().emplace(id, index).second;
	}

	bool deregister_entity(entity_id id) {
		return (entity_map().erase(id) > 0);
	}

	component_index_type get_component_index(entity_id id) const {
		return entity_map().at(id);
	}

	const entity_map_type &entity_map() const { return _entity_map; }
	entity_map_type &entity_map() { return _entity_map; }

	component_index_type allocate_component_index() {
		return _component_index_pool.allocate();
	}

	void free_component_index(component_index_type index) {
		_component_index_pool.free(index);
	}

private:
	entity_map_type _entity_map;
	component_index_pool _component_index_pool;
	data_type _data;
};

} // namespace entity_component_system

#endif // ENTITY_COMPONENT_SYSTEM_SYSTEM_HPP_
