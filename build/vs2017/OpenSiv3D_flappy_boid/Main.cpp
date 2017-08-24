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
		return (_update_function ? _update_function(delta_time) : static_cast<decltype(_update_function(delta_time))>(0));
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

struct point_holder {
	int x;
	std::function<void()> callback;

	void operator()() {
		if (callback) {
			callback();
		}
	}
};

using score_list = std::vector<point_holder>;

struct game_context {
	game_world world;
	rect_list rects;
	score_list scores;
	int score = 0;

	void clear() {
		world.clear();
		rects.clear();
		scores.clear();
		score = 0;
	}
};

struct boid {
	game_context *context;

	Circle circle;
	HSV color;
	double velocity_y;

	int score = 0;

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
			for (auto *rect : context->rects) {
				if (rect->intersects(circle)) {
					alive = false;
				}
			}
		}

		if (alive) {
			// 落下
			velocity_y += gravity * dt * 100;
			//if (velocity_y >   80) velocity_y =   80;
			//if (velocity_y < -100) velocity_y = -100;

			// スコア加算
			for (size_t i = 0; i < context->scores.size(); ++i) {
				auto &point = context->scores[i];
				if (circle.center.x > point.x) {
					context->score++;
					point();
					context->scores.erase(context->scores.begin() + i);
					break;
				}
			}

			// ジャンプ
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
	game_context *context;

	Rect rect;
	Rect rect2;
	Color color;

	bool cleared = false;

	bool operator()(double delta_time) {
		bool alive = update(delta_time);
		if (alive) draw();
		return alive;
	}

	bool update(double dt) {
		bool alive = true;

		rect.x -= 4;
		rect2.x = rect.x;
		if (rect.x <= -rect.w) {
			alive = false;
		}

		if (alive) {
			context->rects.emplace_back(&rect);
			context->rects.emplace_back(&rect2);
			if (!cleared) {
				context->scores.push_back({ rect.x + rect.w / 2, [&] { cleared = true; } });
			}
		}

		return alive;
	}

	void draw() {
		rect.draw(color);
		rect.drawFrame(3, Palette::Black);

		rect2.draw(color);
		rect2.drawFrame(3, Palette::Black);
	}
};

void add_drainpipe(game_context &context);

struct stage {
	game_context *context;

	double span;
	double initial_span;

	bool operator()(double delta_time) {
		span -= delta_time;
		if (span < 0) {
			add_drainpipe(*context);
			span = initial_span;
		}
		return true;
	}

};

template <class T>
actor::pointer make_actor(T &&fn) {
	return std::make_shared<::actor>(fn);
}

void add_boid(game_context &context) {
	auto entity = context.world.make_entity();
	auto height = 30;
	entity.emplace_component<game_components::boid>(
		make_actor<boid>({ &context, Circle(Vec2(Window::Width() * 0.2, Window::Height() / 2), height / 2), RandomHSV(), 0 })
	);
}

void add_drainpipe(game_context &context) {
	auto center_y = Window::Center().y;
	auto y = center_y - Random(-100, 100);
	{
		auto entity = context.world.make_entity();
		entity.emplace_component<game_components::drainpipe>(
			make_actor<drainpipe>({
				&context,
				Rect(Window::Width() + 50, y + 50, 50, Window::Height()),
				Rect(Window::Width() + 50, y - 50 - Window::Height(), 50, Window::Height()),
				Palette::Lightgreen,
				false
			})
		);
	}
}

void setup_world(game_context &context) {
	auto &world = context.world;

	context.clear();

	add_boid(context);
	{
		auto entity = world.make_entity();
		entity.emplace_component<game_components::system>(
			make_actor<stage>({ &context, 0.0, 1.0 })
		);
	}
}

} // namespace

void Main()
{
	Graphics::SetBackground(ColorF(0.8, 0.9, 1.0));

	const Font font(30);

	::game_context context;
	auto &world = context.world;
	auto &rects = context.rects;
	auto &scores = context.scores;

	setup_world(context);

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

	bool reset = false;

	while (System::Update())
	{
		Window::SetTitle(L"Flappy Boid -  FPS: ", Profiler::FPS());

		if (reset) {
			setup_world(context);
			reset = false;
		}

		world.invoke_system<game_components::system>(invoker);

		rects.clear();
		scores.clear();
		world.invoke_system<game_components::drainpipe>(invoker);

		auto count = world.invoke_system<game_components::boid>(invoker);

		if (count == 0) {
			reset = true;
		}

		if (MouseR.down()) {
			add_boid(context);
		}

		{
			auto center = Window::Center();
			auto height = Window::Height() * 0.8 - font.height() / 2;

			font(L"Score:").draw(Arg::topRight = Vec2{ center.x, height }, Palette::Gray);
			font(L" ", context.score).draw(Arg::topLeft = Vec2{ center.x, height }, Palette::Gray);

			height += font.height();

			font(L"Alive:").draw(Arg::topRight = Vec2{ center.x, height }, Palette::Gray);
			font(L" ", count).draw(Arg::topLeft = Vec2{ center.x, height }, Palette::Gray);
		}
	}
}
