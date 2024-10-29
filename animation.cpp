#include <cassert>

#include "animation.h"

static const char *get_animation_path(animation animation)
{
	switch (animation)
	{
	case IDLE:
		return "assets/models/idle.glb";
	case WALK:
		return "assets/models/walk.glb";
	case ATTACK:
		return "assets/models/idle.glb";
	case BOSS:
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
