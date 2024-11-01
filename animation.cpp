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
	const char *path = get_animation_path(animation);
	animation_data ad = {};
	ad.m_file = path;
	ad.m_animation = animation;
	ad.m_model = LoadModel(path);
	return ad;
}
