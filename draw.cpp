#include <cassert>

#include "draw.h"
#include "map.h"

void draw_printf_vector3(int x, int y, const char *name, Vector3 v3)
{
	draw_printf(x, y, "%s: %f %f %f", name, v3.x, v3.y, v3.z);
}

void draw_printf_vector2(int x, int y, const char *name, Vector3 v2)
{
	draw_printf(x, y, "%s: %f %f", name, v2.x, v2.y);
}

void draw_tile(i32 x, i32 y, Color color)
{
	/* Create tile quad. */
	Vector3 vertices[] = {
		{ (float)x, (float)y, MAP_HEIGHT },               //
		{ (float)x + 1.0f, (float)y, MAP_HEIGHT },        //
		{ (float)x, (float)y + 1.0f, MAP_HEIGHT },        //
		{ (float)x + 1.0f, (float)y + 1.0f, MAP_HEIGHT }, //
	};

	/* Draw quad. */
	DrawTriangleStrip3D(&vertices[0], 4, color);

	/* Draw outline. */
	DrawLine3D(vertices[0], vertices[1], WHITE);
	DrawLine3D(vertices[1], vertices[3], WHITE);
	DrawLine3D(vertices[3], vertices[2], WHITE);
	DrawLine3D(vertices[2], vertices[0], WHITE);
}

void draw_tile_overlay(i32 x, i32 y, Color color)
{
	/* Create tile quad. */
	Vector3 vertices[] = {
		{ (float)x, (float)y, -0.16f + 0.015f },               //
		{ (float)x + 1.0f, (float)y, -0.16f + 0.015f },        //
		{ (float)x, (float)y + 1.0f, -0.16f + 0.015f },        //
		{ (float)x + 1.0f, (float)y + 1.0f, -0.16f + 0.015f }, //
	};

	/* Draw quad. */
	DrawTriangleStrip3D(&vertices[0], 4, color);

	/* Draw outline. */
	DrawLine3D(vertices[0], vertices[1], BLACK);
	DrawLine3D(vertices[1], vertices[3], BLACK);
	DrawLine3D(vertices[3], vertices[2], BLACK);
	DrawLine3D(vertices[2], vertices[0], BLACK);
}

void draw_model_mesh(Model model, int mesh, Vector3 position, Vector3 axis, float angle, Vector3 scale, Color tint)
{
	Matrix matrix_scale = MatrixScale(scale.x, scale.y, scale.z);
	Matrix matrix_rotation = MatrixRotate(axis, angle * DEG2RAD);
	Matrix matrix_translation = MatrixTranslate(position.x, position.y, position.z);
	Matrix matrix_transform = MatrixMultiply(MatrixMultiply(matrix_scale, matrix_rotation), matrix_translation);
	model.transform = MatrixMultiply(model.transform, matrix_transform);

	Color color = model.materials[model.meshMaterial[mesh]].maps[MATERIAL_MAP_DIFFUSE].color;
	Color color_tint = WHITE;

	color_tint.r = (unsigned char)(((int)color.r * (int)tint.r) / 255);
	color_tint.g = (unsigned char)(((int)color.g * (int)tint.g) / 255);
	color_tint.b = (unsigned char)(((int)color.b * (int)tint.b) / 255);
	color_tint.a = (unsigned char)(((int)color.a * (int)tint.a) / 255);

	model.materials[model.meshMaterial[mesh]].maps[MATERIAL_MAP_DIFFUSE].color = color_tint;
	DrawMesh(model.meshes[mesh], model.materials[model.meshMaterial[mesh]], model.transform);
	model.materials[model.meshMaterial[mesh]].maps[MATERIAL_MAP_DIFFUSE].color = color;
}
