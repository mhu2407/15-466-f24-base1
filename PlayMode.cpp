#include "PlayMode.hpp"

//for the GL_ERRORS() macro:
#include "gl_errors.hpp"

//for glm::value_ptr() :
#include <glm/gtc/type_ptr.hpp>

#include <random>

#include "data_path.hpp"
#include "load_save_png.hpp"
#include <set>
#include <fstream>

PlayMode::PlayMode() {
	// this section of code is code is based on code from Jude Markabawi 
	// (sent in channel #game1-sprite-based of discord serverl 15-466/666/f24)
	// read in background
	std::string bg_filename = data_path("assets/output_of_asset_gen/background.bin");
	std::ifstream bg_file(bg_filename, std::ios_base::binary);
	bg_file.read(reinterpret_cast<char*> (ppu.background.data()), sizeof(ppu.background));

	// read in tile table
	std::string tile_table_filename = data_path("assets/output_of_asset_gen/tile_table.bin");
	std::ifstream tile_table_file(tile_table_filename, std::ios_base::binary);
	tile_table_file.read(reinterpret_cast<char*> (ppu.tile_table.data()), sizeof(ppu.tile_table));


	// read in palette table
	std::string palette_table_filename = data_path("assets/output_of_asset_gen/palette_table.bin");
	std::ifstream palette_table_file(palette_table_filename, std::ios_base::binary);
	palette_table_file.read(reinterpret_cast<char*> (ppu.palette_table.data()), sizeof(ppu.palette_table));

}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_LEFT) {
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT) {
			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_SPACE) {
			space.downs += 1;
			space.pressed = true;
			return true;
		}

	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_LEFT) {
			left.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT) {
			right.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_SPACE) {
			space.pressed = false;
			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed) {
	auto is_collision = [](glm::vec2 sprite, glm::vec2 sunscreen) {
		return ((sprite.x + 8 > sunscreen.x) && (sprite.x < sunscreen.x + 8)
		&& (sprite.y + 8 > sunscreen.y) && (sprite.y < sunscreen.y + 8));
	};

	constexpr float PlayerSpeed = 70.0f;
	constexpr float SunscreenSpeed = 400.0f;
	if (left.pressed) player_at.x -= PlayerSpeed * elapsed;
	if (right.pressed) player_at.x += PlayerSpeed * elapsed;

	constexpr float SpriteSpeed = 50.0f;
	for (uint32_t i = 0; i < 10; ++i) {
		// initialize positions of people
		if (player_at.x == 0) {
			sprite_positions[i].x = (float)i * 35;
			sprite_positions[i].y = (float)i * 10.0f;
			continue;
		}

		// move people, ensure positions don't go out of scope
		if ((i & 1)) {
			if (sprite_positions[i].x > 256) {
				sprite_positions[i].x = 0;
			}
			if (sprite_positions[i].y != 250) {
				sprite_positions[i].x += SpriteSpeed * elapsed;
			}
		} else {
			if (sprite_positions[i].x <= 0) {
				sprite_positions[i].x = 248;
			}
			if (sprite_positions[i].y != 250) {
				sprite_positions[i].x -= SpriteSpeed * elapsed;
			}
		}
	}
	
	// check if sunscreen is loaded in yet
	if (sunscreen_attribute & 120) {
		sprite_positions[10].x = player_at.x;
		sprite_positions[10].y = player_at.y;
		if (space.pressed) {
			sunscreen_attribute = 6;
		}
	} else if (sprite_positions[10].y <= 0) {
		// rehide sunscreen
		sunscreen_attribute = 14;
	} else {
		for (uint32_t i = 0; i < 10; ++i) {
			if (is_collision(sprite_positions[i], sprite_positions[10])) {
				// move sprite off screen
				sprite_positions[i].y = 250;
				// reset sunscreen
				sunscreen_attribute = 9;
				break;
			}
		}
		sprite_positions[10].y -= SunscreenSpeed * elapsed;
	}

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	space.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//--- set ppu state based on game state ---

	//player sprite:
	ppu.sprites[0].x = int8_t(player_at.x);
	ppu.sprites[0].y = int8_t(player_at.y);
	ppu.sprites[0].index = 49;
	ppu.sprites[0].attributes = 2;

	//people sprite
	for (uint32_t i = 1; i < 11; ++i) {
		ppu.sprites[i].x = int8_t(sprite_positions[i - 1].x);
		ppu.sprites[i].y = int8_t(sprite_positions[i - 1].y);
		ppu.sprites[i].index = 48;
		ppu.sprites[i].attributes = 1;
	}

	// sunscreen drop
	ppu.sprites[11].x = int8_t(sprite_positions[10].x);
	ppu.sprites[11].y = int8_t(sprite_positions[10].y);
	ppu.sprites[11].index = 49;
	ppu.sprites[11].attributes = 2;

	//--- actually draw ---
	ppu.draw(drawable_size);
}
