#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include "raylib.h"
#include "raymath.h"
#pragma clang diagnostic pop

#include "types.h"

enum class model_id
{
	FIRST,

	PLAYER = FIRST,
	BOSS,
	WIND_BLAST,

	COUNT,
};

enum class animation_id
{
	FIRST,

	/* model_id::PLAYER. */
	PLAYER_IDLE = FIRST,
	PLAYER_WALK,
	PLAYER_RUN,
	PLAYER_ATTACK,

	/* model_id::BOSS. */
	BOSS_IDLE,
	BOSS_WALK,
	BOSS_ATTACK,

	/* model_id::WIND_BLAST. */
	WIND_BLAST_FLY,
	WIND_BLAST_EXPLODE,

	COUNT,
};

struct animation_ref
{
	animation_id m_id;
	const char *m_path;
};

class animation
{
public:
	animation() = default;
	~animation() = default;

	void load(animation_ref id);
	float get_length() const;

	animation &operator=(const animation &animation_) = delete;
	animation(const animation &animation_) = delete;

	const char *m_file_path = {};
	animation_id m_animation_id = animation_id::COUNT;
	Model m_model = {};
	std::vector<BoundingBox> m_bounding_boxes = {};
	std::vector<float> m_frame_lengths = {};

	bool m_loaded = false;
};

class animation_cache
{
public:
	animation_cache() = default;
	~animation_cache() = default;

	animation_cache &operator=(const animation_cache &animation_cache) = delete;
	animation_cache(const animation_cache &animation_cache) = delete;

	void add_model_animation(model_id model_id, animation_id animation_id, const char *path);
	void load();
	std::shared_ptr<animation> get_animation(animation_id id);

	bool m_loaded = false;

	std::unordered_map<animation_id, std::shared_ptr<animation>> m_animation_cache = {};
	std::unordered_map<model_id, std::vector<animation_ref>> m_model_animation_ids = {};
};

class model
{
public:
	model() = default;
	~model() = default;

	model &operator=(const model &model) = delete;
	model(const model &model) = delete;

	/* Animation. */
	void load(model_id id);
	void set_active_animation(animation_id id);
	std::shared_ptr<animation> get_active_animation();

	/* Rendering. */
	void tick_render();

	/* Initialization data. */
	bool m_loaded = false;
	model_id m_model_id = model_id::COUNT;
	static animation_cache m_animation_cache;

	/* Animation data. */
	std::unordered_map<animation_id, std::shared_ptr<animation>> m_animations = {};
	animation_id m_active_animation_id = animation_id::COUNT;
	u32 m_animation_current_frame = 0;
	float m_animation_tick = 0.0f;
};
