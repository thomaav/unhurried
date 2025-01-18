#include <cassert>

#include "external/cgltf.h"

#include "entity.h"
#include "model.h"
#include "types.h"

void animation::load(animation_ref ref)
{
	/* Load animation. */
	m_file_path = ref.m_path;
	m_animation_id = ref.m_id;
	m_model = LoadModel(m_file_path);

	/* (TODO, thoave01): Temporary just for animation. */
	/* Re-load the model, extracting animation lengths as `extras` of the nodes. */
	{
		/* Load file. */
		int file_size = 0;
		unsigned char *file_data = LoadFileData(m_file_path, &file_size);
		assert(nullptr != file_data);

		/* Parse glTF. */
		cgltf_options gltf_options = {};
		cgltf_data *gltf_data = nullptr;
		cgltf_result result = cgltf_parse(&gltf_options, file_data, file_size, &gltf_data);
		assert(result == cgltf_result_success);
		assert(gltf_data != nullptr);

		/* Extract animation lengths. */
		assert((int)gltf_data->nodes_count == m_model.meshCount);
		for (u32 node_idx = 0; node_idx < gltf_data->nodes_count; ++node_idx)
		{
			if (nullptr != gltf_data->nodes[node_idx].extras.data)
			{
				const char *extras = (const char *)gltf_data->nodes[node_idx].extras.data;
				m_frame_lengths.push_back(strtof(extras, nullptr));
			}
		}

		UnloadFileData(file_data);
	}

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

float animation::get_length() const
{
	assert(m_loaded);

	float frame_sum = 0.0f;
	for (const float frame_length : m_frame_lengths)
	{
		frame_sum += frame_length;
	}
	return frame_sum;
}

void animation_cache::add_model_animation(model_id model_id, animation_id animation_id, const char *path)
{
	/* Initialize model if first animation. */
	if (m_model_animation_ids.find(model_id) == m_model_animation_ids.end())
	{
		m_model_animation_ids[model_id] = {};
	}

	/* Add animation. */
	m_model_animation_ids[model_id].push_back({ animation_id, path });
}

void animation_cache::load()
{
	if (m_loaded)
	{
		constexpr bool animation_cache_already_loaded = false;
		assert(animation_cache_already_loaded);
	}

	// clang-format off
	add_model_animation(model_id::PLAYER, animation_id::PLAYER_IDLE, "assets/models/player_idle.gltf");
	add_model_animation(model_id::PLAYER, animation_id::PLAYER_WALK, "assets/models/player_walk.gltf");
	add_model_animation(model_id::PLAYER, animation_id::PLAYER_RUN, "assets/models/player_run.gltf");
	add_model_animation(model_id::PLAYER, animation_id::PLAYER_ATTACK, "assets/models/player_attack.gltf");

	add_model_animation(model_id::BOSS, animation_id::BOSS_IDLE, "assets/models/boss_idle.gltf");
	add_model_animation(model_id::BOSS, animation_id::BOSS_ATTACK, "assets/models/boss_attack.gltf");

	add_model_animation(model_id::WIND_BLAST, animation_id::WIND_BLAST_FLY, "assets/models/wind_blast_fly.gltf");
	add_model_animation(model_id::WIND_BLAST, animation_id::WIND_BLAST_EXPLODE, "assets/models/wind_blast_fly.gltf");
	// clang-format on

	/* Load animations from disk. */
	for (const auto &[model_id, animation_refs] : m_model_animation_ids)
	{
		for (const animation_ref &ref : animation_refs)
		{
			/* Load animation into cache. */
			std::shared_ptr<animation> loaded_animation = std::make_shared<animation>();
			loaded_animation->load(ref);
			m_animation_cache[ref.m_id] = loaded_animation;
		}
	}

	m_loaded = true;
}

/* (TODO, thoave01): typedef to animation_ref? */
std::shared_ptr<animation> animation_cache::get_animation(animation_id id)
{
	assert(m_loaded);
	return m_animation_cache[id];
}

animation_cache model::m_animation_cache = {};
void model::load(model_id id)
{
	/* Don't allow reassignment. */
	assert(m_model_id == model_id::COUNT || m_model_id == id);

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

	for (const animation_ref &ref : m_animation_cache.m_model_animation_ids[id])
	{
		std::shared_ptr<animation> animation = m_animation_cache.m_animation_cache[ref.m_id];
		m_animations[ref.m_id] = animation;
	}

	m_loaded = true;
	m_model_id = id;
}

void model::set_active_animation(animation_id id)
{
	assert(m_loaded);
	m_active_animation_id = id;
	m_animation_current_frame = 0;
	m_animation_tick = 0;
}

std::shared_ptr<animation> model::get_active_animation()
{
	assert(m_active_animation_id != animation_id::COUNT);
	return m_animations[m_active_animation_id];
}

void model::tick_render()
{
	m_animation_tick += GetFrameTime();
	const float frame_length = get_active_animation()->m_frame_lengths[m_animation_current_frame];
	while (m_animation_tick > frame_length)
	{
		m_animation_tick -= frame_length;

		/* (TODO, thoave01): We should have an automatic default idle per model. */
		/* Some animations are not looped. */
		++m_animation_current_frame;
		if (m_animation_current_frame >= (u32)get_active_animation()->m_model.meshCount)
		{
			switch (get_active_animation()->m_animation_id)
			{
			case animation_id::PLAYER_ATTACK:
				set_active_animation(animation_id::PLAYER_IDLE);
				break;
			default:
				m_animation_current_frame = 0;
				break;
			}
		}
	}
}
