#include <iostream>
#include <string>

#include "entity_component_system/entity_component_system.hpp"

namespace ecs = entity_component_system;

void test_system() {
	std::cout << "test_system ----------" << std::endl;

	ecs::system<int, int, std::string> system;
	system.emplace_component(100, 123, 456, "foo");
	system.emplace_component(200, 789, 741, "bar");
	system.emplace_component(300, 852, 963, "baz");

	auto component = system.get_component(100);

	std::get<1>(component) = 173;

	std::cout
		<< std::get<1>(component) << ", "
		<< std::get<2>(component) << ", "
		<< std::get<3>(component)
		<< std::endl;

	std::cout << std::endl;

	for (auto a : system.get_members<2>()) {
		std::cout << a << std::endl;
	}

	std::cout << std::endl;
}

void test_empty_system() {
	std::cout << "test_empty_system ----------" << std::endl;

	ecs::system<> system;
	system.emplace_component(123);
	auto component = system.get_component(123);
	std::cout << typeid(component).name() << std::endl;

	std::cout << std::endl;
}

void test_component() {
	std::cout << "test_component ----------" << std::endl;

	using position_system = ecs::system<float, float>;

	struct position {
		using target = position_system::component_view;
		constexpr static decltype(auto) x(target &tuple) { return std::get<1>(tuple); }
		constexpr static decltype(auto) y(target &tuple) { return std::get<2>(tuple); }
	};

	position_system system;
	system.emplace_component(100, 123.456f, 456.0f);
	auto component = system.get_component(100);
	position::x(component) = 789.123f;
	std::cout << position::x(component) << std::endl;

	std::cout << std::endl;
}

void test_remove_component() {
	std::cout << "test_remove_component ----------" << std::endl;

	ecs::system<int> system;

	system.emplace_component(0, 123);
	system.emplace_component(1, 456);
	system.emplace_component(2, 789);

	for (auto &member : system.get_members<0>()) {
		std::cout << member << std::endl;
	}

	system.remove_component(1);
	system.emplace_component(3, 999);

	std::cout << "----------" << std::endl;
	for (auto &member : system.get_members<0>()) {
		std::cout << member << std::endl;
	}

	std::cout << std::endl;
}

void test_system_size() {
	std::cout << "test_system_size ----------" << std::endl;

	std::cout << "ecs::world_system<>: " << sizeof(ecs::system<>) << std::endl;
	std::cout << "ecs::world_system<int>: " << sizeof(ecs::system<int>) << std::endl;
	std::cout << "ecs::world_system<int, int>: " << sizeof(ecs::system<int, int>) << std::endl;
	std::cout << "ecs::world_system<int, int, int, int>: " << sizeof(ecs::system<int, int, int, int>) << std::endl;
	std::cout << "ecs::world_system<int, int, int, int, int, int, int, int>: " << sizeof(ecs::system<int, int, int, int, int, int, int, int>) << std::endl;
}


void test_world() {
	std::cout << "test_world ----------" << std::endl;

	using name_system = ecs::system<std::string>;
	using position_system = ecs::system<int, int>;
	using health_system = ecs::system<float>;

	using my_world = ecs::world<name_system, position_system, health_system>;

	enum systems : size_t {
		k_name_system = 0,
		k_position_system = 1,
		k_health_system = 2,
	};

	auto print_name = [](const name_system::component &component) {
		std::cout << "\tname {" << std::endl;
		std::cout << "\t\tname: " << std::get<1>(component) << std::endl;
		std::cout << "\t}" << std::endl;
	};

	auto print_position = [](const position_system::component &component) {
		std::cout << "\tposition {" << std::endl;
		std::cout << "\t\tx: " << std::get<1>(component) << std::endl;
		std::cout << "\t\ty: " << std::get<2>(component) << std::endl;
		std::cout << "\t}" << std::endl;
	};

	auto print_health = [](const health_system::component &component) {
		std::cout << "\thealth {" << std::endl;
		std::cout << "\t\tpoint: " << std::get<1>(component) << std::endl;
		std::cout << "\t}" << std::endl;
	};

	auto print_entity = [&](const my_world &world, ecs::entity_id id) {
		std::cout << "entity {" << std::endl;
		std::cout << "\tid: " << id << std::endl;
		print_name(world.get_component<k_name_system>(id));
		print_position(world.get_component<k_position_system>(id));
		print_health(world.get_component<k_health_system>(id));
		std::cout << "}" << std::endl;
	};

	my_world world;
	{
		auto entity = world.make_entity();
		entity.emplace_component<k_name_system>(std::string("alpha"));
		entity.emplace_component<k_position_system>(123, 456);
		entity.emplace_component<k_health_system>(0.54f);
	}
	ecs::entity_id id = 0;
	{
		auto entity = world.make_entity();
		world.emplace_component<k_name_system>(entity, std::string("bravo"));
		world.emplace_component<k_position_system>(entity, 789, 999);
		world.emplace_component<k_health_system>(entity, 0.01f);
		id = entity;
	}
	{
		auto entity = world.make_entity();
		world.emplace_component<k_name_system>(entity, std::string("chalie"));
		world.emplace_component<k_position_system>(entity, 100, 200);
		world.emplace_component<k_health_system>(entity, 1.25f);
	}
	world.remove_entity(id);
	{
		auto entity = world.make_entity();
		world.emplace_component<k_name_system>(entity, std::string("delta"));
		world.emplace_component<k_position_system>(entity, 12345, 67890);
		world.emplace_component<k_health_system>(entity, 123.456f);
	}
	for (auto &id : world.entities()) {
		print_entity(world, id);
	}
	{
		std::cout << "position:y [ " << std::endl;
		for (auto &member : world.get_members<k_position_system, 1>()) {
			std::cout << "\t" << member << std::endl;
		}
		std::cout << " ]" << std::endl;
	}
}

int main() {
	//test_system();
	//test_empty_system();
	//test_component();
	//test_remove_component();
	//test_system_size();
	test_world();

#if _DEBUG
	system("pause");
#endif

	return 0;
}
