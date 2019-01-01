#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>

#include "general.h"
#include "bullet.h"
#include "character.h"
#include "fireworks.h"
#include "levelSetting.h"

#define GAME_TERMINATE (-1)

// ALLEGRO Variables
ALLEGRO_DISPLAY *display = NULL;
ALLEGRO_EVENT_QUEUE *event_queue = NULL;
ALLEGRO_BITMAP *cursorImg = NULL;
ALLEGRO_BITMAP *airplaneImgs[20];
ALLEGRO_BITMAP *enemyImgs[20];
ALLEGRO_BITMAP *bulletImgs[20];
ALLEGRO_BITMAP *background = NULL;
ALLEGRO_KEYBOARD_STATE keyState;
ALLEGRO_TIMER *bulletUpdateTimer = NULL;
ALLEGRO_TIMER *shootingDefaultBulletTimer = NULL;
ALLEGRO_TIMER *enemyDefaultBulletTimer = NULL;
ALLEGRO_TIMER *playerMovingTimer = NULL;
ALLEGRO_TIMER *enemyMovingTimer = NULL;
ALLEGRO_TIMER *collisionDetectTimer = NULL;
ALLEGRO_TIMER *generateEnemyBulletTimer = NULL;
ALLEGRO_TIMER *generateEnemyTimer = NULL;

ALLEGRO_TIMER *fireworksTickTimer = NULL;
ALLEGRO_TIMER *generateFirework1Timer = NULL;
ALLEGRO_TIMER *happyTimer = NULL;
ALLEGRO_TIMER *the2019Timer = NULL;

ALLEGRO_SAMPLE *song = NULL;
ALLEGRO_SAMPLE *letoff, *letoff2;
ALLEGRO_SAMPLE_INSTANCE *letoffInstance[2];
ALLEGRO_FONT *bigFont = NULL;
ALLEGRO_FONT *smallFont = NULL;
ALLEGRO_MOUSE_CURSOR *cursor = NULL;

ALLEGRO_COLOR white;

//Custom Definition
const char *title = "Final Project 107062303";
const float FPS = 60;
const int WIDTH = 400;
const int HEIGHT = 600;

Bullet *player_bullet_list = NULL;
Bullet *enemy_bullet_list = NULL;
Bullet *fireworks_bullet_list = NULL;

Character player;
Character boss;
Character *enemy_list = NULL;

LevelSetting settings[6];  // 0 for tutorial

int window = 1;
int draw_count = 0;
int level = 1;
int score;

bool enter_game_window = false;
bool appear = true; //true: appear, false: disappear
bool next = false; //true: trigger
bool dir = true; //true: left, false: right

void show_err_msg(int msg);
void game_init();
void game_begin();
int process_event();
int game_run();
void game_destroy();
void load_images();

bool within(float x, float y, float x1, float y1, float x2, float y2);
bool collide_with(Circle a, Circle b);
void process_bullets(Bullet **list);

void draw_menu();
void draw_about();
void draw_game_scene();
void draw_stage_choosing();

int main(int argc, char *argv[]) {
    int msg = 0;

    srand((unsigned int) time(NULL));

    game_init();
    game_begin();

    while (msg != GAME_TERMINATE){
        msg = game_run();
        if (msg == GAME_TERMINATE){
            printf("Game Over\n");
        }
    }

    game_destroy();
    return 0;
}

void show_err_msg(int msg) {
    fprintf(stderr, "unexpected msg: %d\n", msg);
    game_destroy();
    exit(9);
}

void game_init() {
    if (!al_init()){
        show_err_msg(-1);
    }
    if (!al_install_audio()){
        fprintf(stderr, "failed to initialize audio!\n");
        show_err_msg(-2);
    }
    if (!al_init_acodec_addon()){
        fprintf(stderr, "failed to initialize audio codecs!\n");
        show_err_msg(-3);
    }
    if (!al_reserve_samples(1)){
        fprintf(stderr, "failed to reserve samples!\n");
        show_err_msg(-4);
    }
    // Create display
    display = al_create_display(WIDTH, HEIGHT);
    event_queue = al_create_event_queue();
    if (display == NULL || event_queue == NULL){
        show_err_msg(-5);
    }
    // Initialize Allegro settings
    al_set_window_position(display, 0, 0);
    al_set_window_title(display, title);
    al_init_primitives_addon();
    al_install_keyboard();
    al_install_audio();
    al_install_mouse();
    al_init_image_addon();
    al_init_acodec_addon();
    al_init_font_addon();
    al_init_ttf_addon();

    // Register event
    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_keyboard_event_source());
    al_register_event_source(event_queue, al_get_mouse_event_source());

    // initialize variables
    white = al_map_rgb(255, 255, 255);
}

void load_images() {
    char filename[20];
    for (int i = 0; i < 4; i++){
        sprintf(filename, "bullet%d.png", i);
        bulletImgs[i] = al_load_bitmap(filename);
    }
    for (int i = 0; i < 2; i++){
        sprintf(filename, "airplane%d.png", i);
        airplaneImgs[i] = al_load_bitmap(filename);
    }
    cursorImg = al_load_bitmap("cursor2.png");
    enemyImgs[0] = al_load_bitmap("enemy0.png");
    enemyImgs[10] = al_load_bitmap("boss0.png");
}

void game_begin() {
    // Load sound
    song = al_load_sample("hello.wav");
    letoff = al_load_sample("fireworks.wav");
    letoff2 = al_load_sample("fireworks2.wav");
    al_reserve_samples(3);
    for (int i = 0; i < 2; i++){
        letoffInstance[i] = al_create_sample_instance(letoff);
        al_set_sample_instance_playmode(letoffInstance[i], ALLEGRO_PLAYMODE_ONCE);
        al_set_sample_instance_gain(letoffInstance[i], 1.0);
        al_attach_sample_instance_to_mixer(letoffInstance[i], al_get_default_mixer());
    }
    if (!song || !letoff){
        printf("Audio clip sample not loaded!\n");
        show_err_msg(-6);
    }
    // Loop the song until the display closes
    // al_play_sample(song, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_LOOP, NULL);
    // load font
    bigFont = al_load_ttf_font("pirulen.ttf", 25, 0);
    smallFont = al_load_ttf_font("pirulen.ttf", 15, 0);

    // load images
    load_images();
    draw_menu();
    cursor = al_create_mouse_cursor(cursorImg, 0, 0);
    if (!cursor){
        show_err_msg(5);
    }
    al_set_mouse_cursor(display, cursor);

    // TODO: set level settings
    set_level(&settings[0], 0, 100, 5, 10, 3, 3, 5, enemyImgs[0], enemyImgs[10], bulletImgs[1], 1.0 / 3.0, 5);
    set_level(&settings[1], 1, 100, 12, 30, 5, 8, 12, enemyImgs[0], enemyImgs[10], bulletImgs[1], 1.0 / 3.0, 3);
}


int process_event() {
    // Request the event
    ALLEGRO_EVENT event;
    al_wait_for_event(event_queue, &event);

    if (event.timer.source == bulletUpdateTimer){
        process_bullets(&player_bullet_list);
        process_bullets(&enemy_bullet_list);
        for (Bullet *current = fireworks_bullet_list, *previous = fireworks_bullet_list;
             window == 4 && current != NULL;){
            if (current != fireworks_bullet_list && previous->next != current){
                previous = previous->next;
            }
            float mul = current->speed_multiplier;
            switch (current->mode){
                case up:
                    if (!current->pause){
                        current->pos.y -= 8 * mul;
                    }
                    al_draw_bitmap(current->bitmap, current->pos.x - current->size.x / 2, current->pos.y,
                                   current->flip);
                    break;
                case right_front:
                    if (!current->pause){
                        current->pos.x += 5.7 * sqrtf(mul);
                        current->pos.y -= 5.7 * sqrtf(mul);
                    }
                    al_draw_rotated_bitmap(current->bitmap, current->size.x / 2, 0, current->pos.x, current->pos.y,
                                           (float) (ALLEGRO_PI / 4), current->flip);
                    break;
                case left_front:
                    if (!current->pause){
                        current->pos.x -= 5.7 * sqrtf(mul);
                        current->pos.y -= 5.7 * sqrtf(mul);
                    }
                    al_draw_rotated_bitmap(current->bitmap, current->size.x / 2, 0, current->pos.x, current->pos.y,
                                           (float) (-ALLEGRO_PI / 4), current->flip);
                    break;
                case right:
                    if (!current->pause){
                        current->pos.x += 8 * mul;
                    }
                    al_draw_rotated_bitmap(current->bitmap, current->size.x / 2, 0, current->pos.x, current->pos.y,
                                           (float) (ALLEGRO_PI / 2), current->flip);
                    break;
                case left:
                    if (!current->pause){
                        current->pos.x -= 8 * mul;
                    }
                    al_draw_rotated_bitmap(current->bitmap, current->size.x / 2, 0, current->pos.x, current->pos.y,
                                           (float) (-ALLEGRO_PI / 2), current->flip);
                    break;
                case right_back:
                    if (!current->pause){
                        current->pos.x += 5.7 * sqrtf(mul);
                        current->pos.y += 5.7 * sqrtf(mul);
                    }
                    al_draw_rotated_bitmap(current->bitmap, current->size.x / 2, 0, current->pos.x, current->pos.y,
                                           (float) (ALLEGRO_PI * 3 / 4), current->flip);
                    break;
                case left_back:
                    if (!current->pause){
                        current->pos.x -= 5.7 * sqrtf(mul);
                        current->pos.y += 5.7 * sqrtf(mul);
                    }
                    al_draw_rotated_bitmap(current->bitmap, current->size.x / 2, 0, current->pos.x, current->pos.y,
                                           (float) (-ALLEGRO_PI * 3 / 4), current->flip);
                    break;
                case down:
                    if (!current->pause){
                        current->pos.y += 8 * mul;
                    }
                    al_draw_rotated_bitmap(current->bitmap, current->size.x / 2, 0, current->pos.x, current->pos.y,
                                           (float) ALLEGRO_PI, current->flip);
                    break;
                case rf_f:
                    current->pos.x += 3;
                    current->pos.y -= 7.4;
                    al_draw_rotated_bitmap(current->bitmap, current->size.x / 2, 0, current->pos.x, current->pos.y,
                                           (float) (ALLEGRO_PI / 180 * 22.5), current->flip);
                    break;
                case lf_f:
                    current->pos.x -= 3;
                    current->pos.y -= 7.4;
                    al_draw_rotated_bitmap(current->bitmap, current->size.x / 2, 0, current->pos.x, current->pos.y,
                                           (float) (-ALLEGRO_PI / 180 * 22.5), current->flip);
                    break;
                case rf_r:
                    current->pos.x += 7.4;
                    current->pos.y -= 3;
                    al_draw_rotated_bitmap(current->bitmap, current->size.x / 2, 0, current->pos.x, current->pos.y,
                                           (float) (ALLEGRO_PI / 180 * 67.5), current->flip);
                    break;
                case lf_l:
                    current->pos.x -= 7.4;
                    current->pos.y -= 3;
                    al_draw_rotated_bitmap(current->bitmap, current->size.x / 2, 0, current->pos.x, current->pos.y,
                                           (float) (-ALLEGRO_PI / 180 * 67.5), current->flip);
                    break;
                case rb_r:
                    current->pos.x += 7.4;
                    current->pos.y += 3;
                    al_draw_rotated_bitmap(current->bitmap, current->size.x / 2, 0, current->pos.x, current->pos.y,
                                           (float) (ALLEGRO_PI / 180 * 112.5), current->flip);
                    break;
                case lb_l:
                    current->pos.x -= 7.4;
                    current->pos.y += 3;
                    al_draw_rotated_bitmap(current->bitmap, current->size.x / 2, 0, current->pos.x, current->pos.y,
                                           (float) (-ALLEGRO_PI / 180 * 112.5), current->flip);
                    break;
                case rb_b:
                    current->pos.x += 3;
                    current->pos.y += 7.4;
                    al_draw_rotated_bitmap(current->bitmap, current->size.x / 2, 0, current->pos.x, current->pos.y,
                                           (float) (ALLEGRO_PI / 180 * 157.5), current->flip);
                    break;
                case lb_b:
                    current->pos.x -= 3;
                    current->pos.y += 7.4;
                    al_draw_rotated_bitmap(current->bitmap, current->size.x / 2, 0, current->pos.x, current->pos.y,
                                           (float) (-ALLEGRO_PI / 180 * 157.5), current->flip);
                    break;
                case stopped:
                    destroy_bullet(current, &previous, &fireworks_bullet_list);
                    current = previous;
                    break;
                default:
                    printf("unknown fly mode\n");
                    show_err_msg(1);
                    break;
            }
            if (current == NULL){
                if (fireworks_bullet_list == NULL){
                    break;
                }
                current = fireworks_bullet_list;
                previous = fireworks_bullet_list;
                continue;
            }
            // set the outside bullet stopped
            if (current->pos.y < -40 || current->pos.y > HEIGHT + 40){
                current->mode = stopped;
            }
            else if (current->pos.x < -40 || current->pos.x > WIDTH + 40){
                current->mode = stopped;
            }
            current = current->next;
        }
        draw_count++;
    }
    else if (event.timer.source == playerMovingTimer){
        player.pos.x += player.speed.x;
        player.pos.y += player.speed.y;

        if (player.pos.x < -(player.size.x / 3)){
            player.pos.x = -(player.size.x / 3);
        }
        else if (player.pos.x > WIDTH - player.size.x / 3 * 2){
            player.pos.x = WIDTH - player.size.x / 3 * 2;
        }
        if (player.pos.y < 0){
            player.pos.y = 0;
        }
        else if (player.pos.y > HEIGHT - player.size.y / 3 * 2){
            player.pos.y = HEIGHT - player.size.y / 3 * 2;
        }
    }
    else if (event.timer.source == shootingDefaultBulletTimer){
        for (int i = 0; player.bullet_mode[i]; i++){
            Bullet *bt = make_bullet(player.default_bullet, player.bullet_mode[i],
                                     (Vector2) {player.pos.x + player.firing_point.x,
                                                player.pos.y + player.firing_point.y}, player.default_damage);
            register_bullet(bt, &player_bullet_list);
        }
    }
    else if (event.timer.source == collisionDetectTimer){
        for (Bullet *pla_curr = player_bullet_list; pla_curr != NULL; pla_curr = pla_curr->next){
            for (Bullet *ene_curr = enemy_bullet_list; ene_curr != NULL; ene_curr = ene_curr->next){
                if (collide_with((Circle) {(Vector2) {pla_curr->pos.x + pla_curr->hit_area.center.x,
                                                      pla_curr->pos.y + pla_curr->hit_area.center.y},
                                           pla_curr->hit_area.radius},
                                 (Circle) {(Vector2) {ene_curr->pos.x + ene_curr->hit_area.center.x,
                                                      ene_curr->pos.y + ene_curr->hit_area.center.y},
                                           ene_curr->hit_area.radius})){
                    pla_curr->mode = stopped;
                    ene_curr->mode = stopped;
                    if (player.health > 0){
                        score += rand() % 31 + 30;
                    }
                }
            }
        }
        for (Bullet *pla_curr = player_bullet_list; pla_curr != NULL; pla_curr = pla_curr->next){
            for (Character *enemy = enemy_list; enemy != NULL; enemy = enemy->next){
                if (collide_with((Circle) {(Vector2) {pla_curr->pos.x + pla_curr->hit_area.center.x,
                                                      pla_curr->pos.y + pla_curr->hit_area.center.y},
                                           pla_curr->hit_area.radius},
                                 (Circle) {(Vector2) {enemy->pos.x, enemy->pos.y}, enemy->body.radius})){
                    if (pla_curr->mode != stopped){
                        pla_curr->mode = stopped;
                        enemy->health -= pla_curr->damage;
                        if (player.health > 0){
                            score += rand() % 300 + 300;
                            if (enemy->health <= 0){
                                score += rand() % 600 + 1500;
                            }
                        }
                    }
                }
            }
        }
        for (Bullet *ene_curr = enemy_bullet_list; ene_curr != NULL && player.health > 0; ene_curr = ene_curr->next){
            if (collide_with((Circle) {(Vector2) {ene_curr->pos.x + ene_curr->hit_area.center.x,
                                                  ene_curr->pos.y + ene_curr->hit_area.center.y},
                                       ene_curr->hit_area.radius},
                             (Circle) {(Vector2) {player.pos.x + player.body.center.x,
                                                  player.pos.y + player.body.center.y}, player.body.radius})){
                if (ene_curr->mode == stopped){
                    continue;
                }
                player.health -= ene_curr->damage;
                ene_curr->mode = stopped;
                printf("HURT\n");
                if (player.health <= 0){
                    printf("DEAD\n");
                    player.default_damage = 0;
                }
            }
        }
    }
    else if (event.timer.source == generateEnemyBulletTimer){
        Bullet *bt = make_bullet(settings[level].dropping_bullet, down, (Vector2) {-1, 0}, 5);
        register_bullet(bt, &enemy_bullet_list);
    }
    else if (event.timer.source == enemyMovingTimer){
        for (Character *current = enemy_list, *prev = enemy_list; current != NULL;){
            if (current != enemy_list && prev->next != current){
                prev = prev->next;
            }
            current->pos.x += current->speed.x;
            current->pos.y += current->speed.y;
            al_draw_rotated_bitmap(current->image, current->body.center.x, current->body.center.y,
                                   current->pos.x, current->pos.y,
                                   (float) (ALLEGRO_PI / 180 * current->dire_angle), 0);
            al_draw_circle(current->pos.x, current->pos.y, current->body.radius, white, 0);

            // TODO: change the enemy's direction if necessary

            if (current->health <= 0 || current->pos.y < -100 || current->pos.y > HEIGHT + 100){
                printf("one enemy stopped: %f\n", current->pos.y);
                destroy_enemy(current, &prev);
                current = prev;
                if (current == NULL){
                    current = enemy_list;
                    prev = enemy_list;
                    continue;
                }
            }

            current = current->next;
        }
        draw_count++;
    }
    else if (event.timer.source == enemyDefaultBulletTimer){
        for (Character *enemy = enemy_list; enemy != NULL; enemy = enemy->next){
            enemy->shoot_interval++;
            if (enemy->shoot_interval == enemy->CD){
                enemy->shoot_interval = 0;
                for (int i = 0; enemy->bullet_mode[i]; i++){
                    Bullet *bt = make_bullet(enemy->default_bullet, enemy->bullet_mode[i], enemy->pos,
                                             enemy->default_damage);
                    register_bullet(bt, &enemy_bullet_list);
                }
            }
        }
    }
    else if (event.timer.source == generateEnemyTimer){
        // TODO: change this function call with settings
        Character *ene = create_enemy(settings[level].enemy_img, settings[level].enemy_hp,
                                      settings[level].enemy_damage,
                                      (Vector2) {-1, 0}, 5, 180, bulletImgs[3], 30,
                                      (enum flyMode[]) {down}, 1);
        register_enemy(ene);
        al_set_timer_speed(generateEnemyTimer, settings[level].enemy_rate_base + rand() % 3 - 1);
    }


    static bool normal_fireworks = true;

    if (event.type == ALLEGRO_EVENT_KEY_UP){
        if (window == 5 && event.keyboard.keycode == ALLEGRO_KEY_ENTER){
            enter_game_window = true;
        }
        if (window == 2){
            switch (event.keyboard.keycode){
                // Control
                case ALLEGRO_KEY_UP:
                    player.speed.y -= -10;
                    break;
                case ALLEGRO_KEY_DOWN:
                    player.speed.y -= 10;
                    break;
                case ALLEGRO_KEY_LEFT:
                    player.speed.x -= -10;
                    break;
                case ALLEGRO_KEY_RIGHT:
                    player.speed.x -= 10;
                    break;
                default:
                    break;
            }
        }
    }
    else if (event.type == ALLEGRO_EVENT_KEY_DOWN){
        if (window == 2){
            switch (event.keyboard.keycode){
                case ALLEGRO_KEY_UP:
                    player.speed.y += -10;
                    break;
                case ALLEGRO_KEY_DOWN:
                    player.speed.y += 10;
                    break;
                case ALLEGRO_KEY_LEFT:
                    player.speed.x += -10;
                    break;
                case ALLEGRO_KEY_RIGHT:
                    player.speed.x += 10;
                    break;
                default:
                    break;
            }
        }
        if (window == 4){
            switch (event.keyboard.keycode){
                case ALLEGRO_KEY_F:
                    printf("F\n");
                    draw_H((Vector2) {48, 200}, false, 3);
                    draw_A((Vector2) {138, 200}, false, 3);
                    draw_P2((Vector2) {238, 200}, false, 3);
                    draw_Y((Vector2) {348, 200}, false, 3);
                    al_start_timer(happyTimer);
                    al_start_timer(the2019Timer);
                    normal_fireworks = false;
                    al_stop_timer(generateFirework1Timer);
                    break;
                default:
                    break;
            }
        }
    }

    if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && event.mouse.button == 1){
        printf("press ");
        if (window == 1){
            if (within(event.mouse.x, event.mouse.y, WIDTH / 2 - 150, 225, WIDTH / 2 + 150, 285)){
                //enter_game_window = true;
                draw_stage_choosing();
                window = 5;
                printf("START");
            }

            else if (within(event.mouse.x, event.mouse.y, WIDTH / 2 - 120, 340, WIDTH / 2 + 120, 380)){
                window = 3;
                draw_about();
                printf("ABOUT");
            }
            else if (within(event.mouse.x, event.mouse.y, WIDTH / 2 - 120, 410, WIDTH / 2 + 120, 450)){
                printf("EXIT\n");
                return GAME_TERMINATE;
            }
        }
        else if (window == 2){
            if (within(event.mouse.x, event.mouse.y, 1, HEIGHT - 55, 100, HEIGHT)){
                printf("SPEED UP");
                al_set_timer_speed(playerMovingTimer, 1.0 / 60.0);
            }
            else if (within(event.mouse.x, event.mouse.y, 101, HEIGHT - 55, 200, HEIGHT)){
                printf("SLOW DOWN");
                al_set_timer_speed(playerMovingTimer, 1.0 / 30.0);
            }
            else if (within(event.mouse.x, event.mouse.y, 201, HEIGHT - 55, 300, HEIGHT)){
                printf("FIRE3");
                Bullet *bt_tmp = make_bullet(bulletImgs[0], down, (Vector2) {-1, 0}, player.default_damage);
                register_bullet(bt_tmp, &enemy_bullet_list);
            }
            else if (within(event.mouse.x, event.mouse.y, 301, HEIGHT - 55, 400, HEIGHT)){
                printf("Enemy");
                Character *ene = create_enemy(enemyImgs[0], 10, 8, (Vector2) {-1, 0}, 5, 180,
                                              bulletImgs[3], 30, (enum flyMode[]) {down}, 1);
                register_enemy(ene);
            }
        }
        else if (window == 3){
            if (within(event.mouse.x, event.mouse.y, WIDTH / 2 - 120, 440, WIDTH / 2 + 120, 480)){
                window = 1;
                draw_menu();
                printf("BACK");
            }
            else if (within(event.mouse.x, event.mouse.y, WIDTH / 2 - 120, 360, WIDTH / 2 + 120, 400)){
                window = 4;
                al_clear_to_color(al_map_rgb(10, 10, 20));
                al_flip_display();
                printf("SPECIAL");
                bulletUpdateTimer = al_create_timer(1.0 / 30.0);
                fireworksTickTimer = al_create_timer(1.0 / 30.0);
                generateFirework1Timer = al_create_timer(1.0 / 3.0);
                happyTimer = al_create_timer(1.0 / 30.0);
                the2019Timer = al_create_timer(1.0 / 30.0);
                al_register_event_source(event_queue, al_get_timer_event_source(bulletUpdateTimer));
                al_register_event_source(event_queue, al_get_timer_event_source(fireworksTickTimer));
                al_register_event_source(event_queue, al_get_timer_event_source(generateFirework1Timer));
                al_register_event_source(event_queue, al_get_timer_event_source(happyTimer));
                al_register_event_source(event_queue, al_get_timer_event_source(the2019Timer));
                al_start_timer(bulletUpdateTimer);
                al_start_timer(fireworksTickTimer);
                al_start_timer(generateFirework1Timer);
            }
        }
        else if (window == 5){
            if (within(event.mouse.x, event.mouse.y, 60, 90, 190, 180)){
                printf("1");
                enter_game_window = true;
            }
            else if (within(event.mouse.x, event.mouse.y, 210, 90, 340, 180)){
                printf("2");
            }
            else if (within(event.mouse.x, event.mouse.y, 60, 200, 190, 290)){
                printf("3");
            }
            else if (within(event.mouse.x, event.mouse.y, 210, 200, 340, 290)){
                printf("4");
            }
            else if (within(event.mouse.x, event.mouse.y, 60, 310, 340, 400)){
                printf("5");
            }
            else if (within(event.mouse.x, event.mouse.y, 60, 420, 340, 510)){
                printf("ENDLESS");
            }
        }
        printf("\n");
    }

    if (window == 4){
        static bool finish = false;
        if (event.timer.source == fireworksTickTimer){
            static Vector2 tmp;
            static int size;
            static int tick = 0, tick_saved = -100;

            if (finish && fireworks_bullet_list == NULL){
                al_draw_text(smallFont, white, WIDTH / 2, HEIGHT / 2, ALLEGRO_ALIGN_CENTER, "REPLAY?");
                al_draw_rectangle(WIDTH / 2 - 120, HEIGHT / 2 - 10, WIDTH / 2 + 120, HEIGHT / 2 + 30, white, 0);
                al_draw_text(smallFont, white, WIDTH / 2 + 60, HEIGHT / 2 + 55, ALLEGRO_ALIGN_CENTER, "BACK");
                al_draw_rectangle(WIDTH / 2, HEIGHT / 2 + 50, WIDTH / 2 + 120, HEIGHT / 2 + 80, white, 0);
                al_flip_display();
            }
            for (Bullet *curr = fireworks_bullet_list; curr != NULL; curr = curr->next){
                curr->time--;
                if (curr->time == 0){
                    curr->mode = stopped;
                }
                if (curr->time == 249){
                    curr->pause = false;
                }
                if (curr->time == 15 && curr->pause){
                    curr->pause = false;
                }
            }
            if (!finish){
                tick++;
            }
            else {
                tick = 0;
                tick_saved = -100;
            }
            if (normal_fireworks){
                if (tick % 65 == 0 && tick != 0){
                    tmp = (Vector2) {rand() % 280 + 60, rand() % 250 + 350};
                    tick_saved = 0;
                    tick = 0;
                    size = rand() % 5 + 16;
                    Bullet *bt = make_firework_bullet(bulletImgs[3], up, tmp, 15, 2, NULL);
                    register_bullet(bt, &fireworks_bullet_list);
                    tmp.y -= 240;
                }
                if (tick == tick_saved + 15){
                    al_play_sample(letoff, 0.8, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                    draw_fireworks1(size, tmp);
                }
                if (tick == tick_saved + 20){
                    draw_fireworks1(size, tmp);
                }
                if (tick == tick_saved + 25){
                    draw_fireworks2(size, tmp);
                }
            }
        }
        else if (event.timer.source == generateFirework1Timer){
            al_play_sample(letoff, (float) (1.0 / (rand() % 4 + 2)), 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
            draw_fireworks1(rand() % 4 + 16, (Vector2) {rand() % 240 + 80, rand() % 400 + 200});
            al_set_timer_speed(generateFirework1Timer, 1.0 / (rand() % 3 + 1));
        }
        else if (event.timer.source == happyTimer){
            al_play_sample(letoff, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
            al_play_sample(letoff2, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
            draw_H((Vector2) {48, 200}, true, 1);
            draw_A((Vector2) {138, 200}, true, 2);
            draw_P2((Vector2) {238, 200}, true, 2);
            draw_Y((Vector2) {348, 200}, true, 1);
            draw_H((Vector2) {48, 200}, true, 3);
            draw_A((Vector2) {138, 200}, true, 3);
            draw_P2((Vector2) {238, 200}, true, 3);
            draw_Y((Vector2) {348, 200}, true, 3);
            al_stop_timer(happyTimer);
        }
        else if (event.timer.source == the2019Timer){
            static int tick = 0;
            tick++;
            if (tick == 20){
                draw_2((Vector2) {55, 450}, true, 3);
            }
            else if (tick == 30){
                draw_0((Vector2) {155, 450}, true, 3);
            }
            else if (tick == 40){
                draw_1((Vector2) {255, 450}, true, 3);
            }
            else if (tick == 50){
                draw_9((Vector2) {340, 450}, true, 3);
            }
            if (tick == 105){
                al_play_sample_instance(letoffInstance[0]);
            }
            else if (tick == 115){
                al_play_sample_instance(letoffInstance[1]);
            }
            else if (tick == 125){
                al_play_sample_instance(letoffInstance[0]);
            }
            else if (tick == 135){
                al_play_sample_instance(letoffInstance[1]);
            }
            else if (tick == 150){
                al_play_sample(letoff, 1.2, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                al_play_sample(letoff2, 1.5, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                tick = 0;
                finish = true;
                al_stop_timer(the2019Timer);
            }
        }

        if (finish && event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN){
            if (within(event.mouse.x, event.mouse.y,
                       WIDTH / 2 - 120, HEIGHT / 2 - 10, WIDTH / 2 + 120, HEIGHT / 2 + 30)){
                finish = false;
                normal_fireworks = true;
                al_clear_to_color(al_map_rgb(10, 10, 20));
                al_flip_display();
                al_start_timer(generateFirework1Timer);
            }
            if (within(event.mouse.x, event.mouse.y,
                       WIDTH / 2, HEIGHT / 2 + 50, WIDTH / 2 + 120, HEIGHT / 2 + 80)){
                window = 3;
                al_stop_timer(bulletUpdateTimer);
                al_stop_timer(fireworksTickTimer);
                finish = false;
                normal_fireworks = true;
                draw_about();
                al_flip_display();
            }
        }
    }

    // Shutdown our program
    if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE){
        printf("byebye");
        return GAME_TERMINATE;
    }

    return 0;
}

int game_run() {
    int error = 0;
    // First window(Menu)
    if (window == 1){
        if (!al_is_event_queue_empty(event_queue)){
            error = process_event();
            /**if (enter_game_window){
                window = 2;
                // Setting Character
                player.pos.x = 200;
                player.pos.y = HEIGHT - 100;
                set_character(airplaneImgs[0], settings[level].player_hp, 5, bulletImgs[0], 1.0 / 7.0,
                              (enum flyMode[]) {up}, 1);

                // Initialize Timer
                playerMovingTimer = al_create_timer(1.0 / 30.0);
                enemyMovingTimer = al_create_timer(1.0 / 30.0);
                bulletUpdateTimer = al_create_timer(1.0 / 30.0);
                shootingDefaultBulletTimer = al_create_timer(player.shooting_rate);
                enemyDefaultBulletTimer = al_create_timer(1.0 / 60.0);
                collisionDetectTimer = al_create_timer(1.0 / 60.0);
                generateEnemyBulletTimer = al_create_timer(settings[level].bullet_rate);
                generateEnemyTimer = al_create_timer(rand() % 3 + 4);
                al_register_event_source(event_queue, al_get_timer_event_source(playerMovingTimer));
                al_register_event_source(event_queue, al_get_timer_event_source(enemyMovingTimer));
                al_register_event_source(event_queue, al_get_timer_event_source(bulletUpdateTimer));
                al_register_event_source(event_queue, al_get_timer_event_source(shootingDefaultBulletTimer));
                al_register_event_source(event_queue, al_get_timer_event_source(enemyDefaultBulletTimer));
                al_register_event_source(event_queue, al_get_timer_event_source(collisionDetectTimer));
                al_register_event_source(event_queue, al_get_timer_event_source(generateEnemyBulletTimer));
                al_register_event_source(event_queue, al_get_timer_event_source(generateEnemyTimer));
                al_start_timer(playerMovingTimer);
                al_start_timer(enemyMovingTimer);
                al_start_timer(bulletUpdateTimer);
                al_start_timer(shootingDefaultBulletTimer);
                al_start_timer(enemyDefaultBulletTimer);
                al_start_timer(collisionDetectTimer);
                al_start_timer(generateEnemyBulletTimer);
                al_start_timer(generateEnemyTimer);
            }
            */
        }
    }
        // Second window(Main Game)
    else if (window == 2){
        if (draw_count == 0){
            draw_game_scene();
        }
        if (draw_count >= 2){
            draw_count = 0;
            al_flip_display();
        }

        // Listening for new event
        if (!al_is_event_queue_empty(event_queue)){
            error = process_event();
        }
    }
    else if (window == 3){  // for About
        if (!al_is_event_queue_empty(event_queue)){
            error = process_event();
        }
    }
    else if (window == 4){  // for Special
        if (draw_count == 0){
            al_clear_to_color(al_map_rgb(10, 10, 20));
            al_draw_rectangle(0, 0, 0, 0, al_map_rgb(10, 10, 20), 0);
        }
        if (draw_count >= 1){
            draw_count = 0;
            al_flip_display();
        }

        if (!al_is_event_queue_empty(event_queue)){
            error = process_event();
        }
    }
    else if (window == 5){  // choose stage
        if (!al_is_event_queue_empty(event_queue)){
            error = process_event();
            if (enter_game_window){
                window = 2;
                // Setting Character
                player.pos.x = 200;
                player.pos.y = HEIGHT - 100;
                set_character(airplaneImgs[0], settings[level].player_hp, 5, bulletImgs[0], 1.0 / 7.0,
                              (enum flyMode[]) {up}, 1);

                // Initialize Timer
                playerMovingTimer = al_create_timer(1.0 / 30.0);
                enemyMovingTimer = al_create_timer(1.0 / 30.0);
                bulletUpdateTimer = al_create_timer(1.0 / 30.0);
                shootingDefaultBulletTimer = al_create_timer(player.shooting_rate);
                enemyDefaultBulletTimer = al_create_timer(1.0 / 60.0);
                collisionDetectTimer = al_create_timer(1.0 / 60.0);
                generateEnemyBulletTimer = al_create_timer(settings[level].bullet_rate);
                generateEnemyTimer = al_create_timer(rand() % 3 + 4);
                al_register_event_source(event_queue, al_get_timer_event_source(playerMovingTimer));
                al_register_event_source(event_queue, al_get_timer_event_source(enemyMovingTimer));
                al_register_event_source(event_queue, al_get_timer_event_source(bulletUpdateTimer));
                al_register_event_source(event_queue, al_get_timer_event_source(shootingDefaultBulletTimer));
                al_register_event_source(event_queue, al_get_timer_event_source(enemyDefaultBulletTimer));
                al_register_event_source(event_queue, al_get_timer_event_source(collisionDetectTimer));
                al_register_event_source(event_queue, al_get_timer_event_source(generateEnemyBulletTimer));
                al_register_event_source(event_queue, al_get_timer_event_source(generateEnemyTimer));
                al_start_timer(playerMovingTimer);
                al_start_timer(enemyMovingTimer);
                al_start_timer(bulletUpdateTimer);
                al_start_timer(shootingDefaultBulletTimer);
                al_start_timer(enemyDefaultBulletTimer);
                al_start_timer(collisionDetectTimer);
                al_start_timer(generateEnemyBulletTimer);
                al_start_timer(generateEnemyTimer);
            }
        }
    }
    return error;
}

void game_destroy() {
    // Make sure you destroy all things
    al_destroy_event_queue(event_queue);
    al_destroy_font(bigFont);
    al_destroy_font(smallFont);
    al_destroy_bitmap(cursorImg);
    for (int i = 0; i < 2; i++){
        al_destroy_bitmap(bulletImgs[i]);
    }
    for (int i = 0; i < 1; i++){
        al_destroy_bitmap(airplaneImgs[i]);
    }
    al_destroy_timer(bulletUpdateTimer);
    al_destroy_timer(playerMovingTimer);
    al_destroy_timer(shootingDefaultBulletTimer);
    al_destroy_timer(collisionDetectTimer);
    al_destroy_sample(song);
    al_destroy_mouse_cursor(cursor);
    al_destroy_display(display);
}


bool within(float x, float y, float x1, float y1, float x2, float y2) {
    if (y >= y1 && y <= y2 && x >= x1 && x <= x2){
        return true;
    }
    else {
        return false;
    }
}

bool collide_with(Circle a, Circle b) {
    if (sqrtf(powf(b.center.x - a.center.x, 2) + powf(b.center.y - a.center.y, 2)) <= a.radius + b.radius){
        return true;
    }
    return false;
}

void draw_menu() {
    al_clear_to_color(al_map_rgb(70, 70, 80));
    // draw button
    al_draw_text(bigFont, al_map_rgb(255, 255, 200), WIDTH / 2, HEIGHT / 2 - 60, ALLEGRO_ALIGN_CENTRE, "S T A R T");
    al_draw_rectangle(WIDTH / 2 - 150, 225, WIDTH / 2 + 150, 285, al_map_rgb(255, 255, 200), 0);
    al_draw_text(smallFont, white, WIDTH / 2, HEIGHT / 2 + 50, ALLEGRO_ALIGN_CENTRE, "ABOUT");
    al_draw_rectangle(WIDTH / 2 - 120, 340, WIDTH / 2 + 120, 380, white, 0);
    al_draw_text(smallFont, al_map_rgb(255, 100, 100), WIDTH / 2, HEIGHT / 2 + 120, ALLEGRO_ALIGN_CENTRE, "EXIT");
    al_draw_rectangle(WIDTH / 2 - 120, 410, WIDTH / 2 + 120, 450, al_map_rgb(255, 100, 100), 0);

    al_flip_display();
}

void draw_about() {
    al_clear_to_color(al_map_rgb(70, 70, 10));
    al_draw_text(smallFont, white, WIDTH / 2, 450, ALLEGRO_ALIGN_CENTER, "BACK");
    al_draw_rectangle(WIDTH / 2 - 120, 440, WIDTH / 2 + 120, 480, white, 0);
    al_draw_text(smallFont, white, WIDTH / 2, 370, ALLEGRO_ALIGN_CENTER, "SPECIAL");
    al_draw_rectangle(WIDTH / 2 - 120, 360, WIDTH / 2 + 120, 400, white, 0);
    al_flip_display();
}

void draw_game_scene() {
    al_clear_to_color(al_map_rgb(50, 50, 80));
    /*
    al_draw_text(smallFont, white, 50, HEIGHT - 50 + 15, ALLEGRO_ALIGN_CENTER, "Speed UP");
    al_draw_rectangle(1, HEIGHT - 55, 100, HEIGHT, white, 0);
    al_draw_text(smallFont, white, 150, HEIGHT - 50 + 15, ALLEGRO_ALIGN_CENTER, "Slow");
    al_draw_rectangle(100, HEIGHT - 55, 200, HEIGHT, white, 0);

    al_draw_text(smallFont, white, 250, HEIGHT - 50 + 15, ALLEGRO_ALIGN_CENTER, "FIRE3");
    al_draw_rectangle(200, HEIGHT - 55, 300, HEIGHT, white, 0);
    al_draw_text(smallFont, white, 350, HEIGHT - 50 + 15, ALLEGRO_ALIGN_CENTER, "FIRE4");
    al_draw_rectangle(300, HEIGHT - 55, 400, HEIGHT, white, 0);
    */
    char score_text[10];
    sprintf(score_text, "%08d", score);
    al_draw_text(bigFont, white, WIDTH - 5, 5, ALLEGRO_ALIGN_RIGHT, score_text);
    if (player.health > 0){
        double ratio = (double) player.health / settings[level].player_hp;
        ALLEGRO_COLOR hp_bar = al_map_rgb((unsigned char) (150 + 50 * (1 - ratio)),
                                          (unsigned char) (50 + 150 * ratio), 0);
        al_draw_filled_rectangle(10, HEIGHT - 40, (float) (10 + 150 * ratio), HEIGHT - 10, hp_bar);
        al_draw_rectangle(10, HEIGHT - 40, 160, HEIGHT - 10, white, 0);
        al_draw_bitmap(player.image, player.pos.x, player.pos.y, 0);
        al_draw_circle(player.pos.x + player.body.center.x, player.pos.y + player.body.center.y,
                       player.body.radius, white, 0);
    }
    else {
        al_draw_rectangle(WIDTH / 2 - 150, HEIGHT / 2 - 20, WIDTH / 2 + 150, HEIGHT / 2 + 50, white, 0);
        al_draw_text(bigFont, white, WIDTH / 2, HEIGHT / 2, ALLEGRO_ALIGN_CENTER, "GAME OVER");
        al_draw_rectangle(WIDTH / 2 - 90, HEIGHT / 2 + 60, WIDTH / 2 + 150, HEIGHT / 2 + 100, white, 0);
        al_draw_text(smallFont, white, WIDTH / 2 + 30, HEIGHT / 2 + 70, ALLEGRO_ALIGN_CENTER, "RESTART");
    }
}

void draw_stage_choosing() {
    ALLEGRO_FONT *fnt = al_load_font("pirulen.ttf", 40, 0);
    al_clear_to_color(al_map_rgb(126, 46, 41));
    al_draw_rectangle(60, 90, 190, 180, white, 0);
    al_draw_text(fnt, white, 125, 110, ALLEGRO_ALIGN_CENTER, "1");
    al_draw_rectangle(210, 90, 340, 180, white, 0);
    al_draw_text(fnt, white, 275, 110, ALLEGRO_ALIGN_CENTER, "2");
    al_draw_rectangle(60, 200, 190, 290, white, 0);
    al_draw_text(fnt, white, 125, 220, ALLEGRO_ALIGN_CENTER, "3");
    al_draw_rectangle(210, 200, 340, 290, white, 0);
    al_draw_text(fnt, white, 275, 220, ALLEGRO_ALIGN_CENTER, "4");
    al_draw_rectangle(60, 310, 340, 400, white, 0);
    al_draw_text(fnt, white, 200, 330, ALLEGRO_ALIGN_CENTER, "5");
    al_draw_rectangle(60, 420, 340, 510, white, 0);
    al_draw_text(bigFont, white, 200, 450, ALLEGRO_ALIGN_CENTER, "ENDLESS");

    al_flip_display();
}

void process_bullets(Bullet **list) {
    for (Bullet *current = *list, *previous = *list; current != NULL;){
        if (current != *list && previous->next != current){
            previous = previous->next;
        }
        switch (current->mode){
            case up:
                current->pos.y -= 17;
                al_draw_bitmap(current->bitmap, current->pos.x - current->size.x / 2, current->pos.y,
                               current->flip);
                break;
            case right_front:
                current->pos.x += 12;
                current->pos.y -= 12;
                al_draw_rotated_bitmap(current->bitmap, current->size.x / 2, 0, current->pos.x, current->pos.y,
                                       (float) (ALLEGRO_PI / 4), current->flip);
                break;
            case left_front:
                current->pos.x -= 12;
                current->pos.y -= 12;
                al_draw_rotated_bitmap(current->bitmap, current->size.x / 2, 0, current->pos.x, current->pos.y,
                                       (float) (-ALLEGRO_PI / 4), current->flip);
                break;
            case right:
                current->pos.x += 17;
                al_draw_rotated_bitmap(current->bitmap, current->size.x / 2, 0, current->pos.x, current->pos.y,
                                       (float) (ALLEGRO_PI / 2), current->flip);
                break;
            case left:
                current->pos.x -= 17;
                al_draw_rotated_bitmap(current->bitmap, current->size.x / 2, 0, current->pos.x, current->pos.y,
                                       (float) (-ALLEGRO_PI / 2), current->flip);
                break;
            case right_back:
                current->pos.x += 12;
                current->pos.y += 12;
                al_draw_rotated_bitmap(current->bitmap, current->size.x / 2, 0, current->pos.x, current->pos.y,
                                       (float) (ALLEGRO_PI * 3 / 4), current->flip);
                break;
            case left_back:
                current->pos.x -= 12;
                current->pos.y += 12;
                al_draw_rotated_bitmap(current->bitmap, current->size.x / 2, 0, current->pos.x, current->pos.y,
                                       (float) (-ALLEGRO_PI * 3 / 4), current->flip);
                break;
            case down:
                current->pos.y += 17;
                al_draw_rotated_bitmap(current->bitmap, current->size.x / 2, 0, current->pos.x, current->pos.y,
                                       (float) ALLEGRO_PI, current->flip);
                break;
            case rf_f:
                current->pos.x += 6;
                current->pos.y -= 16;
                al_draw_rotated_bitmap(current->bitmap, current->size.x / 2, 0, current->pos.x, current->pos.y,
                                       (float) (ALLEGRO_PI / 180 * 22.5), current->flip);
                break;
            case lf_f:
                current->pos.x -= 6;
                current->pos.y -= 16;
                al_draw_rotated_bitmap(current->bitmap, current->size.x / 2, 0, current->pos.x, current->pos.y,
                                       (float) (-ALLEGRO_PI / 180 * 22.5), current->flip);
                break;
            case rf_r:
                current->pos.x += 16;
                current->pos.y -= 6;
                al_draw_rotated_bitmap(current->bitmap, current->size.x / 2, 0, current->pos.x, current->pos.y,
                                       (float) (ALLEGRO_PI / 180 * 67.5), current->flip);
                break;
            case lf_l:
                current->pos.x -= 16;
                current->pos.y -= 6;
                al_draw_rotated_bitmap(current->bitmap, current->size.x / 2, 0, current->pos.x, current->pos.y,
                                       (float) (-ALLEGRO_PI / 180 * 67.5), current->flip);
                break;
            case rb_r:
                current->pos.x += 16;
                current->pos.y += 6;
                al_draw_rotated_bitmap(current->bitmap, current->size.x / 2, 0, current->pos.x, current->pos.y,
                                       (float) (ALLEGRO_PI / 180 * 112.5), current->flip);
                break;
            case lb_l:
                current->pos.x -= 16;
                current->pos.y += 6;
                al_draw_rotated_bitmap(current->bitmap, current->size.x / 2, 0, current->pos.x, current->pos.y,
                                       (float) (-ALLEGRO_PI / 180 * 112.5), current->flip);
                break;
            case rb_b:
                current->pos.x += 6;
                current->pos.y += 16;
                al_draw_rotated_bitmap(current->bitmap, current->size.x / 2, 0, current->pos.x, current->pos.y,
                                       (float) (ALLEGRO_PI / 180 * 157.5), current->flip);
                break;
            case lb_b:
                current->pos.x -= 6;
                current->pos.y += 16;
                al_draw_rotated_bitmap(current->bitmap, current->size.x / 2, 0, current->pos.x, current->pos.y,
                                       (float) (-ALLEGRO_PI / 180 * 157.5), current->flip);
                break;
            case stopped:
                destroy_bullet(current, &previous, list);
                current = previous;
                break;
            default:
                printf("unknown fly mode\n");
                show_err_msg(1);
                break;
        }
        if (current == NULL){
            if (*list == NULL){
                break;
            }
            //printf("NULL\n");
            current = *list;
            previous = *list;
            continue;
        }
        al_draw_circle(current->pos.x + current->hit_area.center.x,
                       current->pos.y + current->hit_area.center.y,
                       current->hit_area.radius, al_map_rgb(255, 220, 188), 0);

        // set the outside bullet stopped
        if (current->pos.y < -40 || current->pos.y > HEIGHT + 40){
            current->mode = stopped;
        }
        else if (current->pos.x < -40 || current->pos.x > WIDTH + 40){
            current->mode = stopped;
        }
        current = current->next;
    }

}
