# include <Siv3D.hpp> // OpenSiv3D v0.1.6

#include <functional>
#include <memory>
#include <vector>

#include "entity_component_system/entity_component_system.hpp"

namespace ecs = entity_component_system;

namespace {

class actor {
public:
	using pointer = std::shared_ptr<actor>;
	using handle = std::weak_ptr<actor>;
	using update_function = std::function<bool(double)>;

	actor(update_function &fn) : _update_function(fn) {}
	actor(update_function &&fn) : _update_function(fn) {}
	virtual ~actor() {}

	decltype(auto) invoke(double delta_time) {
		return (_update_function ? _update_function(delta_time) : false);
	}

private:
	update_function _update_function;
};

using actor_system = ecs::system<actor::pointer>;

struct actor_component {
	enum index : size_t {
		entity,
		actor
	};
};

using game_world = ecs::world<actor_system, actor_system, actor_system>;

struct game_components {
	enum index : size_t {
		system,
		boid,
		drainpipe
	};
};

constexpr double gravity = 9.80665;

using rect_list = std::vector<Rect *>;

struct boid {
	rect_list *list;
	Circle circle;
	HSV color;
	double velocity_y;

	bool operator()(double dt) {
		bool alive = update(dt);
		if (alive) draw();
		return alive;
	}

	bool update(double dt) {
		bool alive = true;

		circle.center.y += velocity_y * dt;
		if (circle.center.y <= -circle.r) {
			alive = false;

		} else if (circle.center.y > Window::Height() + circle.r) {
			alive = false;

		} else {
			for (auto *rect : *list) {
				if (rect->intersects(circle)) {
					alive = false;
				}
			}
		}

		if (alive) {
			velocity_y += gravity * dt * 100;
			//if (velocity_y >   80) velocity_y =   80;
			//if (velocity_y < -100) velocity_y = -100;

			if (MouseL.down()) {
				jump();
			}
		}

		return alive;
	}

	void draw() {
		circle.draw(color);
		circle.drawFrame(3, Palette::Black);
	}

	void jump() {
		velocity_y = -gravity * 30;
	}
};

struct drainpipe {
	rect_list *list;

	Rect rect;
	Color color;

	bool operator()(double delta_time) {
		bool alive = update(delta_time);
		if (alive) draw();
		return alive;
	}

	bool update(double dt) {
		bool alive = true;

		rect.x -= 4;
		if (rect.x <= -rect.w) {
			alive = false;
		}

		if (alive) {
			list->emplace_back(&rect);
		}

		return alive;
	}

	void draw() {
		rect.draw(color);
		rect.drawFrame(3, Palette::Black);
	}
};

void add_drainpipe(game_world &world, rect_list *list);

struct stage {
	game_world *world;
	rect_list *list;
	double span;
	double initial_span;

	bool operator()(double delta_time) {
		span -= delta_time;
		if (span < 0) {
			add_drainpipe(*world, list);
			span = initial_span;
		}
		return true;
	}

};

template <class T>
actor::pointer make_actor(T &&fn) {
	return std::make_shared<::actor>(fn);
}

void add_boid(game_world &world, rect_list *list) {
	auto entity = world.make_entity();
	auto height = 30;
	entity.emplace_component<game_components::boid>(
		make_actor<boid>({ list, Circle(Vec2(Window::Width() * 0.2, Window::Height() / 2), height / 2), RandomHSV(), 0 })
	);
}

void add_drainpipe(game_world &world, rect_list *list) {
	auto center_y = Window::Center().y;
	auto y = center_y - Random(-100, 100);
	{
		auto entity = world.make_entity();
		entity.emplace_component<game_components::drainpipe>(
			make_actor<drainpipe>({ list, Rect(Window::Width() + 50, y + 50, 50, Window::Height()), Palette::Lightgreen })
		);
	}
	{
		auto entity = world.make_entity();
		entity.emplace_component<game_components::drainpipe>(
			make_actor<drainpipe>({ list, Rect(Window::Width() + 50, y - 50 - Window::Height(), 50, Window::Height()), Palette::Lightgreen })
		);
	}
}

} // namespace

void Main()
{
	Graphics::SetBackground(ColorF(0.8, 0.9, 1.0));

	const Font font(50);
	const Font font_score(10);

	const Texture textureCat(Emoji(L"🐦"), TextureDesc::Mipped);

	rect_list rects;

	::game_world world;

	add_boid(world, &rects);
	{
		auto entity = world.make_entity();
		entity.emplace_component<game_components::system>(
			make_actor<stage>({ &world, &rects, 0.0, 1.0 })
		);
	}

	auto invoker = [](auto &world, auto &system) {
		size_t count = 0;
		const auto &entities = system.entities();
		auto &actors = system.get_members<actor_component::actor>();
		for (size_t i = 0; i < entities.size(); ++i) {
			if (entities[i] == ecs::invalid_entity_id) continue;
			if (!actors[i]->invoke(System::DeltaTime())) {
				world.remove_entity(entities[i]);
			} else {
				count++;
			}
		}
		return count;
	};

	while (System::Update())
	{
		Window::SetTitle(L"Flappy Boid -  FPS: ", Profiler::FPS());

		world.invoke_system<game_components::system>(invoker);

		rects.clear();
		world.invoke_system<game_components::drainpipe>(invoker);

		auto count = world.invoke_system<game_components::boid>(invoker);

		if (MouseR.down()) {
			add_boid(world, &rects);
		}

		font(L"Alive: ", count).draw(20, 400, ColorF(0.6));
	}
}
