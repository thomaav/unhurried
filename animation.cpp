#include "animation.h"

void switch_animation(const char *path, animation a, animation_data &ad)
{
	ad.m_file = path;
	ad.m_animation = a;
	ad.m_model = LoadModel(path);
}
