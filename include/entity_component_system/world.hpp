
#ifndef ENTITY_COMPONENT_SYSTEM_WORLD_HPP_
#define ENTITY_COMPONENT_SYSTEM_WORLD_HPP_

#include <tuple>
#include <deque>
#include <functional>

#include "utility/id_pool.hpp"
#include "utility/for_each.hpp"

#include "entity.hpp"

namespace entity_component_system {

template <class... Systems>
class world {
public:
	using system_data = std::tuple<Systems...>;

	template <std::size_t I>
	using system = std::tuple_element_t<I, system_data>;

	template <std::size_t I>
	using component = typename system<I>::component;

	using entity_pool = utility::id_pool<entity_id>;
	using entity_list_type = std::deque<entity_id>;

	class entity {
	public:
		using world = world;

		template <size_t I>
		using system = world::system<I>;

		template <size_t I>
		using component = world::component<I>;

	public:
		entity(world &w, entity_id id) : _world(w), _id(id) {}

		entity(const entity &other) = delete;
		entity(entity &&other) : _world(other._world), _id(other._id) {}

		entity &operator=(const entity &other) = delete;
		entity &operator=(entity &&other) = default;

		operator entity_id() const { return id(); }

		entity_id id() const { return _id; }

		template <size_t I = 0>
		decltype(auto) get_system() const { return _world.get_system<I>(); }

		template <size_t I = 0>
		decltype(auto) get_system() { return _world.get_system<I>(); }

		template <size_t I = 0>
		void add_component(component<I> &&initializer) {
			_world.add_component<I>(_id, std::forward<component<I>>(initializer));
		}

		template <size_t I = 0, class... Args>
		void emplace_component(Args&&... args) {
			_world.emplace_component<I>(_id, std::forward<Args>(args)...);
		}

		template <size_t I = 0>
		void remove_component() {
			_world.remove_component<I>(_id);
		}

		template <size_t I = 0>
		decltype(auto) get_component() const {
			return _world.get_component<I>(_id);
		}

		template <size_t I = 0>
		decltype(auto) get_component() {
			return _world.get_component<I>(_id);
		}

		void destroy() {
			_world.remove_entity(_id);
			_id = invalid_entity_id;
		}

	private:
		world &_world;
		entity_id _id;
	};

	static constexpr size_t system_size() { return std::tuple_size_v<system_data>; }

public:
	world() {}

	template <class F>
	decltype(auto) operator()(F &f) {
		return f(*this);
	}

	template <class F>
	decltype(auto) operator()(F &f) const {
		return f(*this);
	}

public:
	template <size_t I = 0>
	decltype(auto) get_system() const { return std::get<I>(_system_data); }

	template <size_t I = 0>
	decltype(auto) get_system() { return std::get<I>(_system_data); }

	const entity_list_type &entities() const { return _entity_list; }

	template <size_t I = 0>
	decltype(auto) system_entities() const {
		return get_system<I>().entities();
	}

	size_t entity_size() const { return entities().size(); }

public:
	entity make_entity() {
		entity_id id = _entity_pool.allocate();
		entity_list().push_back(id);
		return entity(*this, id);
	}

	void remove_entity(entity_id id) {
		entity_list().erase(std::remove(entity_list().begin(), entity_list().end(), id), entity_list().end());
		utility::for_each_in_tuple(
			_system_data,
			[&](auto &system) {
				system.remove_component(id);
			}
		);
		_entity_pool.free(id);
	}

	void clear() {
		utility::for_each_in_tuple(
			_system_data,
			[&](auto &system) {
				system.clear();
			}
		);
		_entity_pool.clear();
		entity_list().clear();
	}

	template <size_t I = 0>
	void add_component(entity_id id, component<I> &&initializer) {
		get_system<I>().add_component(id, initializer);
	}

	template <size_t I = 0, class... Args>
	void emplace_component(entity_id id, Args&&... args) {
		get_system<I>().emplace_component(id, std::forward<Args>(args)...);
	}

	template <size_t I = 0>
	void remove_component(entity_id id) {
		get_system<I>().remove_component(id);
	}

	template <size_t I = 0>
	decltype(auto) get_component(entity_id id) const {
		return get_system<I>().get_component(id);
	}

	template <size_t I = 0>
	decltype(auto) get_component(entity_id id) {
		return get_system<I>().get_component(id);
	}

	template <size_t I = 0, std::size_t Member>
	decltype(auto) get_members() {
		return get_system<I>().get_members<Member>();
	}

	template <std::size_t Index = 0, class Function>
	decltype(auto) invoke_system(Function &f) {
		return f(*this, get_system<Index>());
	}

	template <std::size_t Index = 0, class Function>
	decltype(auto) invoke_system(const Function &f) const {
		return f(*this, get_system<Index>());
	}

	template <std::size_t Index = 0, class Function>
	decltype(auto) invoke_system(Function &&f) {
		return f(*this, get_system<Index>());
	}

	template <std::size_t Index = 0, class Function>
	decltype(auto) invoke_system(const Function &&f) const {
		return f(*this, get_system<Index>());
	}

	template <class F>
	void for_each_system(F &&f) {
		utility::for_each_in_tuple(_system_data, f);
	}

	template <class F>
	void for_each_system(const F &&f) const {
		utility::for_each_in_tuple(_system_data, f);
	}

	template <class F>
	void for_each_system(F &f) {
		utility::for_each_in_tuple(_system_data, f);
	}

	template <class F>
	void for_each_system(const F &f) const {
		utility::for_each_in_tuple(_system_data, f);
	}

protected:
	entity_list_type &entity_list() { return _entity_list; }

private:
	system_data _system_data;
	entity_pool _entity_pool;
	entity_list_type _entity_list;
};

} // namespace entity_component_system

#endif // ENTITY_COMPONENT_SYSTEM_WORLD_HPP_
