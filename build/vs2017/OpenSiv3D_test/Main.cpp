# include <Siv3D.hpp> // OpenSiv3D v0.1.5

#include <functional>

#include "entity_component_system/entity_component_system.hpp"

namespace ecs = entity_component_system;

using CircleSystem = ecs::system<Circle, HSV>;
using MoveSystem = ecs::system<Vec2>;
using ColorTransitionSystem = ecs::system<double>;

enum systems : size_t {
	circle,
	move,
	color_transition
};

using World = ecs::world<CircleSystem, MoveSystem, ColorTransitionSystem>;

void Main()
{
	constexpr int num = 100;
	
	World world;
	for (int i = 0; i < num; ++i) {
		auto entity = world.make_entity();
		world.emplace<systems::circle>(entity, Circle(Vec2(Random(0, Window::Width()), Random(0, Window::Height())), Random(10, 50)), RandomHSV());
		world.emplace<systems::move>(entity, RandomVec2(Random(10.0, 1000.0)));
		world.emplace<systems::color_transition>(entity, 360.0);
	}

	Graphics::SetBackground(ColorF(0.0, 0.0, 0.0));

	const Font font(50);

	const Texture textureCat(Emoji(L"🐈"), TextureDesc::Mipped);

	while (System::Update())
	{
		Window::SetTitle(Profiler::FPS(), L"FPS");

		auto rect = Window::ClientRect();

		world.invoke_system<systems::color_transition>(
			[](auto &world, auto &system) {
				auto &colors = world.get_system<systems::circle>().get_members<1>();
				auto &trans = system.get_members<0>();
				for (size_t i = 0; i < trans.size(); ++i) {
					colors[i].h += trans[i] * System::DeltaTime();
					if (colors[i].h < 0) colors[i].h += 360;
					if (colors[i].h > 360) colors[i].h -= 360;
				}
			}
		);

		world.invoke_system<systems::move>(
			[&](auto &world, auto &system) {
				auto &circles = world.get_system<systems::circle>().get_members<0>();
				auto &velocities = system.get_members<0>();
				for (size_t i = 0; i < circles.size(); ++i) {
					auto &center = circles[i].center;
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

		world.invoke_system<systems::circle>(
			[](auto &, auto &system) {
				const auto &circles = system.get_members<0>();
				const auto &colors = system.get_members<1>();
				for (size_t i = 0; i < circles.size(); ++i) {
					circles[i].draw(colors[i]);
				}
			}
		);

		//font(L"Hello, Siv3D!🐣").drawAt(Window::Center(), Palette::Black);

		//font(Cursor::Pos()).draw(20, 400, ColorF(0.6));

		//textureCat.resize(80).draw(540, 380);

		//Circle(Cursor::Pos(), 60).drawPie(0.5_pi, 1.5_pi, ColorF(1, 0, 0, 0.5));
	}
}
