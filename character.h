//
// Created by user on 2018/12/24.
//

#ifndef FINAL_CLION_CHARACTER_H
#define FINAL_CLION_CHARACTER_H

#include <allegro5/allegro.h>
#include "general.h"
#include "bullet.h"
#include "levelSetting.h"

typedef void (*Skill)(Vector2, enum flyMode, Bullet **, int, float);

typedef struct character {
    int health, damage;
    Vector2 pos;
    Vector2 speed;
    Vector2 size;
    Vector2 firing_point;
    Circle body;
    double shooting_rate;  // for player, boss
    float bullet_speed;
    float e_speed;
    float dire_angle;  // in degree
    int CD, shoot_interval;  // for enemy
    ALLEGRO_BITMAP *image;
    ALLEGRO_BITMAP *default_bullet;
    enum flyMode bullet_mode[MODE_COUNT];
    Skill skill_Q, skill_W, skill_E, skill_R;
    bool can_Q, can_W, can_E, can_R;
    int cd_Q, cd_W, cd_E, cd_R;

    struct character *next;
} Character;

void set_player(ALLEGRO_BITMAP *img, int hp, int dmg, ALLEGRO_BITMAP *bt_img, double rate, float bt_spd, enum flyMode *md,
                int cnt_of_mode, Skill sk_q, int sk_cd[4], int sk_lv);
void set_boss(ALLEGRO_BITMAP *img, int hp, int dmg, ALLEGRO_BITMAP *bt_img, double rate, float bt_spd,
              enum flyMode *md, int cnt_of_mode);

Character *create_enemy(const EnemySetting *prefab);
void register_enemy(Character *enemy);
void destroy_enemy(Character *curr, Character **prev);

#endif //FINAL_CLION_CHARACTER_H
