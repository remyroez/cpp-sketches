
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
	using entity_list = std::vector<entity_id>;

	using data_type = std::tuple<entity_list, std::vector<Args>...>;

	using component = std::tuple<entity_id, Args...>;
	using component_view = std::tuple<entity_id &, Args &...>;
	using component_index_type = std::size_t;
	using component_index_pool = utility::id_pool<component_index_type>;

	using entity_map_type = std::unordered_map<entity_id, component_index_type>;

	static constexpr size_t member_size() { return std::tuple_size_v<data_type>; }

public:
	static component make_component(entity_id id, Args&&... args) {
		return std::make_tuple(id, std::forward<Args>(args)...);
	}

public:
	system() {}

	template <class F>
	decltype(auto) operator()(F &f) {
		return f(*this);
	}

	template <class F>
	decltype(auto) operator()(const F &f) const {
		return f(*this);
	}

public:
	template <std::size_t Index>
	decltype(auto) get_members() {
		return std::get<Index>(data());
	}

	template <std::size_t Index>
	decltype(auto) get_members() const {
		return std::get<Index>(data());
	}

	const data_type &data() const { return _data; }
	data_type &data() { return _data; }

	decltype(auto) entities() const {
		return get_members<0>();
	}

	size_t entity_size() const { return entities().size(); }

public:
	void add_component(entity_id id, component &&initializer) {
		const auto index = allocate_component_index();
		if (register_entity(id, index)) {
			get_component_from_index(index) = initializer;
		}
	}

	void add_component(entity_id id) {
		add_component(id, make_component());
	}

	void emplace_component(entity_id id, Args&&... args) {
		add_component(id, make_component(id, std::forward<Args>(args)...));
	}

	void remove_component(entity_id id) {
		if (has_component(id)) {
			auto index = get_component_index(id);
			get_members<0>()[index] = invalid_entity_id;
			free_component_index(index);
			deregister_entity(id);
		}
	}

	decltype(auto) get_component(entity_id id) const {
		return get_component_from_index(get_component_index(id));
	}

	decltype(auto) get_component(entity_id id) {
		return get_component_from_index(get_component_index(id));
	}

	bool has_component(entity_id id) const {
		return entity_map().find(id) != entity_map().end();
	}

	bool validate_component(entity_id id) const {
		return has_component(id) && (get_member<0>(id) != invalid_entity_id);
	}

	template <std::size_t Index>
	decltype(auto) get_member(entity_id id) const {
		return get_members<Index>()[get_component_index(id)];
	}

	template <std::size_t Index>
	decltype(auto) get_member(entity_id id) {
		return get_members<Index>()[get_component_index(id)];
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
	static decltype(auto) make_component_handle(T &tuple, component_index_type index, std::index_sequence<Indices...>) {
		return std::tie(get_element<Indices>(tuple, index)...);
	}

	template <typename T, std::size_t... Indices>
	static decltype(auto) make_component_handle(const T &tuple, component_index_type index, std::index_sequence<Indices...>) {
		return std::tie(get_element<Indices>(tuple, index)...);
	}

protected:
	decltype(auto) get_component_from_index(component_index_type index) const {
		return make_component_handle(data(), index, std::index_sequence_for<entity_id, Args...>());
	}

	decltype(auto) get_component_from_index(component_index_type index) {
		return make_component_handle(data(), index, std::index_sequence_for<entity_id, Args...>());
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
