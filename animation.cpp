#include <cassert>

#include "animation.h"

static const char *get_animation_path(animation animation)
{
	switch (animation)
	{
	case animation::IDLE:
		return "assets/models/idle.glb";
	case animation::WALK:
		return "assets/models/walk.glb";
	case animation::RUN:
		return "assets/models/run.glb";
	case animation::ATTACK:
		return "assets/models/attack.glb";
	case animation::BOSS:
		return "assets/models/boss.glb";
	default:
		assert(false);
		break;
	}
}

animation_data get_animation(animation animation)
{
	/* Load model with animation meshes. */
	const char *path = get_animation_path(animation);
	animation_data ad = {};
	ad.m_file = path;
	ad.m_animation = animation;
	ad.m_model = LoadModel(path);

	/* Get bounding boxes. */
	ad.m_bounding_boxes.resize(ad.m_model.meshCount);
	for (int mesh_idx = 0; mesh_idx < ad.m_model.meshCount; ++mesh_idx)
	{
		Mesh mesh = ad.m_model.meshes[mesh_idx];
		ad.m_bounding_boxes[mesh_idx] = GetMeshBoundingBox(mesh);
	}

	return ad;
}
