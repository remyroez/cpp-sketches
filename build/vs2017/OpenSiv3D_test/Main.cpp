# include <Siv3D.hpp> // OpenSiv3D v0.1.5

#include <functional>

#include "entity_component_system/entity_component_system.hpp"

namespace ecs = entity_component_system;

namespace {

using circle_system = ecs::system<Circle, HSV>;

struct circle_component {
	enum index : size_t {
		entity,
		circle,
		color,
	};
};

using move_system = ecs::system<Vec2>;

struct move_component {
	enum index : size_t {
		entity,
		velocity,
	};
};

using color_transition_system = ecs::system<double>;

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

using life_transition_system = ecs::system<life_t>;

struct life_component {
	enum index : size_t {
		entity,
		life,
	};
};

using World = ecs::world<
	circle_system,
	move_system,
	color_transition_system,
	life_transition_system
>;

enum world_system : size_t {
	circle,
	move,
	color_transition,
	life,
};

} // namespace

void Main()
{
	constexpr int num = 10;
	
	World world;
	for (int i = 0; i < num; ++i) {
		auto entity = world.make_entity();
		entity.emplace_component<world_system::circle>(Circle(Vec2(Random(0, Window::Width()), Random(0, Window::Height())), Random(10, 50)), RandomHSV());
		entity.emplace_component<world_system::move>(RandomVec2(Random(10.0, 1000.0)));
		entity.emplace_component<world_system::color_transition>(360.0);
		//entity.emplace_component<world_system::life>(Random(1.0, 10.0));
	}

	Graphics::SetBackground(ColorF(0.0, 0.0, 0.0));

	const Font font(50);

	const Texture textureCat(Emoji(L"🐈"), TextureDesc::Mipped);

	while (System::Update())
	{
		Window::SetTitle(Profiler::FPS(), L" FPS : entities=", world.entity_size());

		auto rect = Window::ClientRect();

		if (rect.leftClicked()) {
			for (int i = 0; i < 10; ++i) {
				auto entity = world.make_entity();
				entity.emplace_component<world_system::circle>(Circle(Cursor::Pos(), Random(1, 10)), RandomHSV());
				entity.emplace_component<world_system::move>(RandomVec2(Random(100.0, 300.0)));
				entity.emplace_component<world_system::color_transition>(360.0);
				entity.emplace_component<world_system::life>(Random(0.1, 0.5));
			}
		}

		world.invoke_system<world_system::life>(
			[](auto &world, auto &system) {
				auto &circle_sys = world.get_system<world_system::circle>();

				auto &entities = system.entities();
				auto &lifes = system.get_members<life_component::life>();
				for (size_t i = 0; i < entities.size(); ++i) {
					if (entities[i] == ecs::invalid_entity_id) continue;
					lifes[i].current_life -= System::DeltaTime();
					if (lifes[i].current_life < 0) {
						world.remove_entity(entities[i]);

					} else {
						auto color_component = circle_sys.get_component(entities[i]);
						auto &color = std::get<circle_component::color>(color_component);
						color.a = lifes[i].current_life / lifes[i].initial_life;
					}
				}
			}
		);

		world.invoke_system<world_system::color_transition>(
			[](auto &world, auto &system) {
				auto &circle_sys = world.get_system<world_system::circle>();
				auto &entities = system.entities();
				auto &trans = system.get_members<color_transition_component::hue>();
				for (size_t i = 0; i < entities.size(); ++i) {
					if (entities[i] == ecs::invalid_entity_id) continue;
					auto component = circle_sys.get_component(entities[i]);
					auto &color = std::get<circle_component::color>(component);
					color.h += trans[i] * System::DeltaTime();
					if (color.h <   0) color.h += 360;
					if (color.h > 360) color.h -= 360;
				}
			}
		);

		world.invoke_system<world_system::move>(
			[&](auto &world, auto &system) {
				auto &circle_sys = world.get_system<world_system::circle>();

				auto &entities = system.entities();
				auto &velocities = system.get_members<move_component::velocity>();

				for (size_t i = 0; i < entities.size(); ++i) {
					if (entities[i] == ecs::invalid_entity_id) continue;

					auto component = circle_sys.get_component(entities[i]);
					auto &center = std::get<circle_component::circle>(component).center;
					auto &velocity = velocities[i];
					center += velocity * System::DeltaTime();
					{
						if (center.x < 0) {
							center.x = 0;
							velocity.x = -velocity.x;

						} else if (center.x > rect.w) {
							center.x = rect.w;
							velocity.x = -velocity.x;
						}
						if (center.y < 0) {
							center.y = 0;
							velocity.y = -velocity.y;

						} else if (center.y > rect.h) {
							center.y = rect.h;
							velocity.y = -velocity.y;
						}
					}
				}
			}
		);

		world.invoke_system<world_system::circle>(
			[](auto &, auto &system) {
				const auto &entities = system.entities();
				const auto &circles = system.get_members<circle_component::circle>();
				const auto &colors = system.get_members<circle_component::color>();
				for (size_t i = 0; i < circles.size(); ++i) {
					if (entities[i] == ecs::invalid_entity_id) continue;
					circles[i].draw(colors[i]);
				}
			}
		);

#if 0
		font(L"Hello, Siv3D!🐣").drawAt(Window::Center(), Palette::Black);
		font(Cursor::Pos()).draw(20, 400, ColorF(0.6));
		textureCat.resize(80).draw(540, 380);
		Circle(Cursor::Pos(), 60).drawPie(0.5_pi, 1.5_pi, ColorF(1, 0, 0, 0.5));
#endif
	}
}
