#include <cassert>

#include "model.h"
#include "types.h"

static const char *get_animation_id_path(animation_id id)
{
	switch (id)
	{
	case animation_id::PLAYER_IDLE:
		return "assets/models/idle.gltf";
	case animation_id::PLAYER_WALK:
		return "assets/models/walk.gltf";
	case animation_id::PLAYER_RUN:
		return "assets/models/run.gltf";
	case animation_id::PLAYER_ATTACK:
		return "assets/models/attack.gltf";
	case animation_id::BOSS_IDLE:
		return "assets/models/jad.gltf";
	default:
	{
		constexpr bool invalid_animation_id = false;
		assert(invalid_animation_id);
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
}

void model::load(animation_cache &cache, model_id id)
{
	if (m_loaded)
	{
		constexpr bool model_already_loaded = false;
		assert(model_already_loaded);
	}

	std::vector<animation_id> &animation_ids = cache.m_model_animation_ids[id];
	for (const animation_id &id : animation_ids)
	{
		std::shared_ptr<animation_> animation = cache.m_animation_cache[id];
		m_animations[id] = animation;
	}

	m_loaded = true;
}
