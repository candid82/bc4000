#include "raylib.h"
#include "raymath.h"
#include <assert.h>
#include <string.h>
#include <time.h>

#include "utils.c"

void gameLogic();
void stageSummaryLogic();
void drawStageSummary();

// #define DRAW_CELL_GRID

#define ASIZE(a) (sizeof(a) / sizeof(a[0]))

typedef struct {
    int row;
    int col;
} CellInfo;

const CellInfo fortressWall[] = {
    {13 * 4 - 6 + 2, 5 * 4 + 2 + 4}, {13 * 4 - 5 + 2, 5 * 4 + 2 + 4},
    {13 * 4 - 4 + 2, 5 * 4 + 2 + 4}, {13 * 4 - 3 + 2, 5 * 4 + 2 + 4},
    {13 * 4 - 2 + 2, 5 * 4 + 2 + 4}, {13 * 4 - 1 + 2, 5 * 4 + 2 + 4},

    {13 * 4 - 6 + 2, 5 * 4 + 3 + 4}, {13 * 4 - 5 + 2, 5 * 4 + 3 + 4},
    {13 * 4 - 4 + 2, 5 * 4 + 3 + 4}, {13 * 4 - 3 + 2, 5 * 4 + 3 + 4},
    {13 * 4 - 2 + 2, 5 * 4 + 3 + 4}, {13 * 4 - 1 + 2, 5 * 4 + 3 + 4},

    {13 * 4 - 6 + 2, 5 * 4 + 8 + 4}, {13 * 4 - 5 + 2, 5 * 4 + 8 + 4},
    {13 * 4 - 4 + 2, 5 * 4 + 8 + 4}, {13 * 4 - 3 + 2, 5 * 4 + 8 + 4},
    {13 * 4 - 2 + 2, 5 * 4 + 8 + 4}, {13 * 4 - 1 + 2, 5 * 4 + 8 + 4},

    {13 * 4 - 6 + 2, 5 * 4 + 9 + 4}, {13 * 4 - 5 + 2, 5 * 4 + 9 + 4},
    {13 * 4 - 4 + 2, 5 * 4 + 9 + 4}, {13 * 4 - 3 + 2, 5 * 4 + 9 + 4},
    {13 * 4 - 2 + 2, 5 * 4 + 9 + 4}, {13 * 4 - 1 + 2, 5 * 4 + 9 + 4},

    {13 * 4 - 6 + 2, 6 * 4 + 0 + 4}, {13 * 4 - 5 + 2, 6 * 4 + 0 + 4},
    {13 * 4 - 6 + 2, 6 * 4 + 1 + 4}, {13 * 4 - 5 + 2, 6 * 4 + 1 + 4},

    {13 * 4 - 6 + 2, 6 * 4 + 2 + 4}, {13 * 4 - 5 + 2, 6 * 4 + 2 + 4},
    {13 * 4 - 6 + 2, 6 * 4 + 3 + 4}, {13 * 4 - 5 + 2, 6 * 4 + 3 + 4},
};
const int FONT_SIZE = 40;
const float TITLE_SLIDE_TIME = 1;
const float STAGE_CURTAIN_TIME = 1.5;
const float STAGE_SUMMARY_SLIDE_TIME = 0.5;
const float TIMER_TIME = 15.0;
const float SHIELD_TIME = 15.0;
const float SHOVEL_TIME = 15.0;
const int POWERUP_SCORE = 500;
const int MAX_POWERUP_COUNT = 3;
const int STAGE_COUNT = 16;
const int SCREEN_WIDTH = 1400;
const int SCREEN_HEIGHT = 900;
const int FIELD_COLS = 64;
const int FIELD_ROWS = 56;
const int CELL_SIZE = 16;
const int SNAP_TO = CELL_SIZE * 2;
const int TANK_SIZE = CELL_SIZE * 4;
const int FLAG_SIZE = TANK_SIZE;
const int TANK_TEXTURE_SIZE = 16;
const Vector2 POWER_UP_TEXTURE_SIZE = {30, 28};
const int POWER_UP_SIZE = CELL_SIZE * 4;
const int SPAWN_TEXTURE_SIZE = 32;
const int UI_TANK_TEXTURE_SIZE = 14;
const int UI_TANK_SIZE = CELL_SIZE * 2;
const int PLAYER1_START_COL = 4 * 4 + 4;
const int PLAYER2_START_COL = 4 * 8 + 4;
const Vector2 PLAYER1_START_POS = {CELL_SIZE * PLAYER1_START_COL,
                                   CELL_SIZE *(FIELD_ROWS - 4 - 2)};
const Vector2 PLAYER2_START_POS = {CELL_SIZE * PLAYER2_START_COL,
                                   CELL_SIZE *(FIELD_ROWS - 4 - 2)};
const int PLAYER_SPEED = 220;
const short ENEMY_SPEEDS[3] = {140, 170, 240};
const int MAX_ENEMY_COUNT = 20;
const int MAX_TANK_COUNT = MAX_ENEMY_COUNT + 2;
const int MAX_BULLET_COUNT = 100;
const int MAX_EXPLOSION_COUNT = MAX_BULLET_COUNT;
const short BULLET_SPEEDS[3] = {450, 500, 550};
const int BULLET_SIZE = 16;
const float BULLET_EXPLOSION_TTL = 0.2f;
const float BIG_EXPLOSION_TTL = 0.4f;
const float ENEMY_SPAWN_INTERVAL = 2.0f;
const float SPAWNING_TIME = 1.0f;
const int POWERUP_POSITIONS_COUNT = 16;
const Vector2 POWERUP_POSITIONS[POWERUP_POSITIONS_COUNT] = {
    {(4 * 4 + 2 + 4) * CELL_SIZE, (7 * 4 + 2 + 2) * CELL_SIZE},
    {(4 * 4 + 2 + 4) * CELL_SIZE, (4 * 4 + 2 + 2) * CELL_SIZE},
    {(7 * 4 + 2 + 4) * CELL_SIZE, (7 * 4 + 2 + 2) * CELL_SIZE},
    {(7 * 4 + 2 + 4) * CELL_SIZE, (4 * 4 + 2 + 2) * CELL_SIZE},

    {(1 * 4 + 2 + 4) * CELL_SIZE, (7 * 4 + 2 + 2) * CELL_SIZE},
    {(1 * 4 + 2 + 4) * CELL_SIZE, (4 * 4 + 2 + 2) * CELL_SIZE},
    {(10 * 4 + 2 + 4) * CELL_SIZE, (7 * 4 + 2 + 2) * CELL_SIZE},
    {(10 * 4 + 2 + 4) * CELL_SIZE, (4 * 4 + 2 + 2) * CELL_SIZE},

    {(1 * 4 + 2 + 4) * CELL_SIZE, (1 * 4 + 2 + 2) * CELL_SIZE},
    {(1 * 4 + 2 + 4) * CELL_SIZE, (10 * 4 + 2 + 2) * CELL_SIZE},
    {(10 * 4 + 2 + 4) * CELL_SIZE, (1 * 4 + 2 + 2) * CELL_SIZE},
    {(10 * 4 + 2 + 4) * CELL_SIZE, (10 * 4 + 2 + 2) * CELL_SIZE},

    {(4 * 4 + 2 + 4) * CELL_SIZE, (1 * 4 + 2 + 2) * CELL_SIZE},
    {(7 * 4 + 2 + 4) * CELL_SIZE, (10 * 4 + 2 + 2) * CELL_SIZE},
    {(4 * 4 + 2 + 4) * CELL_SIZE, (1 * 4 + 2 + 2) * CELL_SIZE},
    {(7 * 4 + 2 + 4) * CELL_SIZE, (10 * 4 + 2 + 2) * CELL_SIZE}};

typedef enum {
    TPlayer1,
    TPlayer2,
    TBasic,
    TFast,
    TPower,
    TArmor,
    TMax
} TankType;
typedef enum { TSPending, TSSpawning, TSActive, TSDead } TankStatus;
typedef enum { DLeft, DRight, DUp, DDown } Direction;
typedef enum {
    UIFlag,
    UIPlayer1,
    UIPlayer2,
    UIP1Tank,
    UIP1Lifes,
    UIP2Tank,
    UIP2Lifes,
    UIStageLowDigit,
    UIStageHiDigit,
    UIMax
} UIElementType;

typedef struct {
    int totalScore;
    char kills[TMax];
} PlayerScore;

typedef struct {
    Texture2D *texture;
    Rectangle textureSrc;
    Vector2 pos;
    Vector2 size;
    Vector2 drawSize;
    bool isVisible;
} UIElement;

typedef struct {
    Texture2D *texture;
    Texture2D *powerUpTexture;
    short speed;
    short bulletSpeed;
    char maxBulletCount;
    bool isEnemy;
    short points;
    char texRow;
} TankSpec;

typedef enum {
    PUStar,
    PUTank,
    PUGrenade,
    PUTimer,
    PUShield,
    PUShovel,
    PUMax,
} PowerUpType;

typedef struct {
    Texture2D *texture;
    int texCol;
} PowerUpSpec;

typedef struct {
    PowerUpType type;
    Vector2 pos;
    bool isActive;
} PowerUp;

typedef struct {
    TankType type;
    Vector2 pos;
    Direction direction;
    char texColOffset;
    char firedBulletCount;
    TankStatus status;
    float spawningTime;
    bool isMoving;
    char lifes;
    PowerUp *powerUp;
    PlayerScore *playerScore;
    char tier;
    float shieldTimeLeft;
} Tank;

typedef struct {
    bool fire;
    bool move;
    Direction direction;
} Command;

typedef struct {
    float duration;
    Texture2D *textures;
    char textureCount;
} Animation;

typedef enum { ETBullet, ETBig, ETMax } ExplosionType;

typedef struct {
    ExplosionType type;
    Vector2 pos;
    float ttl;
    float maxTtl;
} Explosion;

typedef enum { BTNone, BTPlayer, BTEnemy } BulletType;

typedef struct {
    Vector2 pos;
    Vector2 speed;
    Direction direction;
    BulletType type;
    Tank *tank;
} Bullet;

typedef enum {
    CTBorder,
    CTBlank,
    CTBrick,
    CTConcrete,
    CTForest,
    CTRiver,
    CTIce,
    CTMax
} CellType;

typedef struct {
    CellType type;
    Vector2 pos;
    char texRow;
    char texCol;
} Cell;

typedef struct {
    Texture2D *texture;
    bool isSolid;
    bool isPassable;
} CellSpec;

typedef struct {
    Texture2D flag;
    Texture2D brick;
    Texture2D border;
    Texture2D concrete;
    Texture2D forest;
    Texture2D river[2];
    Texture2D blank;
    Texture2D player1Tank;
    Texture2D player2Tank;
    Texture2D enemies;
    Texture2D enemiesWithPowerUps;
    Texture2D bullet;
    Texture2D bulletExplosions[3];
    Texture2D bigExplosions[5];
    Texture2D spawningTank;
    Texture2D uiFlag;
    Texture2D ui;
    Texture2D digits;
    Texture2D powerups;
    Texture2D shield;
    Texture2D ice;
    Texture2D title;
    Texture2D leftArrow;
    Texture2D rightArrow;
} Textures;

typedef enum { MNone, MOnePlayer, MTwoPlayers, MMax } MenuSelectedItem;

typedef struct {
    float time;
    MenuSelectedItem menuSelecteItem;
} Title;

typedef struct {
    float time;
} StageSummary;

typedef enum { GMOnePlayer, GMTwoPlayers } GameMode;

typedef struct {
    Cell field[FIELD_ROWS][FIELD_COLS];
    Tank tanks[MAX_TANK_COUNT];
    TankSpec tankSpecs[TMax];
    Bullet bullets[MAX_BULLET_COUNT];
    Vector2 flagPos;
    CellSpec cellSpecs[CTMax];
    PowerUpSpec powerUpSpecs[PUMax];
    Animation explosionAnimations[ETMax];
    Explosion explosions[MAX_EXPLOSION_COUNT];
    Textures textures;
    float frameTime;
    float totalTime;
    float timeSinceSpawn;
    char activeEnemyCount;
    char pendingEnemyCount;
    char maxActiveEnemyCount;
    char stage;
    UIElement uiElements[UIMax];
    PlayerScore playerScores[2];
    PowerUp powerUps[MAX_POWERUP_COUNT];
    float timerPowerUpTimeLeft;
    float shovelPowerUpTimeLeft;
    void (*logic)();
    void (*draw)();
    Title title;
    StageSummary stageSummary;
    GameMode mode;
    float stageCurtainTime;
} Game;

static Game game;

static void drawCell(Cell *cell) {
    Texture2D *tex = game.cellSpecs[cell->type].texture;
    int w = tex->width / 4;
    int h = tex->height / 4;
    DrawTexturePro(*tex, (Rectangle){cell->texCol * w, cell->texRow * h, w, h},
                   (Rectangle){cell->pos.x, cell->pos.y, CELL_SIZE, CELL_SIZE},
                   (Vector2){}, 0, WHITE);
#ifdef DRAW_CELL_GRID
    DrawRectangleLines(cell->pos.x, cell->pos.y, CELL_SIZE, CELL_SIZE, BLUE);
#endif
}

static void drawField() {
    for (int i = 0; i < FIELD_ROWS; i++) {
        for (int j = 0; j < FIELD_COLS; j++) {
            if (game.field[i][j].type != CTForest)
                drawCell(&game.field[i][j]);
        }
    }
}

static void drawForest() {
    for (int i = 0; i < FIELD_ROWS; i++) {
        for (int j = 0; j < FIELD_COLS; j++) {
            if (game.field[i][j].type == CTForest)
                drawCell(&game.field[i][j]);
        }
    }
}

static void drawTank(Tank *tank) {
    static char textureRows[4] = {1, 3, 0, 2};
    Texture2D *tex = !tank->powerUp || ((long)(game.totalTime * 8)) % 2
                         ? game.tankSpecs[tank->type].texture
                         : game.tankSpecs[tank->type].powerUpTexture;
    int texX = (textureRows[tank->direction] * 2 + tank->texColOffset) *
               TANK_TEXTURE_SIZE;
    int texY = game.tankSpecs[tank->type].texRow * TANK_TEXTURE_SIZE;
    int drawSize = TANK_TEXTURE_SIZE * 4;
    int drawOffset = (TANK_SIZE - drawSize) / 2;
    DrawTexturePro(
        *tex, (Rectangle){texX, texY, TANK_TEXTURE_SIZE, TANK_TEXTURE_SIZE},
        (Rectangle){tank->pos.x + drawOffset, tank->pos.y + drawOffset,
                    drawSize, drawSize},
        (Vector2){}, 0, WHITE);
    if (tank->shieldTimeLeft > 0) {
        Texture2D *tex = &game.textures.shield;
        int texY = (((long)(game.totalTime * 32)) % 2) * tex->width;
        DrawTexturePro(
            *tex, (Rectangle){0, texY, tex->width, tex->width},
            (Rectangle){tank->pos.x, tank->pos.y, TANK_SIZE, TANK_SIZE},
            (Vector2){}, 0, WHITE);
    }
}

static void drawSpawningTank(Tank *tank) {
    static char textureCols[] = {3, 2, 1, 0, 1, 2, 3, 2, 1, 0, 1, 2, 3};
    Texture2D *tex = &game.textures.spawningTank;
    int textureSize = tex->height;
    int i = tank->spawningTime / (SPAWNING_TIME / ASIZE(textureCols));
    if (i >= ASIZE(textureCols))
        i = ASIZE(textureCols) - 1;
    int texX = textureCols[i] * textureSize;
    int drawSize = SPAWN_TEXTURE_SIZE * 2;
    DrawTexturePro(*tex, (Rectangle){texX, 0, textureSize, textureSize},
                   (Rectangle){tank->pos.x, tank->pos.y, drawSize, drawSize},
                   (Vector2){}, 0, WHITE);
}

static void drawTanks() {
    for (int i = 0; i < MAX_TANK_COUNT; i++) {
        if (game.tanks[i].status == TSActive) {
            drawTank(&game.tanks[i]);
        } else if (game.tanks[i].status == TSSpawning) {
            drawSpawningTank(&game.tanks[i]);
        }
    }
}

static void drawFlag() {
    Texture2D *tex = &game.textures.flag;
    DrawTexturePro(
        *tex, (Rectangle){0, 0, tex->width, tex->height},
        (Rectangle){game.flagPos.x, game.flagPos.y, FLAG_SIZE, FLAG_SIZE},
        (Vector2){}, 0, WHITE);
}

static void drawBullets() {
    static int x[4] = {24, 8, 0, 16};
    Texture2D *tex = &game.textures.bullet;
    for (int i = 0; i < MAX_BULLET_COUNT; i++) {
        Bullet *b = &game.bullets[i];
        if (b->type == BTNone)
            continue;
        DrawTexturePro(
            *tex, (Rectangle){x[b->direction], 0, 8, 8},
            (Rectangle){b->pos.x, b->pos.y, BULLET_SIZE, BULLET_SIZE},
            (Vector2){}, 0, WHITE);
    }
}

static void drawExplosions() {
    for (int i = 0; i < MAX_EXPLOSION_COUNT; i++) {
        Explosion *e = &game.explosions[i];
        if (e->ttl <= 0)
            continue;
        int texCount = game.explosionAnimations[e->type].textureCount;
        int index =
            e->ttl / (game.explosionAnimations[e->type].duration / texCount);
        if (index >= texCount)
            index = texCount - 1;
        Texture2D *tex =
            &game.explosionAnimations[e->type].textures[texCount - index - 1];
        DrawTexturePro(
            *tex, (Rectangle){0, 0, tex->width, tex->height},
            (Rectangle){e->pos.x, e->pos.y, tex->width * 2, tex->height * 2},
            (Vector2){}, 0, WHITE);
    }
}

static void drawUITanks() {
    Texture2D *tex = &game.textures.ui;
    int drawSize = UI_TANK_TEXTURE_SIZE * 2;
    int drawOffset = (UI_TANK_SIZE - drawSize) / 2;
    for (int i = 0; i < game.pendingEnemyCount; i++) {
        DrawTexturePro(
            *tex, (Rectangle){0, 0, UI_TANK_TEXTURE_SIZE, UI_TANK_TEXTURE_SIZE},
            (Rectangle){(14 * 4 + 2 + 2 * (i % 2)) * CELL_SIZE + drawOffset,
                        ((2 + 2) + (i / 2 * 2)) * CELL_SIZE + drawOffset,
                        drawSize, drawSize},
            (Vector2){}, 0, WHITE);
    }
}

static void drawUIElement(UIElement *el) {
    int drawOffsetX = (el->size.x - el->drawSize.x) / 2;
    int drawOffsetY = (el->size.y - el->drawSize.y) / 2;
    DrawTexturePro(*(el->texture), el->textureSrc,
                   (Rectangle){el->pos.x + drawOffsetX, el->pos.y + drawOffsetY,
                               el->drawSize.x, el->drawSize.y},
                   (Vector2){}, 0, WHITE);
}

static void drawUIElements() {
    for (int i = 0; i < UIMax; i++) {
        if (game.uiElements[i].isVisible)
            drawUIElement(&game.uiElements[i]);
    }
}

static Rectangle digitTextureRect(char digit) {
    static const int w = 8;
    return (Rectangle){(digit % 5) * w, (digit / 5) * w, w, w};
}

static void drawUI() {
    drawUITanks();
    drawUIElements();
}

static void drawPowerUp(PowerUp *p) {
    if (((long)(game.totalTime * 8)) % 2)
        return;
    Texture2D *tex = game.powerUpSpecs[p->type].texture;
    Vector2 drawSize = {POWER_UP_TEXTURE_SIZE.x * 2,
                        POWER_UP_TEXTURE_SIZE.y * 2};
    Vector2 drawOffset = {(POWER_UP_SIZE - drawSize.x) / 2,
                          (POWER_UP_SIZE - drawSize.y) / 2};
    int texX = game.powerUpSpecs[p->type].texCol * POWER_UP_TEXTURE_SIZE.x;
    DrawTexturePro(
        *tex,
        (Rectangle){texX, 0, POWER_UP_TEXTURE_SIZE.x, POWER_UP_TEXTURE_SIZE.y},
        (Rectangle){p->pos.x + drawOffset.x, p->pos.y + drawOffset.y,
                    drawSize.x, drawSize.y},
        (Vector2){}, 0, WHITE);
}

static void drawPowerUps() {
    for (int i = 0; i < MAX_POWERUP_COUNT; i++) {
        PowerUp *p = &game.powerUps[i];
        if (p->isActive) {
            drawPowerUp(p);
        }
    }
}

static int centerX(int size) { return (SCREEN_WIDTH - size) / 2; }

static void drawStageCurtain() {
    if (game.stageCurtainTime >= STAGE_CURTAIN_TIME)
        return;
    float delayTime = 1;
    int visibleHeight =
        SCREEN_HEIGHT * (MAX(game.stageCurtainTime - delayTime, 0) /
                         (STAGE_CURTAIN_TIME - delayTime));
    int h = (SCREEN_HEIGHT - visibleHeight) / 2;
    DrawRectangle(0, 0, SCREEN_WIDTH, h, (Color){115, 117, 115, 255});
    DrawRectangle(0, SCREEN_HEIGHT - h, SCREEN_WIDTH, h,
                  (Color){115, 117, 115, 255});
    if (game.stageCurtainTime < delayTime) {
        char text[20];
        snprintf(text, 20, "STAGE %2d", game.stage);
        int textSize = MeasureText(text, FONT_SIZE);
        DrawText(text, centerX(textSize), (SCREEN_HEIGHT - FONT_SIZE) / 2,
                 FONT_SIZE, BLACK);
    }
}

static void drawGame() {
    drawField();
    drawBullets();
    drawTanks();
    drawFlag();
    drawForest();
    drawExplosions();
    drawPowerUps();
    drawUI();
    drawStageCurtain();
}

static void loadTextures() {
    game.textures.flag = LoadTexture("textures/flag.png");
    game.textures.leftArrow = LoadTexture("textures/leftArrow.png");
    game.textures.rightArrow = LoadTexture("textures/rightArrow.png");
    game.textures.title = LoadTexture("textures/title.png");
    game.textures.shield = LoadTexture("textures/shield.png");
    game.textures.powerups = LoadTexture("textures/powerup.png");
    game.textures.ui = LoadTexture("textures/ui.png");
    game.textures.digits = LoadTexture("textures/digits.png");
    game.textures.uiFlag = LoadTexture("textures/uiFlag.png");
    game.textures.spawningTank = LoadTexture("textures/born.png");
    game.textures.enemies = LoadTexture("textures/enemies.png");
    game.textures.enemiesWithPowerUps =
        LoadTexture("textures/enemies_with_powerups.png");
    game.textures.border = LoadTexture("textures/border.png");
    game.cellSpecs[CTBorder] =
        (CellSpec){.texture = &game.textures.border, .isSolid = true};
    game.textures.brick = LoadTexture("textures/brick.png");
    game.cellSpecs[CTBrick] =
        (CellSpec){.texture = &game.textures.brick, .isSolid = true};
    game.textures.ice = LoadTexture("textures/ice.png");
    game.cellSpecs[CTIce] =
        (CellSpec){.texture = &game.textures.ice, .isPassable = true};
    game.textures.concrete = LoadTexture("textures/concrete.png");
    game.cellSpecs[CTConcrete] =
        (CellSpec){.texture = &game.textures.concrete, .isSolid = true};
    game.textures.forest = LoadTexture("textures/forest.png");
    game.cellSpecs[CTForest] =
        (CellSpec){.texture = &game.textures.forest, .isPassable = true};
    game.textures.river[0] = LoadTexture("textures/river1.png");
    game.textures.river[1] = LoadTexture("textures/river2.png");
    game.cellSpecs[CTRiver] = (CellSpec){.texture = &game.textures.river[0]};
    game.textures.blank = LoadTexture("textures/blank.png");
    game.cellSpecs[CTBlank] =
        (CellSpec){.texture = &game.textures.blank, .isPassable = true};
    game.textures.player1Tank = LoadTexture("textures/player1.png");
    game.textures.player2Tank = LoadTexture("textures/player2.png");
    game.textures.bullet = LoadTexture("textures/bullet.png");
    game.textures.bulletExplosions[0] =
        LoadTexture("textures/bullet_explosion_1.png");
    game.textures.bulletExplosions[1] =
        LoadTexture("textures/bullet_explosion_2.png");
    game.textures.bulletExplosions[2] =
        LoadTexture("textures/bullet_explosion_3.png");
    game.textures.bigExplosions[0] =
        LoadTexture("textures/big_explosion_1.png");
    game.textures.bigExplosions[1] =
        LoadTexture("textures/big_explosion_2.png");
    game.textures.bigExplosions[2] =
        LoadTexture("textures/big_explosion_3.png");
    game.textures.bigExplosions[3] =
        LoadTexture("textures/big_explosion_4.png");
    game.textures.bigExplosions[4] =
        LoadTexture("textures/big_explosion_5.png");
}

static void loadStage(int stage) {
    char filename[50];
    snprintf(filename, 50, "levels/stage%.2d", stage);
    Buffer buf = readFile(filename);
    int ci = 0;
    for (int i = 0; i < FIELD_ROWS; i++) {
        for (int j = 0; j < FIELD_COLS; j++) {
            if (i <= 1 || i >= FIELD_ROWS - 2 || j <= 3 ||
                j >= FIELD_COLS - 8) {
                game.field[i][j].type = CTBorder;
                game.field[i][j].texRow = 0;
                game.field[i][j].texCol = 0;
                continue;
            }
            game.field[i][j].type = buf.bytes[ci];
            char texNumber = buf.bytes[ci + 1];
            game.field[i][j].texRow = texNumber < 2 ? 0 : 1;
            game.field[i][j].texCol = texNumber % 2;
            ci += 2;
        }
    }
}

static void initUIElements() {
    game.uiElements[UIFlag] =
        (UIElement){.isVisible = true,
                    .texture = &game.textures.uiFlag,
                    .textureSrc = (Rectangle){0, 0, game.textures.uiFlag.width,
                                              game.textures.uiFlag.height},
                    .pos =
                        (Vector2){
                            (14 * 4 + 2) * CELL_SIZE,
                            (11 * 4 * CELL_SIZE),
                        },
                    .size = (Vector2){CELL_SIZE * 4, CELL_SIZE * 4},
                    .drawSize = (Vector2){game.textures.uiFlag.width * 2,
                                          game.textures.uiFlag.height * 2}};
    game.uiElements[UIPlayer1] =
        (UIElement){.isVisible = true,
                    .texture = &game.textures.ui,
                    .textureSrc = (Rectangle){28, 0, 28, 14},
                    .pos =
                        (Vector2){
                            (14 * 4 + 2) * CELL_SIZE + 8,
                            ((7 * 4 + 2) * CELL_SIZE),
                        },
                    .size = (Vector2){14 * 4, 14 * 2},
                    .drawSize = (Vector2){14 * 4, 14 * 2}};
    game.uiElements[UIP1Tank] =
        (UIElement){.isVisible = true,
                    .texture = &game.textures.ui,
                    .textureSrc = (Rectangle){14, 0, 14, 14},
                    .pos =
                        (Vector2){
                            (14 * 4 + 2) * CELL_SIZE,
                            ((8 * 4) * CELL_SIZE),
                        },
                    .size = (Vector2){CELL_SIZE * 2, CELL_SIZE * 2},
                    .drawSize = (Vector2){14 * 2, 14 * 2}};
    game.uiElements[UIP1Lifes] =
        (UIElement){.isVisible = true,
                    .texture = &game.textures.digits,
                    .textureSrc = digitTextureRect(game.tanks[0].lifes),
                    .pos =
                        (Vector2){
                            (15 * 4) * CELL_SIZE,
                            ((8 * 4) * CELL_SIZE),
                        },
                    .size = (Vector2){CELL_SIZE * 2, CELL_SIZE * 2},
                    .drawSize = (Vector2){CELL_SIZE * 2, CELL_SIZE * 2}};
    game.uiElements[UIStageLowDigit] =
        (UIElement){.isVisible = true,
                    .texture = &game.textures.digits,
                    .textureSrc = digitTextureRect(game.stage % 10),
                    .pos =
                        (Vector2){
                            (16 * 4 - 4) * CELL_SIZE,
                            (14 * 4 - 8) * CELL_SIZE,
                        },
                    .size = (Vector2){CELL_SIZE * 2, CELL_SIZE * 2},
                    .drawSize = (Vector2){CELL_SIZE * 2, CELL_SIZE * 2}};
    if (game.stage / 10) {
        game.uiElements[UIStageHiDigit] =
            (UIElement){.isVisible = true,
                        .texture = &game.textures.digits,
                        .textureSrc = digitTextureRect(game.stage / 10),
                        .pos =
                            (Vector2){
                                (16 * 4 - 6) * CELL_SIZE,
                                (14 * 4 - 8) * CELL_SIZE,
                            },
                        .size = (Vector2){CELL_SIZE * 2, CELL_SIZE * 2},
                        .drawSize = (Vector2){CELL_SIZE * 2, CELL_SIZE * 2}};
    }
    if (game.tanks[1].status == TSActive) {
        game.uiElements[UIPlayer2] =
            (UIElement){.isVisible = true,
                        .texture = &game.textures.ui,
                        .textureSrc = (Rectangle){56, 0, 28, 14},
                        .pos =
                            (Vector2){
                                (14 * 4 + 2) * CELL_SIZE + 8,
                                ((9 * 4) * CELL_SIZE),
                            },
                        .size = (Vector2){14 * 4, 14 * 2},
                        .drawSize = (Vector2){14 * 4, 14 * 2}};
        game.uiElements[UIP2Tank] =
            (UIElement){.isVisible = true,
                        .texture = &game.textures.ui,
                        .textureSrc = (Rectangle){14, 0, 14, 14},
                        .pos =
                            (Vector2){
                                (14 * 4 + 2) * CELL_SIZE,
                                ((9 * 4 + 2) * CELL_SIZE),
                            },
                        .size = (Vector2){CELL_SIZE * 2, CELL_SIZE * 2},
                        .drawSize = (Vector2){14 * 2, 14 * 2}};
        game.uiElements[UIP2Lifes] =
            (UIElement){.isVisible = true,
                        .texture = &game.textures.digits,
                        .textureSrc = digitTextureRect(game.tanks[1].lifes),
                        .pos =
                            (Vector2){
                                (15 * 4) * CELL_SIZE,
                                ((9 * 4 + 2) * CELL_SIZE),
                            },
                        .size = (Vector2){CELL_SIZE * 2, CELL_SIZE * 2},
                        .drawSize = (Vector2){CELL_SIZE * 2, CELL_SIZE * 2}};
    }
}

static void spawnPlayer(Tank *t, bool resetTier) {
    t->pos = t->type == TPlayer1 ? PLAYER1_START_POS : PLAYER2_START_POS;
    t->direction = DUp;
    t->status = TSActive;
    if (resetTier) {
        t->tier = 0;
        game.tankSpecs[t->type].bulletSpeed = BULLET_SPEEDS[0];
        game.tankSpecs[t->type].maxBulletCount = 1;
    }
}

static void initStage(char stage) {
    game.stage = stage;
    game.stageCurtainTime = 0;
    for (int i = 0; i < FIELD_ROWS; i++) {
        for (int j = 0; j < FIELD_COLS; j++) {
            game.field[i][j] =
                (Cell){.type = CTBlank,
                       .pos = (Vector2){j * CELL_SIZE, i * CELL_SIZE}};
        }
    }
    loadStage(game.stage);
    game.tanks[0] = (Tank){
        .type = TPlayer1, .lifes = 2, .playerScore = &game.playerScores[0]};
    spawnPlayer(&game.tanks[0], false);
    game.tanks[1] = (Tank){.type = TPlayer2,
                           .lifes = 2,
                           .status = TSPending,
                           .playerScore = &game.playerScores[1]};
    if (game.mode == GMTwoPlayers) {
        spawnPlayer(&game.tanks[1], false);
    }
    static char startingCols[3] = {4, 4 + (FIELD_COLS - 12) / 4 / 2 * 4,
                                   FIELD_COLS - 8 - 4};
    for (int i = 0; i < MAX_ENEMY_COUNT; i++) {
        game.tanks[i + 2] = (Tank){
            .type = TBasic,
            .pos = (Vector2){CELL_SIZE * startingCols[i % 3], CELL_SIZE * 2},
            .direction = DDown,
            .status = TSPending,
        };
        if (i + 1 == 4)
            game.tanks[i + 2].powerUp = &game.powerUps[0];
        else if (i + 1 == 11)
            game.tanks[i + 2].powerUp = &game.powerUps[1];
        else if (i + 1 == 18)
            game.tanks[i + 2].powerUp = &game.powerUps[2];
    }
    for (int i = 0; i < MAX_POWERUP_COUNT; i++) {
        game.powerUps[i] = (PowerUp){
            .type = rand() % PUMax,
            .pos = POWERUP_POSITIONS[rand() % POWERUP_POSITIONS_COUNT],
            .isActive = false};
    }
    memset(game.bullets, 0, sizeof(game.bullets));
    memset(game.explosions, 0, sizeof(game.explosions));
    game.pendingEnemyCount = MAX_ENEMY_COUNT;
    game.maxActiveEnemyCount = 4;
    game.timeSinceSpawn = ENEMY_SPAWN_INTERVAL;
    game.activeEnemyCount = 0;
    initUIElements();
}

static void initGame() {
    loadTextures();
    game.explosionAnimations[ETBullet] =
        (Animation){.duration = BULLET_EXPLOSION_TTL,
                    .textureCount = ASIZE(game.textures.bulletExplosions),
                    .textures = &game.textures.bulletExplosions[0]};
    game.explosionAnimations[ETBig] =
        (Animation){.duration = BIG_EXPLOSION_TTL,
                    .textureCount = ASIZE(game.textures.bigExplosions),
                    .textures = &game.textures.bigExplosions[0]};
    game.flagPos = (Vector2){CELL_SIZE * ((FIELD_COLS - 12) / 2 - 2 + 4),
                             CELL_SIZE * (FIELD_ROWS - 4 - 2)};
    game.tankSpecs[TPlayer1] = (TankSpec){.texture = &game.textures.player1Tank,
                                          .texRow = 0,
                                          .bulletSpeed = BULLET_SPEEDS[0],
                                          .maxBulletCount = 1,
                                          .speed = PLAYER_SPEED};
    game.tankSpecs[TPlayer2] = (TankSpec){.texture = &game.textures.player2Tank,
                                          .texRow = 0,
                                          .bulletSpeed = BULLET_SPEEDS[0],
                                          .maxBulletCount = 1,
                                          .speed = PLAYER_SPEED};
    game.tankSpecs[TBasic] =
        (TankSpec){.texture = &game.textures.enemies,
                   .powerUpTexture = &game.textures.enemiesWithPowerUps,
                   .texRow = 0,
                   .speed = ENEMY_SPEEDS[0],
                   .bulletSpeed = BULLET_SPEEDS[0],
                   .maxBulletCount = 1,
                   .points = 100,
                   .isEnemy = true};
    game.tankSpecs[TFast] =
        (TankSpec){.texture = &game.textures.enemies,
                   .powerUpTexture = &game.textures.enemiesWithPowerUps,
                   .texRow = 1,
                   .speed = ENEMY_SPEEDS[2],
                   .maxBulletCount = 1,
                   .bulletSpeed = BULLET_SPEEDS[1],
                   .points = 200,
                   .isEnemy = true};
    game.tankSpecs[TPower] =
        (TankSpec){.texture = &game.textures.enemies,
                   .powerUpTexture = &game.textures.enemiesWithPowerUps,
                   .texRow = 2,
                   .speed = ENEMY_SPEEDS[1],
                   .bulletSpeed = BULLET_SPEEDS[2],
                   .maxBulletCount = 1,
                   .points = 300,
                   .isEnemy = true};
    game.tankSpecs[TArmor] =
        (TankSpec){.texture = &game.textures.enemies,
                   .powerUpTexture = &game.textures.enemiesWithPowerUps,
                   .texRow = 3,
                   .speed = ENEMY_SPEEDS[1],
                   .bulletSpeed = BULLET_SPEEDS[1],
                   .maxBulletCount = 1,
                   .points = 400,
                   .isEnemy = true};
    game.powerUpSpecs[PUTank] =
        (PowerUpSpec){.texture = &game.textures.powerups, .texCol = 0};
    game.powerUpSpecs[PUTimer] =
        (PowerUpSpec){.texture = &game.textures.powerups, .texCol = 1};
    game.powerUpSpecs[PUShovel] =
        (PowerUpSpec){.texture = &game.textures.powerups, .texCol = 2};
    game.powerUpSpecs[PUGrenade] =
        (PowerUpSpec){.texture = &game.textures.powerups, .texCol = 3};
    game.powerUpSpecs[PUStar] =
        (PowerUpSpec){.texture = &game.textures.powerups, .texCol = 4};
    game.powerUpSpecs[PUShield] =
        (PowerUpSpec){.texture = &game.textures.powerups, .texCol = 5};
}

static void fireBullet(Tank *t) {
    if (t->firedBulletCount >= game.tankSpecs[t->type].maxBulletCount)
        return;
    t->firedBulletCount++;
    for (int i = 0; i < MAX_BULLET_COUNT; i++) {
        Bullet *b = &game.bullets[i];
        if (b->type != BTNone) {
            assert(i != MAX_BULLET_COUNT - 1);
            continue;
        }
        b->type = BTPlayer;
        b->direction = t->direction;
        b->tank = t;
        short bulletSpeed = game.tankSpecs[t->type].bulletSpeed;
        switch (b->direction) {
        case DRight:
            b->pos = (Vector2){t->pos.x + TANK_SIZE - BULLET_SIZE,
                               t->pos.y + TANK_SIZE / 2 - BULLET_SIZE / 2};
            b->speed = (Vector2){bulletSpeed, 0};
            break;
        case DLeft:
            b->pos =
                (Vector2){t->pos.x, t->pos.y + TANK_SIZE / 2 - BULLET_SIZE / 2};
            b->speed = (Vector2){-bulletSpeed, 0};
            break;
        case DUp:
            b->pos =
                (Vector2){t->pos.x + TANK_SIZE / 2 - BULLET_SIZE / 2, t->pos.y};
            b->speed = (Vector2){0, -bulletSpeed};
            break;
        case DDown:
            b->pos = (Vector2){t->pos.x + TANK_SIZE / 2 - BULLET_SIZE / 2,
                               t->pos.y + TANK_SIZE - BULLET_SIZE};
            b->speed = (Vector2){0, bulletSpeed};
            break;
        }
        break;
    }
}

static bool checkTankToTankCollision(Tank *t) {
    for (int i = 0; i < MAX_TANK_COUNT; i++) {
        Tank *tank = &game.tanks[i];
        if (t == tank || tank->status != TSActive)
            continue;
        if (collision(t->pos.x, t->pos.y, TANK_SIZE, TANK_SIZE, tank->pos.x,
                      tank->pos.y, TANK_SIZE, TANK_SIZE))
            return true;
    }
    return false;
}

static bool checkTankCollision(Tank *tank) {
    switch (tank->direction) {
    case DRight: {
        int startRow = ((int)tank->pos.y) / CELL_SIZE;
        int endRow = ((int)tank->pos.y + TANK_SIZE - 1) / CELL_SIZE;
        int col = ((int)tank->pos.x + TANK_SIZE - 1) / CELL_SIZE;
        for (int r = startRow; r <= endRow; r++) {
            CellType cellType = game.field[r][col].type;
            if (!game.cellSpecs[cellType].isPassable) {
                tank->pos.x = game.field[r][col].pos.x - TANK_SIZE;
                return true;
            }
        }
        return false;
    }
    case DLeft: {
        int startRow = ((int)tank->pos.y) / CELL_SIZE;
        int endRow = ((int)tank->pos.y + TANK_SIZE - 1) / CELL_SIZE;
        int col = ((int)tank->pos.x) / CELL_SIZE;
        for (int r = startRow; r <= endRow; r++) {
            CellType cellType = game.field[r][col].type;
            if (!game.cellSpecs[cellType].isPassable) {
                tank->pos.x = game.field[r][col].pos.x + CELL_SIZE;
                return true;
            }
        }
        return false;
    }
    case DUp: {
        int startCol = ((int)tank->pos.x) / CELL_SIZE;
        int endCol = ((int)tank->pos.x + TANK_SIZE - 1) / CELL_SIZE;
        int row = ((int)(tank->pos.y)) / CELL_SIZE;
        for (int c = startCol; c <= endCol; c++) {
            CellType cellType = game.field[row][c].type;
            if (!game.cellSpecs[cellType].isPassable) {
                tank->pos.y = game.field[row][c].pos.y + CELL_SIZE;
                return true;
            }
        }
        return false;
    }
    case DDown: {
        int startCol = ((int)tank->pos.x) / CELL_SIZE;
        int endCol = ((int)tank->pos.x + TANK_SIZE - 1) / CELL_SIZE;
        int row = ((int)tank->pos.y + TANK_SIZE - 1) / CELL_SIZE;
        for (int c = startCol; c <= endCol; c++) {
            CellType cellType = game.field[row][c].type;
            if (!game.cellSpecs[cellType].isPassable) {
                tank->pos.y = game.field[row][c].pos.y - TANK_SIZE;
                return true;
            }
        }
        return false;
    }
    }
}

static int snap(int x) {
    int x1 = (x / SNAP_TO) * SNAP_TO;
    int x2 = x1 + SNAP_TO;
    return (x - x1 < x2 - x) ? x1 : x2;
}

static void updatePlayerLifesUI() {
    game.uiElements[UIP1Lifes].textureSrc =
        digitTextureRect(game.tanks[0].lifes);
    game.uiElements[UIP2Lifes].textureSrc =
        digitTextureRect(game.tanks[1].lifes);
}

static void createExplosion(ExplosionType type, Vector2 targetPos,
                            int targetSize) {
    int explosionSize = game.explosionAnimations[type].textures[0].width * 2;
    int offset = (explosionSize - targetSize) / 2;
    for (int i = 0; i < MAX_EXPLOSION_COUNT; i++) {
        if (game.explosions[i].ttl <= 0) {
            game.explosions[i].ttl = game.explosionAnimations[type].duration;
            game.explosions[i].type = type;
            game.explosions[i].pos =
                (Vector2){targetPos.x - offset, targetPos.y - offset};
            break;
        }
    }
}

static void destroyTank(Tank *t) {
    t->status = TSDead;
    t->lifes--;
    if (game.tankSpecs[t->type].isEnemy) {
        game.activeEnemyCount--;
    }
    if (t->powerUp) {
        t->powerUp->isActive = true;
    }
    createExplosion(ETBig, t->pos, TANK_SIZE);
}

static void destroyAllTanks() {
    for (int i = 0; i < MAX_TANK_COUNT; i++) {
        Tank *t = &game.tanks[i + 2];
        if (t->status == TSActive)
            destroyTank(t);
    }
}

static void handlePowerUpHit(Tank *t) {
    if (game.tankSpecs[t->type].isEnemy)
        return;
    for (int i = 0; i < MAX_POWERUP_COUNT; i++) {
        PowerUp *p = &game.powerUps[i];
        if (p->isActive &&
            collision(t->pos.x, t->pos.y, TANK_SIZE, TANK_SIZE, p->pos.x,
                      p->pos.y, POWER_UP_SIZE, POWER_UP_SIZE)) {
            p->isActive = false;
            t->playerScore->totalScore += POWERUP_SCORE;
            switch (p->type) {
            case PUTank:
                t->lifes++;
                updatePlayerLifesUI();
                break;
            case PUStar:
                if (t->tier == 3)
                    return;
                t->tier++;
                switch (t->tier) {
                case 1:
                    game.tankSpecs[t->type].bulletSpeed = BULLET_SPEEDS[2];
                    break;
                case 2:
                    game.tankSpecs[t->type].maxBulletCount = 2;
                    break;
                case 3:
                    break;
                }
                break;
            case PUGrenade:
                destroyAllTanks();
                break;
            case PUTimer:
                game.timerPowerUpTimeLeft = TIMER_TIME;
                break;
            case PUShield:
                t->shieldTimeLeft = SHIELD_TIME;
                break;
            case PUShovel:
                game.shovelPowerUpTimeLeft = SHOVEL_TIME;
                for (int i = 0; i < ASIZE(fortressWall); i++) {
                    game.field[fortressWall[i].row][fortressWall[i].col].type =
                        CTConcrete;
                    game.field[fortressWall[i].row][fortressWall[i].col]
                        .texRow = fortressWall[i].row % 2;
                    game.field[fortressWall[i].row][fortressWall[i].col]
                        .texCol = fortressWall[i].col % 2;
                }
                break;
            case PUMax:
                break;
            }
            return;
        }
    }
}

static void handleCommand(Tank *t, Command cmd) {
    if (cmd.fire) {
        fireBullet(t);
    }
    t->isMoving = cmd.move;
    if (!cmd.move)
        return;
    t->texColOffset = (t->texColOffset + 1) % 2;
    Vector2 prevPos = t->pos;
    if (t->direction == cmd.direction) {
        int delta = game.frameTime * game.tankSpecs[t->type].speed;
        switch (t->direction) {
        case DLeft:
            t->pos.x -= delta;
            break;
        case DRight:
            t->pos.x += delta;
            break;
        case DUp:
            t->pos.y -= delta;
            break;
        case DDown:
            t->pos.y += delta;
            break;
        }
    } else if (((t->direction == DRight && cmd.direction == DLeft) ||
                (t->direction == DLeft && cmd.direction == DRight)) ||
               ((t->direction == DUp && cmd.direction == DDown) ||
                (t->direction == DDown && cmd.direction == DUp))) {
        t->direction = cmd.direction;
        return;
    } else {
        switch (t->direction) {
        case DLeft:
        case DRight:
            t->pos.x = snap((int)t->pos.x);
            break;
        case DUp:
        case DDown:
            t->pos.y = snap((int)t->pos.y);
            break;
        }
    }
    handlePowerUpHit(t);
    if (checkTankToTankCollision(t)) {
        t->pos = prevPos;
        t->isMoving = false;
    } else {
        if (checkTankCollision(t)) {
            t->isMoving = false;
        }
    }
    t->direction = cmd.direction;
}

static float randomFloat() { return (float)rand() / (float)RAND_MAX; }

static bool randomTrue(float trueChance) { return randomFloat() < trueChance; }

static void handleTankAI(Tank *t) {
    static Direction dirs[] = {DDown,  DDown, DDown, DDown, DRight,
                               DRight, DLeft, DLeft, DUp};
    Command cmd = {};
    cmd.fire = randomTrue(0.01f);
    cmd.move = t->isMoving ? randomTrue(0.995f) : randomTrue(0.08f);
    if (cmd.move) {
        cmd.direction = (t->isMoving && randomTrue(0.99f))
                            ? t->direction
                            : dirs[rand() % ASIZE(dirs)];
    }
    handleCommand(t, cmd);
}

static void handleAI() {
    if (game.timerPowerUpTimeLeft > 0)
        return;
    for (int i = 2; i < MAX_TANK_COUNT; i++) {
        Tank *t = &game.tanks[i];
        if (t->status != TSActive)
            continue;
        handleTankAI(t);
    }
}

static void handleInput() {
    Command cmd = {};
    if (IsKeyDown(KEY_RIGHT)) {
        cmd.move = true;
        cmd.direction = DRight;
    } else if (IsKeyDown(KEY_LEFT)) {
        cmd.move = true;
        cmd.direction = DLeft;
    } else if (IsKeyDown(KEY_UP)) {
        cmd.move = true;
        cmd.direction = DUp;
    } else if (IsKeyDown(KEY_DOWN)) {
        cmd.move = true;
        cmd.direction = DDown;
    }
    if (IsKeyPressed(KEY_Z)) {
        cmd.fire = true;
    }
    handleCommand(&game.tanks[0], cmd);
}

static void destroyBullet(Bullet *b, bool explosion) {
    b->type = BTNone;
    b->tank->firedBulletCount--;
    if (explosion) {
        createExplosion(ETBullet, b->pos, BULLET_SIZE);
    }
}

static void destroyBrick(int row, int col, bool destroyConcrete) {
    if (row < 0 || row >= FIELD_ROWS || col < 0 || col >= FIELD_COLS)
        return;
    if (game.field[row][col].type == CTBrick ||
        (destroyConcrete && game.field[row][col].type == CTConcrete))
        game.field[row][col].type = CTBlank;
}

static void checkBulletRows(Bullet *b, int startRow, int endRow, int col,
                            int nextCol) {
    for (int r = startRow; r <= endRow; r++) {
        CellType cellType = game.field[r][col].type;
        if (game.cellSpecs[cellType].isSolid) {
            destroyBullet(b, true);
            bool destroyConcrete = b->tank->tier == 3;
            for (int rr = startRow - 1; rr <= endRow + 1; rr++) {
                destroyBrick(rr, col, destroyConcrete);
                if (destroyConcrete) {
                    destroyBrick(rr, nextCol, destroyConcrete);
                }
            }
            return;
        }
    }
}

static void checkBulletCols(Bullet *b, int startCol, int endCol, int row,
                            int nextRow) {
    for (int c = startCol; c <= endCol; c++) {
        CellType cellType = game.field[row][c].type;
        if (game.cellSpecs[cellType].isSolid) {
            destroyBullet(b, true);
            bool destroyConcrete = b->tank->tier == 3;
            for (int cc = startCol - 1; cc <= endCol + 1; cc++) {
                destroyBrick(row, cc, destroyConcrete);
                if (destroyConcrete) {
                    destroyBrick(nextRow, cc, destroyConcrete);
                }
            }
            return;
        }
    }
}

static void handlePlayerKill(Tank *t) {
    if (game.tankSpecs[t->type].isEnemy)
        return;
    if (t->lifes == -1) {
        t->lifes = 2;
    }
    spawnPlayer(t, true);
    updatePlayerLifesUI();
}

static void checkStageEnd() {
    if (!game.activeEnemyCount) {
        // if (true) {

        game.logic = stageSummaryLogic;
        game.draw = drawStageSummary;
    }
}

static void checkBulletHit(Bullet *b) {
    for (int i = 0; i < MAX_TANK_COUNT; i++) {
        Tank *t = &game.tanks[i];
        if (t->status != TSActive || b->tank == t ||
            (game.tankSpecs[b->tank->type].isEnemy &&
             game.tankSpecs[t->type].isEnemy))
            continue;
        if (collision(b->pos.x, b->pos.y, BULLET_SIZE, BULLET_SIZE, t->pos.x,
                      t->pos.y, TANK_SIZE, TANK_SIZE)) {
            destroyBullet(b, true);
            if (t->shieldTimeLeft <= 0) {
                destroyTank(t);
                handlePlayerKill(t);
            }
            if (!game.tankSpecs[b->tank->type].isEnemy) {
                game.playerScores[b->tank->type].totalScore +=
                    game.tankSpecs[t->type].points;
                game.playerScores[b->tank->type].kills[t->type]++;
            }
            break;
        }
    }
}

static bool checkBulletToBulletCollision(Bullet *b) {
    for (int i = 0; i < MAX_BULLET_COUNT; i++) {
        Bullet *b2 = &game.bullets[i];
        if (b == b2 || b2->type == BTNone)
            continue;
        if (collision(b->pos.x, b->pos.y, BULLET_SIZE, BULLET_SIZE, b2->pos.x,
                      b2->pos.y, BULLET_SIZE, BULLET_SIZE)) {
            destroyBullet(b, false);
            destroyBullet(b2, false);
            return true;
        }
    }
    return false;
}

static void destroyFlag() { createExplosion(ETBig, game.flagPos, FLAG_SIZE); }

static bool checkFlagHit(Bullet *b) {
    if (collision(b->pos.x, b->pos.y, BULLET_SIZE, BULLET_SIZE, game.flagPos.x,
                  game.flagPos.y, FLAG_SIZE, FLAG_SIZE)) {
        destroyBullet(b, true);
        destroyFlag();
        return true;
    }
    return false;
}

static void checkBulletCollision(Bullet *b) {
    if (checkFlagHit(b))
        return;
    if (checkBulletToBulletCollision(b))
        return;
    checkBulletHit(b);
    switch (b->direction) {
    case DRight: {
        int startRow = ((int)b->pos.y) / CELL_SIZE;
        int endRow = ((int)b->pos.y + BULLET_SIZE - 1) / CELL_SIZE;
        int col = ((int)b->pos.x + BULLET_SIZE - 1) / CELL_SIZE;
        checkBulletRows(b, startRow, endRow, col, col + 1);
        return;
    }
    case DLeft: {
        int startRow = ((int)b->pos.y) / CELL_SIZE;
        int endRow = ((int)b->pos.y + BULLET_SIZE - 1) / CELL_SIZE;
        int col = ((int)b->pos.x) / CELL_SIZE;
        checkBulletRows(b, startRow, endRow, col, col - 1);
        return;
    }
    case DUp: {
        int startCol = ((int)b->pos.x) / CELL_SIZE;
        int endCol = ((int)b->pos.x + BULLET_SIZE - 1) / CELL_SIZE;
        int row = ((int)(b->pos.y)) / CELL_SIZE;
        checkBulletCols(b, startCol, endCol, row, row - 1);
        return;
    }
    case DDown: {
        int startCol = ((int)b->pos.x) / CELL_SIZE;
        int endCol = ((int)b->pos.x + BULLET_SIZE - 1) / CELL_SIZE;
        int row = ((int)b->pos.y + BULLET_SIZE - 1) / CELL_SIZE;
        checkBulletCols(b, startCol, endCol, row, row + 1);
        return;
    }
    }
}

static void updateBulletsState() {
    for (int i = 0; i < MAX_BULLET_COUNT; i++) {
        Bullet *b = &game.bullets[i];
        if (b->type == BTNone)
            continue;
        b->pos.x += (b->speed.x * game.frameTime);
        b->pos.y += (b->speed.y * game.frameTime);
        checkBulletCollision(b);
    }
}

static void updateExplosionsState() {
    for (int i = 0; i < MAX_EXPLOSION_COUNT; i++) {
        if (game.explosions[i].ttl > 0) {
            game.explosions[i].ttl -= game.frameTime;
        }
    }
}

static void spawnTanks() {
    if (game.timeSinceSpawn < ENEMY_SPAWN_INTERVAL ||
        game.activeEnemyCount >= game.maxActiveEnemyCount)
        return;
    game.timeSinceSpawn = 0;
    for (int i = 2; i < MAX_TANK_COUNT; i++) {
        if (game.tanks[i].status == TSPending) {
            game.tanks[i].status = TSSpawning;
            game.activeEnemyCount++;
            game.pendingEnemyCount--;
            return;
        }
    }
}

static void updateGameState() {
    game.cellSpecs[CTRiver].texture =
        &game.textures.river[((long)(game.totalTime * 2)) % 2];
    updateExplosionsState();
    updateBulletsState();
    for (int i = 0; i < MAX_TANK_COUNT; i++) {
        Tank *tank = &game.tanks[i];
        if (tank->status == TSActive) {
            // updateTankState(&game.tanks[i]);
        } else if (tank->status == TSSpawning) {
            tank->spawningTime += game.frameTime;
            if (tank->spawningTime >= SPAWNING_TIME) {
                tank->status = TSActive;
                if (tank->powerUp) {
                    for (int k = 0; k < MAX_POWERUP_COUNT; k++) {
                        game.powerUps[k].isActive = false;
                    }
                }
            }
        }
    }
    spawnTanks();
    checkStageEnd();
}

void drawFloat(float x) {
    char buffer[20];
    snprintf(buffer, 20, "%f", x);
    DrawText(buffer, 10, 10, 10, WHITE);
}

void stageSummaryLogic() {
    game.stageSummary.time += game.frameTime;
    if (IsKeyPressed(KEY_ENTER)) {
        initStage(game.stage + 1);
        game.logic = gameLogic;
        game.draw = drawGame;
    }
}

void drawStageSummary() {
    int topY = SCREEN_HEIGHT -
               (SCREEN_HEIGHT - 150) *
                   (MIN(game.stageSummary.time, STAGE_SUMMARY_SLIDE_TIME) /
                    STAGE_SUMMARY_SLIDE_TIME);
    static const int N = 256;
    char text[N];
    snprintf(text, N, "STAGE %2d", game.stage);
    int textSize = MeasureText(text, FONT_SIZE);
    DrawText(text, centerX(textSize), topY, FONT_SIZE, WHITE);
    int linePadding = 50;
    int halfWidth = SCREEN_WIDTH / 2;
    DrawText("I-PLAYER", (halfWidth - MeasureText(text, FONT_SIZE)) / 2,
             topY + FONT_SIZE + linePadding, FONT_SIZE,
             (Color){205, 62, 26, 255});
    if (game.mode == GMTwoPlayers) {
        DrawText("II-PLAYER",
                 halfWidth + (halfWidth - MeasureText(text, FONT_SIZE)) / 2,
                 topY + FONT_SIZE + linePadding, FONT_SIZE,
                 (Color){205, 62, 26, 255});
    }
    int arrowWidth = game.textures.leftArrow.width;
    int arrowDrawWidth = arrowWidth * 4;
    int arrowHeight = game.textures.leftArrow.height;
    int arrowDrawHeight = arrowHeight * 4;
    int player1TotalKills = 0;
    int player2TotalKills = 0;
    for (int i = 2; i < TMax; i++) {
        int y = topY + (FONT_SIZE + linePadding) * i;
        Texture2D *tex = game.tankSpecs[i].texture;
        int texX = 0;
        int texY = game.tankSpecs[i].texRow * TANK_TEXTURE_SIZE;
        int drawSize = TANK_TEXTURE_SIZE * 4;
        int drawOffset = (TANK_SIZE - drawSize) / 2;
        DrawTexturePro(
            *tex, (Rectangle){texX, texY, TANK_TEXTURE_SIZE, TANK_TEXTURE_SIZE},
            (Rectangle){halfWidth - (drawSize / 2) + drawOffset,
                        y - (drawSize - FONT_SIZE) / 2 + drawOffset, drawSize,
                        drawSize},
            (Vector2){}, 0, WHITE);
        DrawTexturePro(
            game.textures.leftArrow, (Rectangle){0, 0, arrowWidth, arrowHeight},
            (Rectangle){halfWidth - (drawSize / 2) - 10 - arrowDrawWidth,
                        y - (arrowDrawHeight - FONT_SIZE) / 2, arrowDrawWidth,
                        arrowDrawHeight},
            (Vector2){}, 0, WHITE);

        int kills = game.playerScores[TPlayer1].kills[i];
        player1TotalKills += kills;
        snprintf(text, N, "%4d PTS  %2d", kills * game.tankSpecs[i].points,
                 kills);
        DrawText(text, halfWidth - MeasureText(text, FONT_SIZE) - 100, y,
                 FONT_SIZE, WHITE);
        if (game.mode == GMTwoPlayers) {
            DrawTexturePro(game.textures.rightArrow,
                           (Rectangle){0, 0, arrowWidth, arrowHeight},
                           (Rectangle){halfWidth + (drawSize / 2) + 10,
                                       y - (arrowDrawHeight - FONT_SIZE) / 2,
                                       arrowDrawWidth, arrowDrawHeight},
                           (Vector2){}, 0, WHITE);

            kills = game.playerScores[TPlayer2].kills[i];
            player2TotalKills += kills;
            snprintf(text, N, "%2d  %4d PTS", kills * game.tankSpecs[i].points,
                     kills);
            DrawText(text, halfWidth + 100, y, FONT_SIZE, WHITE);
        }
    }
    snprintf(text, N, "TOTAL %2d", player1TotalKills);
    DrawText(text, halfWidth - MeasureText(text, FONT_SIZE) - 100,
             topY + (FONT_SIZE + linePadding) * TMax, FONT_SIZE, WHITE);
    if (game.mode == GMTwoPlayers) {
        snprintf(text, N, "%2d", player2TotalKills);
        DrawText(text, halfWidth + 100, topY + (FONT_SIZE + linePadding) * TMax,
                 FONT_SIZE, WHITE);
    }
}

void titleLogic() {
    game.title.time += game.frameTime;
    if (game.title.time > TITLE_SLIDE_TIME &&
        game.title.menuSelecteItem == MNone) {
        game.title.menuSelecteItem = MOnePlayer;
    }
    if (IsKeyPressed(KEY_LEFT_SHIFT)) {
        game.title.time = TITLE_SLIDE_TIME;
        game.title.menuSelecteItem =
            game.title.menuSelecteItem % (MMax - 1) + 1;
    } else if (IsKeyPressed(KEY_ENTER)) {
        switch (game.title.menuSelecteItem) {
        case MOnePlayer:
            game.mode = GMOnePlayer;
            break;
        case MTwoPlayers:
            game.mode = GMTwoPlayers;
            break;
        default:
            break;
        }
        game.logic = gameLogic;
        game.draw = drawGame;
        initStage(1);
    }
}

void drawTitle() {
    int topY = 200;
    Texture2D *tex = &game.textures.title;
    int titleTexHeight = tex->height;
    int x = (SCREEN_WIDTH - tex->width * 2) / 2;
    int y = SCREEN_HEIGHT -
            (SCREEN_HEIGHT - topY) *
                (MIN(game.title.time, TITLE_SLIDE_TIME) / TITLE_SLIDE_TIME);
    DrawTexturePro(*tex, (Rectangle){0, 0, tex->width, titleTexHeight},
                   (Rectangle){x, y, tex->width * 2, titleTexHeight * 2},
                   (Vector2){}, 0, WHITE);
    if (game.title.menuSelecteItem != MNone) {
        tex = &game.textures.player1Tank;
        int texX =
            (3 * 2 + ((long)(game.totalTime * 16) % 2)) * TANK_TEXTURE_SIZE;
        DrawTexturePro(
            *tex, (Rectangle){texX, 0, TANK_TEXTURE_SIZE, TANK_TEXTURE_SIZE},
            (Rectangle){x + 150,
                        topY + titleTexHeight * 2 - 110 +
                            (game.title.menuSelecteItem - 1) * 60,
                        TANK_TEXTURE_SIZE * 4, TANK_TEXTURE_SIZE * 4},
            (Vector2){}, 0, WHITE);
    }
}

void gameLogic() {
    if (!game.stageCurtainTime) {
        if (IsKeyPressed(KEY_ENTER)) {
            game.stageCurtainTime = 0.001;
        }
        return;
    }
    if (game.stageCurtainTime && game.stageCurtainTime < STAGE_CURTAIN_TIME) {
        game.stageCurtainTime += game.frameTime;
    }
    game.timeSinceSpawn += game.frameTime;
    if (game.timerPowerUpTimeLeft > 0) {
        game.timerPowerUpTimeLeft -= game.frameTime;
    }
    if (game.shovelPowerUpTimeLeft > 0) {
        game.shovelPowerUpTimeLeft -= game.frameTime;
        if (game.shovelPowerUpTimeLeft <= 0) {
            for (int i = 0; i < ASIZE(fortressWall); i++) {
                game.field[fortressWall[i].row][fortressWall[i].col].type =
                    CTBrick;
            }
        }
    }
    if (game.tanks[0].shieldTimeLeft > 0) {
        game.tanks[0].shieldTimeLeft -= game.frameTime;
    }
    if (game.tanks[1].shieldTimeLeft > 0) {
        game.tanks[1].shieldTimeLeft -= game.frameTime;
    }
    handleInput();
    handleAI();
    updateGameState();
}

int main(void) {

    srand(time(0));

    SetTraceLogLevel(LOG_NONE);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Battle City 4000");

    initGame();

    game.logic = titleLogic;
    game.draw = drawTitle;

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        game.frameTime = GetFrameTime();
        game.totalTime = GetTime();

        game.logic();
        BeginDrawing();
        ClearBackground(BLACK);
        game.draw();
        // drawFloat(game.tanks[0].shieldTimeLeft);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
