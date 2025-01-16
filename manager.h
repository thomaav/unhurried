#pragma once

#include <deque>
#include <list>
#include <unordered_map>

#include "raylib.h"

#include "entity.h"
#include "map.h"
#include "menu.h"
#include "model.h"
#include "sprite.h"

extern int SCREEN_WIDTH;
extern int SCREEN_HEIGHT;

enum class context_type
{
	MENU,
	ENTITY_SELECTOR,
	GAME,
	PLAYGROUND,
};

/* (TODO, thoave01): Perhaps also add some sort of pre-loading/caching for models for animations? */
class asset_manager
{
public:
	void load_assets();
	sprite_animation &get_sprite_animation(sprite_type sprite);

	/* (TODO, thoave01): All loading should be done automatically from just a map of type -> path. */
	/* (TODO, thoave01): Path should parse more paths to find frames. */
	/* (TODO, thoave01): We should have a map type -> sprite animation as well. Like model animations. */
	sprite_animation m_click_yellow = { sprite_type::CLICK_YELLOW };
	sprite_animation m_click_red = { sprite_type::CLICK_RED };
	sprite_animation m_hitsplat_red = { sprite_type::HITSPLAT_RED };
	sprite_animation m_hitsplat_blue = { sprite_type::HITSPLAT_BLUE };

private:
};

class manager
{
public:
	manager() = default;
	~manager() = default;

	manager &operator=(const manager &manager) = delete;
	manager(const manager &manager) = delete;

	void run();

	void init();
	void set_map(map &map);
	void update_camera();

	void parse_events();

	void tick_attacks();
	void tick_sprites();
	void tick();

	void draw();

	void loop();
	void loop_menu_context();
	void init_entity_selector_context();
	void loop_entity_selector_context();
	void init_game_context();
	void loop_game_context();
	void init_playground();
	void loop_playground();

	void add_active_sprite_animation(sprite_type type, Vector2 position);
	void add_active_sprite_animation(sprite_type type, Vector3 position, Camera3D *camera);
	void add_active_sprite_animation(sprite_type type, entity &entity, Camera3D *camera);

	/* (TODO, thoave01): I don't really know how to do this correctly. */
	/* Assets. */
	asset_manager m_asset_manager = {};

	/* (TODO, thoave01): This wouldn't work with m_current_map. */
	entity *m_player = nullptr;
	entity *m_boss = nullptr;

	Camera3D m_root_camera = {};
	Camera3D m_camera = {};

	context_type m_current_context = context_type::MENU;
	model_id m_boss_model_id = model_id::BOSS;

	map *m_current_map = nullptr;
	menu *m_current_menu = nullptr;

	/* (TODO, thoave01): Temporary, should have a map/menu manager/loader? */
	map m_map = {};
	menu m_menu = {};

	/* (TODO, thoave01): Menu. */
	RenderTexture2D m_player_texture = {};
	entity m_player_idle = { { 0, 0 }, m_map, m_asset_manager, *this };
	Camera3D m_player_menu_camera = {};

	RenderTexture2D m_boss_texture = {};
	entity m_boss_idle = { { 0, 0 }, m_map, m_asset_manager, *this };
	Camera3D m_boss_menu_camera = {};

	/* Active sprites. */
	std::list<active_sprite_animation> m_active_sprite_animations = {};

	/* Combat. */
	std::list<attack> m_active_attacks = {};
	bool m_attack_select_active = false;

	/* Debug statistics. */
	std::vector<float> m_frame_times = {};

	/* Playground. */
	model m_pg_model = {};
};
