#pragma once
#include "headers/includes.h"
#include "headers/classes.h"
#include "headers/csgo.h"
#include "cfg/config.h"

namespace features
{
	//IMPROVEMENT:
	//obviously an in-cross triggerbot is pretty dogshit
	//go for a hitbox triggerbot or a raytracing triggerbot
	//the former is very easy, the latter is a little more difficult externally
	//both are a massive improvement

	void rect_esp( c_player enemy )
	{
		//visuals->enabled really just means rectangle enabled
		if ( !settings->visuals.enabled )
			return;

		//head pos
		vec3_t enemy_head_pos = enemy.get_bone_pos( 8 );
		enemy_head_pos.z += 10;
		vec2_t enemy_head_screen{};
		world_to_screen( enemy_head_pos, enemy_head_screen );

		//base origin
		vec3_t enemy_origin_pos = enemy.get_origin();
		vec2_t enemy_origin_screen{};
		world_to_screen( enemy_origin_pos, enemy_origin_screen );

		float h = enemy_origin_screen.y - enemy_head_screen.y;
		float w = (h / 2);

		draw_box( enemy_head_screen.x - (w / 2), enemy_head_screen.y, w, h, 1 );
	}

	void visuals()
	{
		if ( !settings->visuals.enabled
			&& !settings->misc.custom_flash
			&& !settings->visuals.recoil_crosshair 
			&& !settings->misc.radar )
			return;

		while ( true )
		{
			//fast-esp = no sleep
			if ( !settings->misc.multi_threads )
				Sleep( 1 );

			if ( !globals::b_run_features )
				continue;

			for ( int i = 0; i != 32; i++ )
			{
				c_player enemy = c_player( i );

				if ( !enemy.address
					|| enemy.get_dormant()
					|| enemy.get_team() == g_localplayer.get_team()
					|| enemy.address == g_localplayer.address
					|| !enemy.get_health() )
					continue;

				rect_esp( enemy );
			}
		}
	}

	//IMPROVEMENT:
	//currently mouse aimbot uses the angle calculated with the normal aimbot's smoothing
	//this converted smoothing, if at a higher value, can cause the aimbot to be inaccurate and aim around the player
	//this fixes itself while firing, but it still a noticable problem on very high smoothing values
	//to fix this - remove the smoothing in the normal aimbots calculations, and add your own mouse_event smoothing
	void mouse_aimbot( vec3_t aim_angle )
	{
		float sens = settings->aimbot.sens;

		vec3_t local_angle = g_engine.get_viewangles();

		float delta_x = local_angle.x - aim_angle.x;
		float delta_y = local_angle.y - aim_angle.y;

		vec3_t mouse_angle{};
		mouse_angle.x = (delta_x * -1.0f) / (0.022f * sens);
		mouse_angle.y = (delta_y) / (0.022f * sens);

		angle_sanitize( mouse_angle );

		LI_FN(mouse_event).cached()( 0x1, (int)mouse_angle.y, (int)mouse_angle.x, 0, 0 );
	}

	//IMPROVEMENT:
	//there is multiple things that can be improved with the targeting
	//you can change it from bones to hitboxes pretty easily, you can add settings for bones or for if it should use nearest bone or not
	//also you can easily add your own randomization with util::random_int / random_float
	//overall it's a decent aimbot, but try to make it your own
	void aimbot()
	{
		if ( !settings->aimbot.enabled )
			return;

		float aim_smooth = settings->aimbot.smooth * 10.0f;

		if ( !GetAsyncKeyState( settings->aimbot.key ) )
			return;

		if ( auto target = find_target() )
		{
			c_player entity = c_player( target );

			vec3_t local_eye = g_localplayer.get_local_eye();
			vec3_t local_viewangles = g_engine.get_viewangles();
			vec3_t target_position = entity.get_bone_pos( get_nearest_bone( entity ) );

			vec3_t aim_angle = calc_aim( local_eye, target_position, local_viewangles );
			aim_angle /= aim_smooth;
			aim_angle = local_viewangles += aim_angle;

			angle_sanitize( aim_angle );
			mouse_aimbot( aim_angle );
		}
	}

	void triggerbot()
	{
		if (!settings->triggerbot.enabled)
			return;

		if (auto target = find_target())
		{
			c_player entity = c_player(target);

			vec3_t local_eye = g_localplayer.get_local_eye();
			vec3_t local_viewangles = g_engine.get_viewangles();
			vec3_t target_position = entity.get_bone_pos(get_nearest_bone(entity));

			vec3_t aim_angle = calc_aim(local_eye, target_position, local_viewangles);
			aim_angle /= 10.f;
			aim_angle = local_viewangles += aim_angle;

			angle_sanitize(aim_angle);
			mouse_aimbot(aim_angle);
		}
		c_player enemy = g_localplayer.get_crosshair_entity();

		if (!enemy.address
			|| enemy.get_dormant()
			|| enemy.get_team() == g_localplayer.get_team()
			|| !enemy.get_health())
			return;


			LI_FN(mouse_event).cached()(MOUSEEVENTF_LEFTDOWN, NULL, NULL, NULL, NULL);
			std::this_thread::sleep_for(std::chrono::milliseconds(15));
			LI_FN(mouse_event).cached()(MOUSEEVENTF_LEFTUP, NULL, NULL, NULL, NULL);
	}
}
