#include <cassert>

#include "entity.h"
#include "model.h"
#include "types.h"

static const char *get_animation_id_path(animation_id id)
{
	switch (id)
	{
	case animation_id::PLAYER_IDLE:
		return "assets/models/player_idle.gltf";
	case animation_id::PLAYER_WALK:
		return "assets/models/player_walk.gltf";
	case animation_id::PLAYER_RUN:
		return "assets/models/player_run.gltf";
	case animation_id::PLAYER_ATTACK:
		return "assets/models/player_attack.gltf";
	case animation_id::BOSS_IDLE:
		return "assets/models/boss_idle.gltf";
	case animation_id::WIND_BLAST_FLY:
		return "assets/models/wind_blast_fly.gltf";
	case animation_id::WIND_BLAST_EXPLODE:
		/* (TODO, thoave01): Wrong. */
		return "assets/models/wind_blast_fly.gltf";
	default:
	{
		constexpr bool invalid_animation_id_path = false;
		assert(invalid_animation_id_path);
	}
	}
}

void animation_::load(animation_id id)
{
	/* Load animation. */
	m_file_path = get_animation_id_path(id);
	m_animation_id = id;
	m_model = LoadModel(m_file_path);

	/* Calculate animation bounding boxes, one per mesh. */
	m_bounding_boxes.resize(m_model.meshCount);
	for (int mesh_idx = 0; mesh_idx < m_model.meshCount; ++mesh_idx)
	{
		Mesh mesh = m_model.meshes[mesh_idx];
		m_bounding_boxes[mesh_idx] = GetMeshBoundingBox(mesh);
	}

	/* Done. */
	m_loaded = true;
}

void animation_cache::load()
{
	if (m_loaded)
	{
		constexpr bool animation_cache_already_loaded = false;
		assert(animation_cache_already_loaded);
	}

	/* Load animations from disk. */
	animation_id id = animation_id::FIRST;
	while (id != animation_id::COUNT)
	{
		/* Load animation into cache. */
		std::shared_ptr<animation_> animation = std::make_shared<animation_>();
		animation->load(id);
		m_animation_cache[id] = animation;

		/* (TODO, thoave01): Make enum class iterable. */
		id = animation_id((int)id + 1);
	}

	/* (TODO, thoave01): Make this part of the models themselves in config files. */
	std::vector<animation_id> player_animations = {};
	player_animations.push_back(animation_id::PLAYER_IDLE);
	player_animations.push_back(animation_id::PLAYER_WALK);
	player_animations.push_back(animation_id::PLAYER_RUN);
	player_animations.push_back(animation_id::PLAYER_ATTACK);
	m_model_animation_ids[model_id::PLAYER] = player_animations;

	std::vector<animation_id> boss_animations = {};
	boss_animations.push_back(animation_id::BOSS_IDLE);
	m_model_animation_ids[model_id::BOSS] = boss_animations;

	std::vector<animation_id> wind_blast_animations = {};
	wind_blast_animations.push_back(animation_id::WIND_BLAST_FLY);
	wind_blast_animations.push_back(animation_id::WIND_BLAST_EXPLODE);
	m_model_animation_ids[model_id::WIND_BLAST] = wind_blast_animations;

	m_loaded = true;
}

/* (TODO, thoave01): typedef to animation_ref? */
std::shared_ptr<animation_> animation_cache::get_animation(animation_id id)
{
	assert(m_loaded);
	return m_animation_cache[id];
}

animation_cache model::m_animation_cache = {};
void model::load(model_id id)
{
	/* (TODO, thoave01): Load entire cache first time it's used. */
	/* (TODO, thoave01): We need streaming. */
	if (!m_animation_cache.m_loaded)
	{
		m_animation_cache.load();
	}

	if (m_loaded)
	{
		constexpr bool model_already_loaded = false;
		assert(model_already_loaded);
	}

	std::vector<animation_id> &animation_ids = m_animation_cache.m_model_animation_ids[id];
	for (const animation_id &id : animation_ids)
	{
		std::shared_ptr<animation_> animation = m_animation_cache.m_animation_cache[id];
		m_animations[id] = animation;
	}

	m_loaded = true;
}

void model::set_active_animation(animation_id id)
{
	assert(m_loaded);
	m_active_animation_id = id;
	m_animation_current_frame = 0;
}

std::shared_ptr<animation_> model::get_active_animation()
{
	assert(m_active_animation_id != animation_id::COUNT);
	return m_animations[m_active_animation_id];
}

void model::tick_render()
{
	m_animation_tick += GetFrameTime();
	while (m_animation_tick > ANIMATION_TICK_RATE)
	{
		m_animation_tick -= ANIMATION_TICK_RATE;
		m_animation_current_frame = (m_animation_current_frame + 1) % get_active_animation()->m_model.meshCount;
	}
}
