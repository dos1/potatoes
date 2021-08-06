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

	struct Character *pyry[8], *buzie[8], *buzia;
	int mode[8];
	int hovered;
	double timer;

	struct Frame {
		int frame;
		int potato;
		bool alternative;
	} frame[8];

	ALLEGRO_SAMPLE* sample[8][5];
	ALLEGRO_SAMPLE_INSTANCE* song[8][5];

	ALLEGRO_MIXER* mixer[8];

	ALLEGRO_BITMAP *scene, *light, *mic;

	ALLEGRO_FONT* font;
};

int Gamestate_ProgressCount = 71; // number of loading steps as reported by Gamestate_Load; 0 when missing

static void MixerPostprocess(void* buffer, unsigned int samples, void* userdata) {
	struct Frame* frame = userdata;
	float* buf = buffer;
	float sum = 0.0;
	for (unsigned int i = 0; i < samples; i += 2) {
		sum += fabsf(buf[i]);
	}
	sum = pow(sum / 10.0, 2);
	frame->frame = sum;
	if (frame->frame > 3) {
		frame->frame = 3;
	}
	frame->frame = 3 - frame->frame;
	if (frame->frame == 0) {
		frame->alternative = !frame->alternative;
	}
	//printf("potato %d sum %f\n", frame->potato, sum);
}

static void UpdateHover(struct Game* game, struct GamestateResources* data) {
	data->hovered = -1;
	for (int i = 0; i < 8; i++) {
		if (IsOnCharacter(game, data->pyry[i], game->data->mouseX * game->viewport.width, game->data->mouseY * game->viewport.height, true)) {
			data->hovered = i;
		}
	}
}

void Gamestate_Logic(struct Game* game, struct GamestateResources* data, double delta) {
	// Here you should do all your game logic as if <delta> seconds have passed.
	if (data->timer > 0) {
		data->timer -= delta;
		if (data->timer <= 0) {
			data->hovered = -1;
		}
	}
}

void Gamestate_Draw(struct Game* game, struct GamestateResources* data) {
	// Draw everything to the screen here.

	float time = al_get_sample_instance_position(data->song[0][0]) / (float)al_get_sample_instance_length(data->song[0][0]) * 8;

	al_draw_rotated_bitmap(data->light, 0, 0, 445, 160, cos(al_get_time() * ALLEGRO_PI / 2.0) / 32.0, ALLEGRO_FLIP_HORIZONTAL);
	al_draw_rotated_bitmap(data->light, al_get_bitmap_width(data->light), 0, 1640, 160, sin(al_get_time() * ALLEGRO_PI / 2.0) / 32.0, 0);
	al_draw_bitmap(data->scene, 0, 0, 0);

	ALLEGRO_TRANSFORM transform, orig = *al_get_current_transform();

	//PrintConsole(game, "%f", time);

	for (int i = 0; i < 8; i++) {
		float facescale = 0.9;
		if (i < 4) {
			facescale = 0.75;
		}

		int foffsetx, foffsety;
		bool fflip = false;
		switch (i) {
			case 0:
				foffsetx = 0;
				foffsety = 15;
				break;
			case 1:
				foffsetx = 40;
				foffsety = 25;
				break;
			case 2:
				foffsetx = 20;
				foffsety = 10;
				break;
			case 3:
				foffsetx = 10;
				foffsety = 30;
				break;
			case 4:
				foffsetx = 5;
				foffsety = 5;
				break;
			case 5:
				foffsetx = -45;
				foffsety = 25;
				fflip = true;
				break;
			case 6:
				foffsetx = 20;
				foffsety = 60;
				break;
			case 7:
				foffsetx = 10;
				foffsety = 50;
				fflip = true;
				break;
			default:
				foffsetx = 0;
				foffsety = 0;
				break;
		}

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
			ALLEGRO_BITMAP* buzia = data->buzie[i]->spritesheets->frames[data->frame[i].frame].bitmap;
			if (data->frame[i].alternative) {
				buzia = data->buzie[i]->spritesheets->next->frames[data->frame[i].frame].bitmap;
			}
			DrawCenteredScaled(buzia, foffsetx, foffsety, facescale, facescale, fflip ? ALLEGRO_FLIP_HORIZONTAL : 0);

			al_use_transform(&orig);
		} else {
			al_use_transform(&orig);
			DrawCharacter(game, data->pyry[i]);
			DrawCenteredScaled(data->buzie[i]->frame->bitmap, GetCharacterX(game, data->pyry[i]) + foffsetx, GetCharacterY(game, data->pyry[i]) + foffsety, facescale, facescale, fflip ? ALLEGRO_FLIP_HORIZONTAL : 0);
		}

		int offsetx, offsety;
		bool flip = false;
		switch (i) {
			case 0:
				offsetx = 80;
				offsety = 40;
				break;
			case 1:
				offsetx = 100;
				offsety = 100;
				break;
			case 2:
				offsetx = 90;
				offsety = 60;
				break;
			case 3:
				offsetx = 55;
				offsety = 100;
				break;
			case 4:
				offsetx = 80;
				offsety = 110;
				break;
			case 5:
				offsetx = -90;
				offsety = 80;
				flip = true;
				break;
			case 6:
				offsetx = 75;
				offsety = 90;
				break;
			case 7:
				offsetx = -60;
				offsety = 80;
				flip = true;
				break;
			default:
				offsetx = 0;
				offsety = 0;
				break;
		}

		DrawCenteredScaled(data->mic, GetCharacterX(game, data->pyry[i]) + offsetx, GetCharacterY(game, data->pyry[i]) + offsety, 0.2, 0.2, flip ? ALLEGRO_FLIP_HORIZONTAL : 0);
	}

	al_use_transform(&orig);

	for (int i = 0; i < 8; i++) {
		if (data->mode[i] >= 0 && data->hovered == i) {
			al_draw_text(data->font, al_map_rgb(255, 255, 255), (game->data->mouseX + 0.02) * game->viewport.width + 3, (game->data->mouseY + 0.02) * game->viewport.height + 3, ALLEGRO_ALIGN_LEFT, PunchNumber(game, "X", 'X', data->mode[i] + 1));
			al_draw_text(data->font, al_map_rgb(0, 0, 0), (game->data->mouseX + 0.02) * game->viewport.width, (game->data->mouseY + 0.02) * game->viewport.height, ALLEGRO_ALIGN_LEFT, PunchNumber(game, "X", 'X', data->mode[i] + 1));
		}
	}

	if (game->config.mute) {
		al_draw_text(data->font, al_map_rgb(0, 0, 0), 10, 10, ALLEGRO_ALIGN_LEFT, "muted!!1");
	}

#ifndef ALLEGRO_ANDROID
#ifndef __SWITCH__
#ifndef __vita__
	if (game->config.fullscreen) {
		al_draw_text(data->font, al_map_rgb(0, 0, 0), game->viewport.width - 50, 20, ALLEGRO_ALIGN_RIGHT, "X");
	}
#endif
#endif
#endif
}

void Gamestate_ProcessEvent(struct Game* game, struct GamestateResources* data, ALLEGRO_EVENT* ev) {
	// Called for each event in Allegro event queue.
	// Here you can handle user input, expiring timers etc.
	if ((ev->type == ALLEGRO_EVENT_KEY_DOWN) && (ev->keyboard.keycode == ALLEGRO_KEY_ESCAPE)) {
		UnloadCurrentGamestate(game); // mark this gamestate to be stopped and unloaded
		// When there are no active gamestates, the engine will quit.
	}

	if (ev->type == ALLEGRO_EVENT_TOUCH_BEGIN && !ev->touch.primary && data->hovered != -1 && data->mode[data->hovered] != -1) {
		al_set_sample_instance_gain(data->song[data->hovered][data->mode[data->hovered]], 0.0);
		data->mode[data->hovered] = -1;
		data->hovered = -1;
		return;
	}

	if (ev->type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN || ev->type == ALLEGRO_EVENT_MOUSE_AXES) {
		game->data->mouseX = Clamp(0, 1, (ev->mouse.x - game->clip_rect.x) / (double)game->clip_rect.w);
		game->data->mouseY = Clamp(0, 1, (ev->mouse.y - game->clip_rect.y) / (double)game->clip_rect.h);
		UpdateHover(game, data);
		data->timer = 0;
	}
	if (ev->type == ALLEGRO_EVENT_TOUCH_BEGIN) {
		game->data->mouseX = Clamp(0, 1, (ev->touch.x - game->clip_rect.x) / (double)game->clip_rect.w);
		game->data->mouseY = Clamp(0, 1, (ev->touch.y - game->clip_rect.y) / (double)game->clip_rect.h);
		UpdateHover(game, data);
		data->timer = 3;
	}

#ifndef ALLEGRO_ANDROID
#ifndef __SWITCH__
#ifndef __vita__
	if ((ev->type == ALLEGRO_EVENT_TOUCH_BEGIN && game->config.fullscreen) ||
		(ev->type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && game->config.fullscreen)) {
		if ((game->data->mouseX >= 0.93) && (game->data->mouseY <= 0.1)) {
			UnloadAllGamestates(game);
			return;
		}
	}
#endif
#endif
#endif

	if (ev->type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN || ev->type == ALLEGRO_EVENT_TOUCH_BEGIN) {
		if (data->hovered >= 0) {
			if (data->mode[data->hovered] >= 0) {
				al_set_sample_instance_gain(data->song[data->hovered][data->mode[data->hovered]], 0.0);
			}

			data->mode[data->hovered]++;
			if (data->mode[data->hovered] > 4) {
				data->mode[data->hovered] = -1;
			}

			if (ev->type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && ev->mouse.button > 1) {
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

	data->mic = al_load_bitmap(GetDataFilePath(game, "mic.png"));
	progress(game);

	data->buzia = CreateCharacter(game, "face");
	RegisterSpritesheet(game, data->buzia, "1");
	RegisterSpritesheet(game, data->buzia, "2");
	LoadSpritesheets(game, data->buzia, progress);
	progress(game);

	for (int i = 0; i < 8; i++) {
		data->buzie[i] = CreateCharacter(game, "face");
		data->buzie[i]->shared = true;
		data->buzie[i]->spritesheets = data->buzia->spritesheets;
		SelectSpritesheet(game, data->buzie[i], "1");
		progress(game);

		data->pyry[i] = CreateCharacter(game, "potato");
		RegisterSpritesheet(game, data->pyry[i], PunchNumber(game, "X", 'X', i));
		LoadSpritesheets(game, data->pyry[i], progress);

		data->mixer[i] = al_create_mixer(al_get_voice_frequency(game->audio.v), ALLEGRO_AUDIO_DEPTH_FLOAT32, ALLEGRO_CHANNEL_CONF_2);
		al_attach_mixer_to_mixer(data->mixer[i], game->audio.music);
		al_set_mixer_postprocess_callback(data->mixer[i], MixerPostprocess, &data->frame[i]);
		progress(game);

		for (int j = 0; j < 5; j++) {
			data->sample[i][j] = al_load_sample(GetDataFilePath(game, PunchNumber(game, PunchNumber(game, "pX/Y.flac", 'X', i), 'Y', j + 1)));
			data->song[i][j] = al_create_sample_instance(data->sample[i][j]);
			al_attach_sample_instance_to_mixer(data->song[i][j], data->mixer[i]);
			al_set_sample_instance_playmode(data->song[i][j], ALLEGRO_PLAYMODE_LOOP);
			al_set_sample_instance_pan(data->song[i][j], -0.375 + (i % 4) * 0.25);

			if (al_get_sample_instance_length(data->song[i][j]) < 392020) {
				PrintConsole(game, "TOO SHORT i %d j %d length %d", i, j + 1, al_get_sample_instance_length(data->song[i][j]));
				//al_rest(1.0);
			}
			al_set_sample_instance_length(data->song[i][j], 391000);

			progress(game);
		}
	}

	data->font = al_load_font(GetDataFilePath(game, "fonts/ComicNeue-Bold.ttf"), 96, 0);
	progress(game);

	return data;
}

void Gamestate_Unload(struct Game* game, struct GamestateResources* data) {
	// Called when the gamestate library is being unloaded.
	// Good place for freeing all allocated memory and resources.

	for (int i = 0; i < 8; i++) {
		DestroyCharacter(game, data->pyry[i]);
		DestroyCharacter(game, data->buzie[i]);
		for (int j = 0; j < 5; j++) {
			al_destroy_sample_instance(data->song[i][j]);
			al_destroy_sample(data->sample[i][j]);
		}
		al_destroy_mixer(data->mixer[i]);
	}
	DestroyCharacter(game, data->buzia);
	al_destroy_bitmap(data->scene);
	al_destroy_bitmap(data->light);
	al_destroy_bitmap(data->mic);
	al_destroy_font(data->font);
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
	data->timer = 0;
	data->hovered = -1;
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
