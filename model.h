#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include "raylib.h"
#include "raymath.h"
#pragma clang diagnostic pop

enum class model_id
{
	FIRST,

	PLAYER = FIRST,
	BOSS,

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

	COUNT,
};

class animation_
{
public:
	animation_() = default;
	~animation_() = default;

	void load(animation_id id);

	animation_ &operator=(const animation_ &animation_) = delete;
	animation_(const animation_ &animation_) = delete;

	const char *m_file_path = {};
	animation_id m_animation_id = {};
	Model m_model = {};
	std::vector<BoundingBox> m_bounding_boxes = {};

	bool m_loaded = false;
};

class animation_cache
{
public:
	animation_cache() = default;
	~animation_cache() = default;

	animation_cache &operator=(const animation_cache &animation_cache) = delete;
	animation_cache(const animation_cache &animation_cache) = delete;

	void load();

	std::unordered_map<animation_id, std::shared_ptr<animation_>> m_animation_cache = {};
	std::unordered_map<model_id, std::vector<animation_id>> m_model_animation_ids = {};
};

class model
{
public:
	model() = default;
	~model() = default;

	model &operator=(const model &model) = delete;
	model(const model &model) = delete;

	void load(animation_cache &cache, model_id id);

	bool m_loaded = false;

	model_id m_model_id;
	std::unordered_map<animation_id, std::shared_ptr<animation_>> m_animations = {};
};
