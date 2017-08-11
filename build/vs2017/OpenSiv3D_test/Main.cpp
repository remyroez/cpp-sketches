# include <Siv3D.hpp> // OpenSiv3D v0.1.5

#include <functional>
#include <shared_mutex>
#include <thread>

#include "entity_component_system/entity_component_system.hpp"

namespace ecs = entity_component_system;

namespace {

template <class T>
class thread_safe : public T {
public:
	using T::T;

	using mutex_type = std::shared_mutex;

	mutex_type &mutex() { return _mutex; }

private:
	mutex_type _mutex;
};

using lock_guard = std::lock_guard<std::shared_mutex>;
using shared_lock = std::shared_lock<std::shared_mutex>;

template <typename... Args>
using system = thread_safe<ecs::system<Args...>>;

using circle_system = system<Circle>;

struct circle_component {
	enum index : size_t {
		entity,
		circle,
	};
};

using color_system = system<HSV>;

struct color_component {
	enum index : size_t {
		entity,
		color,
	};
};

using move_system = system<Vec2>;

struct move_component {
	enum index : size_t {
		entity,
		velocity,
	};
};

using gravity_system = system<>;

struct gravity_component {
	enum index : size_t {
		entity,
	};
};

using color_transition_system = system<double>;

struct color_transition_component {
	enum index : size_t {
		entity,
		hue,
	};
};

struct life_t {
	life_t(double value) : current_life(value), initial_life(value) {}
	life_t() : life_t(0) {}
	double current_life;
	double initial_life;
};

using life_transition_system = system<life_t>;

struct life_component {
	enum index : size_t {
		entity,
		life,
	};
};

using World = thread_safe<ecs::world<
	circle_system,
	color_system,
	move_system,
	color_transition_system,
	life_transition_system,
	gravity_system
>>;

enum world_system : size_t {
	circle,
	color,
	move,
	color_transition,
	life,
	gravity
};

void resetBalls(World &world, int num = 1) {
	lock_guard lock(world.mutex());
	lock_guard lock2(world.get_system<world_system::circle>().mutex());
	lock_guard lock3(world.get_system<world_system::color>().mutex());
	lock_guard lock4(world.get_system<world_system::move>().mutex());
	lock_guard lock5(world.get_system<world_system::color_transition>().mutex());
	lock_guard lock6(world.get_system<world_system::life>().mutex());
	lock_guard lock7(world.get_system<world_system::gravity>().mutex());

	world.clear();

	for (int i = 0; i < num; ++i) {
		auto entity = world.make_entity();
		entity.emplace_component<world_system::circle>(Circle(Vec2(Random(0, Window::Width()), Random(0, Window::Height())), Random(10, 50)));
		entity.emplace_component<world_system::color>(RandomHSV());
		entity.emplace_component<world_system::move>(RandomVec2(Random(10.0, 1000.0)));
		entity.emplace_component<world_system::color_transition>(360.0);
		entity.emplace_component<world_system::gravity>();
		//entity.emplace_component<world_system::life>(Random(1.0, 10.0));
	}
}

void addEffects(World &world, int num = 1) {
	lock_guard lock(world.mutex());
	lock_guard lock2(world.get_system<world_system::circle>().mutex());
	lock_guard lock3(world.get_system<world_system::move>().mutex());
	lock_guard lock4(world.get_system<world_system::color_transition>().mutex());
	lock_guard lock5(world.get_system<world_system::life>().mutex());
	lock_guard lock6(world.get_system<world_system::color>().mutex());
	for (int i = 0; i < num; ++i) {
		auto entity = world.make_entity();
		entity.emplace_component<world_system::circle>(Circle(Cursor::Pos(), Random(1, 10)));
		entity.emplace_component<world_system::color>(RandomHSV());
		entity.emplace_component<world_system::move>(RandomVec2(Random(100.0, 300.0)));
		entity.emplace_component<world_system::color_transition>(360.0);
		entity.emplace_component<world_system::life>(Random(0.1, 0.5));
	}
}

using thread_list = std::vector<std::thread>;

template <class F>
void push_thread(thread_list &threads, F &&f) {
	threads.push_back(std::thread(f));
}

template <class F>
void push_thread(thread_list &threads, F &f) {
	threads.push_back(std::thread(f));
}

} // namespace

void Main()
{
	constexpr int num = 1000;
	
	World world;

	resetBalls(world, num);

	Graphics::SetBackground(ColorF(0.0, 0.0, 0.0));

	const Font font(50);

	const Texture textureCat(Emoji(L"🐈"), TextureDesc::Mipped);

	std::vector<std::thread> threads;

	while (System::Update())
	{
		Window::SetTitle(Profiler::FPS(), L" FPS : entities=", world.entity_size());

		auto rect = Window::ClientRect();

		push_thread(threads, 
			[&rect, &world]() {
				if (rect.leftClicked()) {
					addEffects(world, 10);
				}
			}
		);

		push_thread(threads,
			[&world, &num]() {
				if (KeySpace.down()) {
					resetBalls(world, num);
				}
			}
		);

		push_thread(threads,
			[&]() {
				auto &system = world.get_system<world_system::life>();
				auto &circle_sys = world.get_system<world_system::color>();

				//lock_guard lock(system.mutex());

				auto &entities = system.entities();

				size_t size;
				{
					shared_lock slock(system.mutex());
					size = entities.size();
				}

				auto &lifes = system.get_members<life_component::life>();
				for (size_t i = 0; i < size; ++i) {
					ecs::entity_id entity;
					{
						shared_lock slock(system.mutex());
						entity = entities[i];
					}
					if (entity == ecs::invalid_entity_id) continue;

					{
						lock_guard lock(system.mutex());
						lifes[i].current_life -= System::DeltaTime();
					}
					{
						life_t life;
						{
							shared_lock slock(system.mutex());
							life = lifes[i];
						}
						if (life.current_life < 0) {
							lock_guard lockw(world.mutex());
							world.remove_entity(entity);

						} else if (circle_sys.mutex().try_lock()) {
							if (circle_sys.validate_component(entity)) {
								auto &color = circle_sys.get_member<color_component::color>(entity);
								color.a = life.current_life / life.initial_life;
							}
							circle_sys.mutex().unlock();
						}
					}
				}
			}
		);

		push_thread(threads,
			[&]() {
				auto &system = world.get_system<world_system::color_transition>();
				auto &circle_sys = world.get_system<world_system::color>();
				if (!system.mutex().try_lock_shared()) return;
				//shared_lock slock(system.mutex());
				//shared_lock slock2(circle_sys.mutex());

				auto &entities = system.entities();

				auto &trans = system.get_members<color_transition_component::hue>();
				for (size_t i = 0; i < entities.size(); ++i) {
					ecs::entity_id entity = entities[i];
					if (entity == ecs::invalid_entity_id) continue;

					{
						shared_lock lockc(circle_sys.mutex());
						if (!circle_sys.validate_component(entities[i])) continue;
					}
					if (circle_sys.mutex().try_lock()) {
						//lock_guard lockc(circle_sys.mutex());
						auto &color = circle_sys.get_member<color_component::color>(entities[i]);
						color.h += trans[i] * System::DeltaTime();
						if (color.h <   0) color.h += 360;
						if (color.h > 360) color.h -= 360;
						circle_sys.mutex().unlock();
					}
				}

				system.mutex().unlock_shared();
			}
		);

		push_thread(threads,
			[&]() {
				auto &system = world.get_system<world_system::gravity>();
				auto &move_sys = world.get_system<world_system::move>();
				shared_lock slock(system.mutex());
				//lock_guard lock2(move_sys.mutex());
				if (!move_sys.mutex().try_lock()) return;

				auto &entities = system.entities();

				for (size_t i = 0; i < entities.size(); ++i) {
					if (entities[i] == ecs::invalid_entity_id) continue;

					{
						if (!move_sys.validate_component(entities[i])) continue;
					}

					{
						auto &velocity = move_sys.get_member<move_component::velocity>(entities[i]);
						velocity.y += 9.80665 * System::DeltaTime() * 100;
					}
				}
				move_sys.mutex().unlock();
			}
		);

		push_thread(threads,
			[&]() {
				auto &system = world.get_system<world_system::move>();
				auto &circle_sys = world.get_system<world_system::circle>();
				
				//lock_guard lock(system.mutex());

				auto &entities = system.entities();
				auto &velocities = system.get_members<move_component::velocity>();

				size_t size;
				{
					shared_lock slock(system.mutex());
					size = entities.size();
				}

				for (size_t i = 0; i < size; ++i) {
					ecs::entity_id entity;
					{
						shared_lock slock(system.mutex());
						entity = entities[i];
					}
					if (entity == ecs::invalid_entity_id) continue;

					{
						shared_lock slock(circle_sys.mutex());
						if (!circle_sys.validate_component(entity)) continue;
					}
					if (system.mutex().try_lock()) {
						//lock_guard locks(system.mutex());
						//lock_guard lockc(circle_sys.mutex());
						if (circle_sys.mutex().try_lock()) {
							auto &center = circle_sys.get_member<circle_component::circle>(entity).center;
							auto &velocity = velocities[i];
							center += velocity * System::DeltaTime();
							constexpr auto cor = 0.9;
							if (center.x < 0) {
								center.x = 0;
								velocity.x = -velocity.x * cor;

							} else if (center.x > rect.w) {
								center.x = rect.w;
								velocity.x = -velocity.x * cor;
							}
							if (center.y < 0) {
								center.y = 0;
								velocity.y = -velocity.y * cor;

							} else if (center.y > rect.h) {
								center.y = rect.h;
								velocity.y = -velocity.y * cor;
							}
							circle_sys.mutex().unlock();
						}
						system.mutex().unlock();
					}
				}
			}
		);

		push_thread(threads,
			[&]() {
				auto &system = world.get_system<world_system::circle>();
				auto &color_sys = world.get_system<world_system::color>();
				shared_lock lock(system.mutex());
				const auto &entities = system.entities();
				const auto &circles = system.get_members<circle_component::circle>();
				for (size_t i = 0; i < circles.size(); ++i) {
					if (entities[i] == ecs::invalid_entity_id) continue;

					HSV color;
					{
						shared_lock lockc(color_sys.mutex());
						color = color_sys.get_member<color_component::color>(entities[i]);
					}
					circles[i].draw(color);
				}
			}
		);

		std::this_thread::yield();

		for (auto &thread : threads) {
			thread.join();
		}
		threads.clear();
#if 0
		font(L"Hello, Siv3D!🐣").drawAt(Window::Center(), Palette::Black);
		font(Cursor::Pos()).draw(20, 400, ColorF(0.6));
		textureCat.resize(80).draw(540, 380);
		Circle(Cursor::Pos(), 60).drawPie(0.5_pi, 1.5_pi, ColorF(1, 0, 0, 0.5));
#endif
	}
}
