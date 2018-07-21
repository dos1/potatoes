/*! \file empty.c
 *  \brief Empty gamestate.
 */
/*
 * Copyright (c) Sebastian Krzyszkowiak <dos@dosowisko.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "../common.h"
#include <libsuperderpy.h>

struct GamestateResources {
	// This struct is for every resource allocated and used by your gamestate.
	// It gets created on load and then gets passed around to all other function calls.

	struct Character *pyry[8], *buzie[8];
	int mode[8];
	int hovered;

	ALLEGRO_SAMPLE* sample[8][5];
	ALLEGRO_SAMPLE_INSTANCE* song[8][5];

	ALLEGRO_BITMAP *scene, *light;

	ALLEGRO_FONT* font;
};

int Gamestate_ProgressCount = 50; // number of loading steps as reported by Gamestate_Load; 0 when missing

void Gamestate_Logic(struct Game* game, struct GamestateResources* data, double delta) {
	// Here you should do all your game logic as if <delta> seconds have passed.
	//PrintConsole(game, "%f x %f", game->data->mouseX, game->data->mouseY);
	data->hovered = -1;
	for (int i = 0; i < 8; i++) {
		if (IsOnCharacter(game, data->pyry[i], game->data->mouseX * game->viewport.width, game->data->mouseY * game->viewport.height, true)) {
			data->hovered = i;
		}
	}

	/*
	PrintConsole(game, "START");
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 5; j++) {
			PrintConsole(game, "%d", al_get_sample_instance_position(data->song[i][j]));
		}
	}
	*/
}

void Gamestate_Draw(struct Game* game, struct GamestateResources* data) {
	// Draw everything to the screen here.

	float time = al_get_sample_instance_position(data->song[0][0]) / (float)al_get_sample_instance_length(data->song[0][0]) * 8;

	al_clear_to_color(al_map_rgb(255, 255, 255));
	al_draw_rotated_bitmap(data->light, 0, 0, 445, 160, cos(al_get_time() * ALLEGRO_PI / 2.0) / 32.0, ALLEGRO_FLIP_HORIZONTAL);
	al_draw_rotated_bitmap(data->light, al_get_bitmap_width(data->light), 0, 1640, 160, sin(al_get_time() * ALLEGRO_PI / 2.0) / 32.0, 0);
	al_draw_bitmap(data->scene, 0, 0, 0);

	ALLEGRO_TRANSFORM transform, orig = *al_get_current_transform();

	//PrintConsole(game, "%f", time);

	for (int i = 0; i < 8; i++) {
		data->pyry[i]->tint = i == data->hovered ? al_map_rgb_f(2, 2, 2) : al_map_rgb(255, 255, 255);

		data->pyry[i]->spritesheet->pivotX = 0.5;
		data->pyry[i]->spritesheet->pivotY = 0.5;

		if (data->mode[i] >= 0) {
			al_identity_transform(&transform);

			int x = i;
			if (x > 3) {
				x--;
			}

			al_translate_transform(&transform, -al_get_bitmap_width(data->pyry[i]->frame->bitmap) / 2.0, -al_get_bitmap_height(data->pyry[i]->frame->bitmap) / 2.0);
			//al_scale_transform(&transform, data->pyry[i]->scaleX, data->pyry[i]->scaleY);
			al_horizontal_shear_transform(&transform, sin(time * ALLEGRO_PI + ALLEGRO_PI * x + 0.075 * i) * 0.05);
			al_translate_transform(&transform, al_get_bitmap_width(data->pyry[i]->frame->bitmap) / 2.0, al_get_bitmap_height(data->pyry[i]->frame->bitmap) / 2.0);

			al_translate_transform(&transform, 0, -al_get_bitmap_height(data->pyry[i]->frame->bitmap));
			al_scale_transform(&transform, 1.0, 1.0 + cos(time * ALLEGRO_PI * 2 + ALLEGRO_PI * i) * 0.05);
			al_translate_transform(&transform, 0, al_get_bitmap_height(data->pyry[i]->frame->bitmap));

			al_translate_transform(&transform, GetCharacterX(game, data->pyry[i]), GetCharacterY(game, data->pyry[i]));

			al_compose_transform(&transform, &orig);
			al_use_transform(&transform);

			DrawCenteredTintedScaled(data->pyry[i]->frame->bitmap, data->pyry[i]->tint, 0, 0, data->pyry[i]->scaleX, data->pyry[i]->scaleY, 0);

			if (data->hovered == i) {
				al_use_transform(&orig);
				al_draw_text(data->font, al_map_rgb(255, 255, 255), game->data->mouseX * game->viewport.width + 3, game->data->mouseY * game->viewport.height + 3, ALLEGRO_ALIGN_LEFT, PunchNumber(game, "X", 'X', data->mode[i] + 1));
				al_draw_text(data->font, al_map_rgb(0, 0, 0), game->data->mouseX * game->viewport.width, game->data->mouseY * game->viewport.height, ALLEGRO_ALIGN_LEFT, PunchNumber(game, "X", 'X', data->mode[i] + 1));
			}
		} else {
			al_use_transform(&orig);
			DrawCharacter(game, data->pyry[i]);
		}
	}

	al_use_transform(&orig);
}

void Gamestate_ProcessEvent(struct Game* game, struct GamestateResources* data, ALLEGRO_EVENT* ev) {
	// Called for each event in Allegro event queue.
	// Here you can handle user input, expiring timers etc.
	if ((ev->type == ALLEGRO_EVENT_KEY_DOWN) && (ev->keyboard.keycode == ALLEGRO_KEY_ESCAPE)) {
		UnloadCurrentGamestate(game); // mark this gamestate to be stopped and unloaded
		// When there are no active gamestates, the engine will quit.
	}

	if (ev->type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
		if (data->hovered >= 0) {
			if (data->mode[data->hovered] >= 0) {
				al_set_sample_instance_gain(data->song[data->hovered][data->mode[data->hovered]], 0.0);
			}

			data->mode[data->hovered]++;
			if (data->mode[data->hovered] > 4) {
				data->mode[data->hovered] = -1;
			}

			if (ev->mouse.button > 1) {
				data->mode[data->hovered] = -1;
			}

			if (data->mode[data->hovered] >= 0) {
				al_set_sample_instance_gain(data->song[data->hovered][data->mode[data->hovered]], 1.0);
			}

			PrintConsole(game, "potato %d: sound %d", data->hovered, data->mode[data->hovered]);
		}
	}
}

void* Gamestate_Load(struct Game* game, void (*progress)(struct Game*)) {
	// Called once, when the gamestate library is being loaded.
	// Good place for allocating memory, loading bitmaps etc.
	//
	// NOTE: There's no OpenGL context available here. If you want to prerender something,
	// create VBOs, etc. do it in Gamestate_PostLoad.

	struct GamestateResources* data = calloc(1, sizeof(struct GamestateResources));

	data->scene = al_load_bitmap(GetDataFilePath(game, "scene.png"));
	progress(game); // report that we progressed with the loading, so the engine can move a progress bar

	data->light = al_load_bitmap(GetDataFilePath(game, "light.png"));
	progress(game);

	for (int i = 0; i < 8; i++) {
		data->pyry[i] = CreateCharacter(game, "potato");
		RegisterSpritesheet(game, data->pyry[i], PunchNumber(game, "X", 'X', i));
		LoadSpritesheets(game, data->pyry[i], progress);

		for (int j = 0; j < 5; j++) {
			data->sample[i][j] = al_load_sample(GetDataFilePath(game, PunchNumber(game, PunchNumber(game, "pX/Y.flac", 'X', i), 'Y', j + 1)));
			data->song[i][j] = al_create_sample_instance(data->sample[i][j]);
			al_attach_sample_instance_to_mixer(data->song[i][j], game->audio.music);
			al_set_sample_instance_playmode(data->song[i][j], ALLEGRO_PLAYMODE_LOOP);

			if (al_get_sample_instance_length(data->song[i][j]) < 392020) {
				PrintConsole(game, "i %d j %d length %d", i, j, al_get_sample_instance_length(data->song[i][j]));
			}
			al_set_sample_instance_length(data->song[i][j], 392020);

			progress(game);
		}
	}

	data->font = al_load_font(GetDataFilePath(game, "fonts/comicsans.ttf"), 64, 0);

	return data;
}

void Gamestate_Unload(struct Game* game, struct GamestateResources* data) {
	// Called when the gamestate library is being unloaded.
	// Good place for freeing all allocated memory and resources.
	free(data);
}

void Gamestate_Start(struct Game* game, struct GamestateResources* data) {
	// Called when this gamestate gets control. Good place for initializing state,
	// playing music etc.
	for (int i = 0; i < 4; i++) {
		SetCharacterPosition(game, data->pyry[i], 300 * i + 600, pow(sin(i / 7.0 * ALLEGRO_PI), 2) * 20 + 470, 0);
		data->pyry[i]->scaleX = 0.5;
		data->pyry[i]->scaleY = 0.5;
	}
	for (int i = 4; i < 8; i++) {
		SetCharacterPosition(game, data->pyry[i], 320 * (i - 4) + 500, pow(cos(i / 7.0 * ALLEGRO_PI), 2) * 50 + 630, 0);
		data->pyry[i]->scaleX = 0.666;
		data->pyry[i]->scaleY = 0.666;
	}

	for (int i = 0; i < 8; i++) {
		data->mode[i] = -1;
		for (int j = 0; j < 5; j++) {
			al_set_sample_instance_gain(data->song[i][j], 0.0);
			al_play_sample_instance(data->song[i][j]);
		}
	}
}

void Gamestate_Stop(struct Game* game, struct GamestateResources* data) {
	// Called when gamestate gets stopped. Stop timers, music etc. here.
}

// Optional endpoints:

void Gamestate_PostLoad(struct Game* game, struct GamestateResources* data) {
	// This is called in the main thread after Gamestate_Load has ended.
	// Use it to prerender bitmaps, create VBOs, etc.
}

void Gamestate_Pause(struct Game* game, struct GamestateResources* data) {
	// Called when gamestate gets paused (so only Draw is being called, no Logic nor ProcessEvent)
	// Pause your timers and/or sounds here.
}

void Gamestate_Resume(struct Game* game, struct GamestateResources* data) {
	// Called when gamestate gets resumed. Resume your timers and/or sounds here.
}

void Gamestate_Reload(struct Game* game, struct GamestateResources* data) {
	// Called when the display gets lost and not preserved bitmaps need to be recreated.
	// Unless you want to support mobile platforms, you should be able to ignore it.
}
