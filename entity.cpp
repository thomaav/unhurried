#include <algorithm>
#include <cassert>
#include <cmath>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include "raylib.h"
#include "raymath.h"
#pragma clang diagnostic pop

#include "draw.h"
#include "entity.h"
#include "manager.h"
#include "math.h"

float WALK_TICK_RATE = 0.4f;
float RUN_TICK_RATE = WALK_TICK_RATE / 2.0f;
float TURN_TICK_RATE = 2.1f;
float GAME_TICK_RATE = 0.6f;
float SPRITE_ANIMATION_TICK_RATE = 0.12f;
float ATTACK_TICK_RATE = 8.0f;

entity::entity(tile p, map &map, asset_manager &asset_manager, manager &manager)
    : m_map(map)
    , m_asset_manager(asset_manager)
    , m_manager(manager)
    , m_position_logic(p)
    , m_position_render({ (float)p.x + 0.5f, (float)p.y + 0.5f, 0.0f })
    , m_target_logic(p)
    , m_target_render({ (float)p.x + 0.5f, (float)p.y + 0.5f, 0.0f })
{
}

void entity::tick_combat()
{
	m_current_attack_cooldown = std::max(0.0f, m_current_attack_cooldown - GetFrameTime());
	if (m_current_action == action::ATTACK)
	{
		/* Move if we're not close enough and not currently attacking. */
		if (m_current_attack_cast_time == 0.0f && get_attack_distance(*m_target) > m_current_attack_range)
		{
			if (m_position_logic == m_target_logic)
			{
				/* (TODO, thoave01): Used three places now... some common movement trigger code. */
				std::deque<tile> path = {};
				m_map.generate_path(m_position_logic, get_closest_tile(*m_target), path);
				m_path_logic.push_back(path.front());

				m_target_logic = m_path_logic.front();
				m_path_logic.pop_front();
				m_path_render.push_back(m_target_logic);

				if (m_model.get_active_animation()->m_animation_id == animation_id::PLAYER_ATTACK)
				{
					animation_id id = m_running ? animation_id::PLAYER_RUN : animation_id::PLAYER_WALK;
					m_model.set_active_animation(id);
				}
			}
		}
		/* Attack if we're close enough, (or already attacking -- don't interrupt). */
		else if (m_current_attack_cooldown == 0.0f)
		{
			if (m_model.get_active_animation()->m_animation_id != animation_id::PLAYER_ATTACK)
			{
				m_model.set_active_animation(animation_id::PLAYER_ATTACK);
			}

			/* (TODO, thoave01): Ticked twice, somehow? */
			m_current_attack_cast_time += GetFrameTime();
			if (m_current_attack_cast_time >= m_attack_cast_time)
			{
				m_current_attack_cast_time = 0.0f;
				m_current_attack_cooldown = m_attack_cooldown;

				m_manager.m_active_attacks.emplace_back(*this, *m_target);
			}
		}
	}
	else
	{
		m_current_attack_cast_time = 0.0f;
	}

	/* (TODO, thoave01): Placeholder. */
	if (m_health == 0.0f)
	{
		m_health = 100.0f;
	}
}

void entity::tick_movement_logic()
{
	m_movement_tick += GetFrameTime();

	/* Scale logic movement tick based on length to accommodate diagonal movement. */
	float tick_rate = m_movement_tick_rate;
	const Vector2 current = { (float)m_position_logic.x, (float)m_position_logic.y };
	const Vector2 target = { (float)m_target_logic.x, (float)m_target_logic.y };
	const float scale = Vector2Length(target - current);
	if (scale >= (0.0f + 0.005f))
	{
		tick_rate *= scale;
	}

	while (m_movement_tick > tick_rate)
	{
		m_movement_tick -= tick_rate;
		m_position_logic = m_target_logic;
		if (!m_path_logic.empty())
		{
			m_target_logic = m_path_logic.front();
			m_path_logic.pop_front();
			m_path_render.push_back(m_target_logic);
		}
	}
}

void entity::tick_render()
{
	/* Tick animation. */
	m_model.tick_render();

	if (m_current_action == action::IDLE)
	{
		return;
	}

	/* Don't move if we're close, to avoid stuttering. */
	Vector3 position = m_position_render;
	if (Vector3Length(position - m_target_render) <= 0.05f)
	{
		/* Set next target tile. */
		if (!m_path_render.empty())
		{
			tile next = m_path_render.front();
			m_target_render = { (float)next.x + 0.5f, (float)next.y + 0.5f, 0.0f };
			m_path_render.pop_front();
		}
		else if (m_current_action == action::MOVE && m_path_logic.empty())
		{
			set_action({ action::IDLE, {} });
			return;
		}
	}

	/* Update render position by some increment. */
	Vector2 direction = { m_target_render.x - m_position_render.x, m_target_render.y - m_position_render.y };
	Vector2 direction_normalized = normalize(direction);

	/* (TODO, thoave01): Improve desync catch-up mechanic to sync all the to the logic position. */
	float tick_scale = 1.0f;
	float normalized_tick_rate = m_movement_tick_rate / tick_scale;
	float increment = GetFrameTime() / normalized_tick_rate;

	m_position_render.x += direction.x > 0 ? std::min(direction_normalized.x * increment, direction.x) :
	                                         std::max(direction_normalized.x * increment, direction.x);
	m_position_render.y += direction.y > 0 ? std::min(direction_normalized.y * increment, direction.y) :
	                                         std::max(direction_normalized.y * increment, direction.y);
}

BoundingBox entity::get_active_bounding_box()
{
	const BoundingBox model_bbox = m_model.get_active_animation()->m_bounding_boxes[m_model.m_animation_current_frame];
	BoundingBox bbox = model_bbox;
	bbox.min.x = model_bbox.min.x;
	bbox.min.y = model_bbox.min.z;
	bbox.min.z = model_bbox.min.y;
	bbox.max.x = model_bbox.max.x;
	bbox.max.y = model_bbox.max.z;
	bbox.max.z = model_bbox.max.y;
	bbox.min = Vector3Add(bbox.min, m_position_render);
	bbox.max = Vector3Add(bbox.max, m_position_render);
	return bbox;
}

float entity::get_attack_distance(entity &target) const
{
	/* Return distance to bounding box. */
	const BoundingBox bbox = target.get_active_bounding_box();

	const float x = Clamp(m_position_logic.x, bbox.min.x, bbox.max.x);
	const float y = Clamp(m_position_logic.y, bbox.min.y, bbox.max.y);

	const float dx = m_position_logic.x - x;
	const float dy = m_position_logic.y - y;

	return sqrtf(dx * dx + dy * dy);
}

tile entity::get_closest_tile(entity &target) const
{
	/* Return distance to bounding box. */
	const BoundingBox bbox = target.get_active_bounding_box();

	const float x = Clamp(m_position_logic.x, bbox.min.x, bbox.max.x);
	const float y = Clamp(m_position_logic.y, bbox.min.y, bbox.max.y);

	const float dx = m_position_logic.x - x;
	const float dy = m_position_logic.y - y;

	return { m_position_logic.x - (i32)dx, m_position_logic.y - (i32)dy };
}

static void matrix_to_rotation(Matrix m, Vector3 &axis, float &angle)
{
	Quaternion q = QuaternionFromMatrix(m);
	QuaternionToAxisAngle(q, &axis, &angle);
	angle *= RAD2DEG;
}

void entity::draw(Camera3D &camera)
{
	/* Positioning. */
	Vector3 position = { m_position_render.x, m_position_render.y, 0.0f };
	Vector3 move_target = m_target_render;

	/* Directions. */
	Vector3 eye_direction = Vector3Transform({ 0.0f, 0.0f, 1.0f }, m_model_rotation);
	Vector3 target_look_direction = Vector3Normalize(move_target - position);
	if (m_target != nullptr)
	{
		Vector3 target_position = { m_target->m_position_render.x, m_target->m_position_render.y, 0.0f };
		target_look_direction = Vector3Normalize(target_position - position);
	}
	float dot = Vector3DotProduct(eye_direction, target_look_direction);

	/* Work out rotations. */
	if (m_target == nullptr && Vector3Length(move_target - position) <= 0.05f)
	{
		/* Do nothing when we're close. */
	}
	else if (fabsf(dot - 1.0f) < 0.000001f)
	{
		/* Parallel. Do nothing. */
	}
	else if (fabs(dot + 1.0f) < 0.000001f)
	{
		/* Antiparallel. Reverse. */
		Vector3 up = { 0.0f, 0.0f, 1.0f };
		float max_angle = GetFrameTime() * TURN_TICK_RATE * PI;
		m_model_rotation = MatrixMultiply(m_model_rotation, MatrixRotate(up, max_angle));
	}
	else
	{
		/* Angle. */
		Vector3 rotation_axis = Vector3CrossProduct(eye_direction, target_look_direction);
		float rotation_angle = acosf(Clamp(dot, -1.0f, 1.0f));
		float max_angle = GetFrameTime() * TURN_TICK_RATE * PI;
		rotation_angle = Clamp(rotation_angle, -max_angle, max_angle);
		m_model_rotation = MatrixMultiply(m_model_rotation, MatrixRotate(rotation_axis, rotation_angle));
	}

	/* Convert to axis/angle so we don't yet have to write a custom draw path for models. */
	Vector3 rotation_axis;
	float rotation_angle;
	matrix_to_rotation(m_model_rotation, rotation_axis, rotation_angle);

	/* Draw. */
	Vector3 draw_position = { m_position_render.x, m_position_render.y, 0.0f };
	Vector3 draw_scale = { 1.0f, 1.0f, 1.0f };
	BeginMode3D(camera);
	{
		/* Draw entity. */
		std::shared_ptr<animation_> active_animation = m_model.get_active_animation();
		draw_model_mesh(active_animation->m_model, m_model.m_animation_current_frame, draw_position, rotation_axis,
		                rotation_angle, draw_scale, WHITE);

		/* Draw bounding box. */
		if (m_draw_bbox)
		{
			const BoundingBox bbox_ = active_animation->m_bounding_boxes[m_model.m_animation_current_frame];
			BoundingBox bbox = bbox_;
			bbox.min.x = bbox_.min.x;
			bbox.min.y = bbox_.min.z;
			bbox.min.z = bbox_.min.y;
			bbox.max.x = bbox_.max.x;
			bbox.max.y = bbox_.max.z;
			bbox.max.z = bbox_.max.y;

			const float width = bbox.max.x - bbox.min.x;
			const float height = bbox.max.z - bbox.min.z;
			const float length = bbox.max.y - bbox.min.y;

			/* (TODO, thoave01): We need to account for rotation. */
			Vector3 bbox_position = Vector3Add(draw_position, { 0.0f, 0.0f, height / 2.0f });
			bbox_position = Vector3Add(bbox_position, { 0.0f, 0.0f, bbox.min.z });
			DrawCubeWires(bbox_position, width, length, height, BLUE); /* YZ flipped. */
		}
	}
	EndMode3D();
}

void entity::set_action(action_data action_data)
{
	switch (action_data.action)
	{
	case action::IDLE:
		idle();
		break;
	case action::MOVE:
		move(action_data.MOVE.end);
		break;
	case action::ATTACK:
		attack(action_data.ATTACK.entity);
		break;
	}

	/* (TODO, thoave01): We have to set action after, so we know what the previous action was. */
	m_current_action = action_data.action;
}

void entity::reset()
{
	/* Movement. */
	std::deque<tile>().swap(m_path_logic);
	std::deque<tile>().swap(m_path_render);

	/* Combat. */
	m_target = nullptr;
}

void entity::idle()
{
	reset();
	if (m_model.m_active_animation_id != animation_id::BOSS_IDLE)
	{
		m_model.set_active_animation(animation_id::PLAYER_IDLE);
	}
}

void entity::move(tile end)
{
	if (end == m_position_logic)
	{
		/* Do nothing. */
	}
	else if (m_current_action == action::MOVE)
	{
		/* Empty current paths. */
		std::deque<tile>().swap(m_path_logic);
		std::deque<tile>().swap(m_path_render);

		m_map.generate_path(m_position_logic, end, m_path_logic);

		m_target_logic = m_path_logic.front();
		m_path_logic.pop_front();
		m_path_render.push_back(m_target_logic);
	}
	else
	{
		reset();
		m_map.generate_path(m_position_logic, end, m_path_logic);

		m_target_logic = m_path_logic.front();
		m_path_logic.pop_front();
		m_path_render.push_back(m_target_logic);

		if (m_model.m_active_animation_id != animation_id::BOSS_IDLE)
		{
			animation_id id = m_running ? animation_id::PLAYER_RUN : animation_id::PLAYER_WALK;
			m_model.set_active_animation(id);
		}
	}
}

void entity::attack(entity &target)
{
	m_path_logic.clear();
	m_target = &target;

	if (get_attack_distance(target) <= m_current_attack_range)
	{
		m_model.set_active_animation(animation_id::PLAYER_ATTACK);
	}
	else
	{
		/* (TODO, thoave01): Not a good way to pretend that we're walking? */
		animation_id id = m_running ? animation_id::PLAYER_RUN : animation_id::PLAYER_WALK;
		m_model.set_active_animation(id);
	}

	/* Current attack state. Let cooldown persist. */
	m_current_attack_cast_time = 0.0f;
}

attack::attack(entity &source, entity &target)
    : m_source_entity(source)
    , m_target_entity(target)
    , m_position_render(source.m_position_render)
{
	/* Set position to be at the source. */
	const BoundingBox bbox = source.m_model.get_active_animation()->m_bounding_boxes[0];
	const float height = bbox.max.y - bbox.min.y;
	m_position_render.z = 2.0f * (height / 3.0f);

	/* Set attack model. */
	m_model.load(model_id::WIND_BLAST);
	m_model.set_active_animation(animation_id::WIND_BLAST_FLY);
}

bool attack::tick_render()
{
	/* Update animation. */
	m_model.tick_render();

	/* Find target. */
	const BoundingBox target_bbox = m_target_entity.m_model.get_active_animation()->m_bounding_boxes[0];
	const float height = 2.0f * (target_bbox.max.y - target_bbox.min.y) / 3.0f;
	const Vector3 target = { m_target_entity.m_position_render.x, m_target_entity.m_position_render.y, height };

	/* Move towards target. */
	/* (TODO, thoave01): Constant speed, not decreasing as we get close. */
	const Vector3 direction = Vector3Normalize(target - m_position_render);
	const Vector3 move = Vector3Scale(direction, GetFrameTime() * ATTACK_TICK_RATE);
	m_position_render += move;

	/* Trigger the attack when we hit the target. */
	if (Vector3Length(target - m_position_render) <= 0.05f)
	{
		return true;
	}

	return false;
}

static Matrix matrix_rotation_glb()
{
	/* GLB is +Y up, +Z forward, -X right. We're +Z up. */
	return MatrixRotateX(90.0f * DEG2RAD);
}

void attack::draw(Camera3D &camera)
{
	/* Positioning. */
	Vector3 position = { m_position_render.x, m_position_render.y, 0.0f };
	Vector3 target_position = { m_target_entity.m_position_render.x, m_target_entity.m_position_render.y, 0.0f };

	/* Directions. */
	Matrix model_rotation = matrix_rotation_glb();
	Vector3 eye_direction = Vector3Transform({ 0.0f, 0.0f, 1.0f }, model_rotation);
	Vector3 target_look_direction = Vector3Normalize(target_position - position);
	float dot = Vector3DotProduct(eye_direction, target_look_direction);

	/* Work out rotations. */
	if (fabsf(dot - 1.0f) < 0.000001f)
	{
		/* Parallel. Do nothing. */
	}
	else if (fabs(dot + 1.0f) < 0.000001f)
	{
		/* Antiparallel. Reverse. */
		Vector3 up = { 0.0f, 0.0f, 1.0f };
		model_rotation = MatrixMultiply(model_rotation, MatrixRotate(up, PI));
	}
	else
	{
		/* Angle. */
		Vector3 rotation_axis = Vector3CrossProduct(eye_direction, target_look_direction);
		float rotation_angle = acosf(Clamp(dot, -1.0f, 1.0f));
		model_rotation = MatrixMultiply(model_rotation, MatrixRotate(rotation_axis, rotation_angle));
	}

	/* Convert to axis/angle so we don't yet have to write a custom draw path for models. */
	Vector3 rotation_axis;
	float rotation_angle;
	matrix_to_rotation(model_rotation, rotation_axis, rotation_angle);

	/* Draw. */
	Vector3 draw_position = m_position_render;
	Vector3 draw_scale = { 1.0f, 1.0f, 1.0f };
	BeginMode3D(camera);
	{
		/* Draw entity. */
		std::shared_ptr<animation_> active_animation = m_model.get_active_animation();
		draw_model_mesh(active_animation->m_model, m_model.m_animation_current_frame, draw_position, rotation_axis,
		                rotation_angle, draw_scale, RAYWHITE);
	}
	EndMode3D();
}
