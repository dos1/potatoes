/*! \file common.c
 *  \brief Common stuff that can be used by all gamestates.
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

#include "common.h"
#include <libsuperderpy.h>

bool GlobalEventHandler(struct Game* game, ALLEGRO_EVENT* ev) {
	if ((ev->type == ALLEGRO_EVENT_KEY_DOWN) && (ev->keyboard.keycode == ALLEGRO_KEY_F)) {
		game->config.fullscreen = !game->config.fullscreen;
		if (game->config.fullscreen) {
			SetConfigOption(game, "SuperDerpy", "fullscreen", "1");
		} else {
			SetConfigOption(game, "SuperDerpy", "fullscreen", "0");
		}
#ifdef ALLEGRO_ANDROID
		al_set_display_flag(game->display, ALLEGRO_FRAMELESS, game->config.fullscreen);
#endif
		al_set_display_flag(game->display, ALLEGRO_FULLSCREEN_WINDOW, game->config.fullscreen);
		SetupViewport(game, game->viewport_config);
		PrintConsole(game, "Fullscreen toggled");
	}

	if (ev->type == ALLEGRO_EVENT_MOUSE_AXES) {
		game->data->mouseX = Clamp(0, 1, (ev->mouse.x - game->_priv.clip_rect.x) / (double)game->_priv.clip_rect.w);
		game->data->mouseY = Clamp(0, 1, (ev->mouse.y - game->_priv.clip_rect.y) / (double)game->_priv.clip_rect.h);
	}

	return false;
}

struct CommonResources* CreateGameData(struct Game* game) {
	struct CommonResources* data = calloc(1, sizeof(struct CommonResources));
	return data;
}

void DestroyGameData(struct Game* game) {
	free(game->data);
}