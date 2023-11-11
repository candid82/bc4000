#include "raylib.h"
#include "raymath.h"
#include <assert.h>

#include "utils.c"

// #define DRAW_CELL_GRID

#define ASIZE(a) (sizeof(a) / sizeof(a[0]))

const int SCREEN_WIDTH = 1400;
const int SCREEN_HEIGHT = 900;
const int FIELD_COLS = 64;
const int FIELD_ROWS = 56;
const int CELL_SIZE = 16;
const int SNAP_TO = CELL_SIZE * 2;
const int TANK_SIZE = CELL_SIZE * 4;
const int TANK_TEXTURE_SIZE = 28;
const int PLAYER1_START_COL = 4 * 4 + 4;
const int PLAYER2_START_COL = 4 * 8 + 4;
const int PLAYER_SPEED = 300;
const int MAX_TANK_COUNT = 100;
const int MAX_BULLET_COUNT = 200;
const int MAX_EXPLOSION_COUNT = MAX_BULLET_COUNT;
const int BULLET_SPEED = 400;
const int BULLET_SIZE = 16;
const int EXPLOSION_SIZE = 64;
const float BULLET_EXPLOSION_TTL = 0.2f;

typedef enum { TPlayer1, TPlayer2, TBasic } TankType;
typedef enum { DLeft, DRight, DUp, DDown } Direction;

typedef struct {
    TankType type;
    Vector2 pos;
    Vector2 speed;
    Direction direction;
    Texture2D *texture;
    char texCol;
    char texColOffset;
    bool isMoving;
    char firedBulletCount;
    bool isFiring;
} Tank;

typedef struct {
    Vector2 pos;
    float ttl;
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
    Texture2D river;
    Texture2D blank;
    Texture2D player1Tank;
    Texture2D player2Tank;
    Texture2D bullet;
    Texture2D bulletExplosions[3];
} Textures;

typedef struct {
    Cell field[FIELD_ROWS][FIELD_COLS];
    Tank tanks[MAX_TANK_COUNT];
    Bullet bullets[MAX_BULLET_COUNT];
    Vector2 flagPos;
    int tankCount;
    CellSpec cellSpecs[CTMax];
    Explosion explosions[MAX_EXPLOSION_COUNT];
    Textures textures;
} Game;

static Game game;

void drawCell(Cell *cell) {
    Texture2D *tex = game.cellSpecs[cell->type].texture;
    int w = tex->width / 4;
    int h = tex->height / 4;
    DrawTexturePro(*tex, (Rectangle){cell->texCol * w, cell->texRow * h, w, h},
                   (Rectangle){cell->pos.x, cell->pos.y, CELL_SIZE, CELL_SIZE},
                   (Vector2){}, 0, WHITE);
#ifdef DRAW_CELL_GRID
    DrawRectangleLines(cell->pos.x, cell->pos.y, CELL_SIZE, CELL_SIZE, BLACK);
#endif
}

void drawField() {
    for (int i = 0; i < FIELD_ROWS; i++) {
        for (int j = 0; j < FIELD_COLS; j++) {
            if (game.field[i][j].type != CTForest)
                drawCell(&game.field[i][j]);
        }
    }
}

void drawForest() {
    for (int i = 0; i < FIELD_ROWS; i++) {
        for (int j = 0; j < FIELD_COLS; j++) {
            if (game.field[i][j].type == CTForest)
                drawCell(&game.field[i][j]);
        }
    }
}

void drawTank(Tank *tank) {
    static char textureRows[4] = {3, 1, 0, 2};
    Texture2D *tex = tank->texture;
    int texX = (tank->texCol + tank->texColOffset) * TANK_TEXTURE_SIZE;
    int texY = textureRows[tank->direction] * TANK_TEXTURE_SIZE;
    int drawSize = TANK_TEXTURE_SIZE * 2;
    int drawOffset = (TANK_SIZE - drawSize) / 2;
    DrawTexturePro(
        *tex, (Rectangle){texX, texY, TANK_TEXTURE_SIZE, TANK_TEXTURE_SIZE},
        (Rectangle){tank->pos.x + drawOffset, tank->pos.y + drawOffset,
                    drawSize, drawSize},
        (Vector2){}, 0, WHITE);
}

void drawTanks() {
    for (int i = 0; i < game.tankCount; i++) {
        drawTank(&game.tanks[i]);
    }
}

void drawFlag() {
    Texture2D *tex = &game.textures.flag;
    DrawTexturePro(
        *tex, (Rectangle){0, 0, tex->width, tex->height},
        (Rectangle){game.flagPos.x, game.flagPos.y, TANK_SIZE, TANK_SIZE},
        (Vector2){}, 0, WHITE);
}

void drawBullets() {
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

void drawExplosions() {
    for (int i = 0; i < MAX_EXPLOSION_COUNT; i++) {
        Explosion *e = &game.explosions[i];
        if (e->ttl <= 0)
            continue;
        int texCount = ASIZE(game.textures.bulletExplosions);
        int index = e->ttl / (BULLET_EXPLOSION_TTL / texCount);
        if (index >= texCount)
            index = texCount - 1;
        Texture2D *tex = &game.textures.bulletExplosions[texCount - index - 1];
        DrawTexturePro(
            *tex, (Rectangle){0, 0, tex->width, tex->height},
            (Rectangle){e->pos.x, e->pos.y, EXPLOSION_SIZE, EXPLOSION_SIZE},
            (Vector2){}, 0, WHITE);
    }
}

void drawGame() {
    drawField();
    drawBullets();
    drawTanks();
    drawFlag();
    drawForest();
    drawExplosions();
}

void loadTextures() {
    game.textures.flag = LoadTexture("textures/flag.png");
    game.textures.border = LoadTexture("textures/border.png");
    game.cellSpecs[CTBorder] =
        (CellSpec){.texture = &game.textures.border, .isSolid = true};
    game.textures.brick = LoadTexture("textures/brick.png");
    game.cellSpecs[CTBrick] =
        (CellSpec){.texture = &game.textures.brick, .isSolid = true};
    game.textures.concrete = LoadTexture("textures/concrete.png");
    game.cellSpecs[CTConcrete] =
        (CellSpec){.texture = &game.textures.concrete, .isSolid = true};
    game.textures.forest = LoadTexture("textures/forest.png");
    game.cellSpecs[CTForest] =
        (CellSpec){.texture = &game.textures.forest, .isPassable = true};
    game.textures.river = LoadTexture("textures/river.png");
    game.cellSpecs[CTRiver] = (CellSpec){.texture = &game.textures.river};
    game.textures.blank = LoadTexture("textures/blank.png");
    game.cellSpecs[CTBlank] =
        (CellSpec){.texture = &game.textures.blank, .isPassable = true};
    game.textures.player1Tank = LoadTexture("textures/tank1.png");
    game.textures.player2Tank = LoadTexture("textures/tank2.png");
    game.textures.bullet = LoadTexture("textures/bullet.png");
    game.textures.bulletExplosions[0] =
        LoadTexture("textures/bullet_explosion_1.png");
    game.textures.bulletExplosions[1] =
        LoadTexture("textures/bullet_explosion_2.png");
    game.textures.bulletExplosions[2] =
        LoadTexture("textures/bullet_explosion_3.png");
}

void loadField(const char *filename) {
    Buffer buf = readFile("levels/1.level");
    for (int i = 0; i < FIELD_ROWS; i++) {
        for (int j = 0; j < FIELD_COLS; j++) {
            game.field[i][j].texRow = i - 2 % 4;
            game.field[i][j].texCol = j % 4;
            if (i <= 1 || i >= FIELD_ROWS - 2 || j <= 3 ||
                j >= FIELD_COLS - 8) {
                game.field[i][j].type = CTBorder;
                continue;
            }
            int index =
                ((i - 2) / 4) * ((FIELD_COLS - 12) / 4 + 1) + ((j - 4) / 4);
            switch (buf.bytes[index]) {
            case 'b':
                game.field[i][j].type = CTBrick;
                break;
            case 'c':
                game.field[i][j].type = CTConcrete;
                break;
            case 'f':
                game.field[i][j].type = CTForest;
                break;
            case 'r':
                game.field[i][j].type = CTRiver;
                break;
            default:
                game.field[i][j].type = CTBlank;
                break;
            }
        }
    }
}

void initGame() {
    loadTextures();
    for (int i = 0; i < FIELD_ROWS; i++) {
        for (int j = 0; j < FIELD_COLS; j++) {
            game.field[i][j] =
                (Cell){.type = CTBlank,
                       .pos = (Vector2){j * CELL_SIZE, i * CELL_SIZE}};
        }
    }
    loadField("levels/1.level");
    game.tanks[0] = (Tank){.type = TPlayer1,
                           .pos = (Vector2){CELL_SIZE * PLAYER1_START_COL,
                                            CELL_SIZE * (FIELD_ROWS - 4 - 2)},
                           .direction = DUp,
                           .texture = &game.textures.player1Tank};
    game.tankCount = 1;
    game.flagPos = (Vector2){CELL_SIZE * ((FIELD_COLS - 12) / 2 - 2 + 4),
                             CELL_SIZE * (FIELD_ROWS - 4 - 2)};
}

static void finishFire(Tank *t) {
    if (!t->firedBulletCount)
        t->isFiring = false;
}

void fireBullet(Tank *t) {
    if (t->firedBulletCount >= 1 || t->isFiring)
        return;
    t->firedBulletCount++;
    t->isFiring = true;
    for (int i = 0; i < MAX_BULLET_COUNT; i++) {
        Bullet *b = &game.bullets[i];
        if (b->type != BTNone) {
            assert(i != MAX_BULLET_COUNT - 1);
            continue;
        }
        b->type = BTPlayer;
        b->direction = t->direction;
        b->tank = t;
        switch (b->direction) {
        case DRight:
            b->pos = (Vector2){t->pos.x + TANK_SIZE - BULLET_SIZE,
                               t->pos.y + TANK_SIZE / 2 - BULLET_SIZE / 2};
            b->speed = (Vector2){BULLET_SPEED, 0};
            break;
        case DLeft:
            b->pos =
                (Vector2){t->pos.x, t->pos.y + TANK_SIZE / 2 - BULLET_SIZE / 2};
            b->speed = (Vector2){-BULLET_SPEED, 0};
            break;
        case DUp:
            b->pos =
                (Vector2){t->pos.x + TANK_SIZE / 2 - BULLET_SIZE / 2, t->pos.y};
            b->speed = (Vector2){0, -BULLET_SPEED};
            break;
        case DDown:
            b->pos = (Vector2){t->pos.x + TANK_SIZE / 2 - BULLET_SIZE / 2,
                               t->pos.y + TANK_SIZE - BULLET_SIZE};
            b->speed = (Vector2){0, BULLET_SPEED};
            break;
        }
        break;
    }
}

void handleInput() {
    Tank *t = &game.tanks[0];
    if (t->isMoving)
        return;
    t->speed.x = t->speed.y = 0;
    if (IsKeyDown(KEY_RIGHT)) {
        t->isMoving = true;
        t->speed.x = PLAYER_SPEED;
        t->direction = DRight;
    } else if (IsKeyDown(KEY_LEFT)) {
        t->isMoving = true;
        t->speed.x = -PLAYER_SPEED;
        t->direction = DLeft;
    } else if (IsKeyDown(KEY_UP)) {
        t->isMoving = true;
        t->speed.y = -PLAYER_SPEED;
        t->direction = DUp;
    } else if (IsKeyDown(KEY_DOWN)) {
        t->isMoving = true;
        t->speed.y = PLAYER_SPEED;
        t->direction = DDown;
    }
    if (IsKeyDown(KEY_Z)) {
        fireBullet(t);
    } else {
        finishFire(t);
    }
}

static void destroyBullet(Bullet *b) {
    b->type = BTNone;
    b->tank->firedBulletCount--;
    int offset = (EXPLOSION_SIZE - BULLET_SIZE) / 2;
    for (int i = 0; i < MAX_EXPLOSION_COUNT; i++) {
        if (game.explosions[i].ttl <= 0) {
            game.explosions[i].ttl = BULLET_EXPLOSION_TTL;

            game.explosions[i].pos =
                (Vector2){b->pos.x - offset, b->pos.y - offset};
            break;
        }
    }
}

static void destroyBrick(int row, int col) {
    if (row < 0 || row >= FIELD_ROWS || col < 0 || col >= FIELD_COLS)
        return;
    if (game.field[row][col].type == CTBrick)
        game.field[row][col].type = CTBlank;
}

static void checkBulletRows(Bullet *b, int startRow, int endRow, int col) {
    for (int r = startRow; r <= endRow; r++) {
        CellType cellType = game.field[r][col].type;
        if (game.cellSpecs[cellType].isSolid) {
            destroyBullet(b);
            for (int rr = startRow - 1; rr <= endRow + 1; rr++)
                destroyBrick(rr, col);
            return;
        }
    }
}

static void checkBulletCols(Bullet *b, int startCol, int endCol, int row) {
    for (int c = startCol; c <= endCol; c++) {
        CellType cellType = game.field[row][c].type;
        if (game.cellSpecs[cellType].isSolid) {
            destroyBullet(b);
            for (int cc = startCol - 1; cc <= endCol + 1; cc++)
                destroyBrick(row, cc);
            return;
        }
    }
}

void checkBulletCollision(Bullet *b) {
    switch (b->direction) {
    case DRight: {
        int startRow = ((int)b->pos.y) / CELL_SIZE;
        int endRow = ((int)b->pos.y + BULLET_SIZE - 1) / CELL_SIZE;
        int col = ((int)b->pos.x + BULLET_SIZE - 1) / CELL_SIZE;
        checkBulletRows(b, startRow, endRow, col);
        return;
    }
    case DLeft: {
        int startRow = ((int)b->pos.y) / CELL_SIZE;
        int endRow = ((int)b->pos.y + BULLET_SIZE - 1) / CELL_SIZE;
        int col = ((int)b->pos.x) / CELL_SIZE;
        checkBulletRows(b, startRow, endRow, col);
        return;
    }
    case DUp: {
        int startCol = ((int)b->pos.x) / CELL_SIZE;
        int endCol = ((int)b->pos.x + BULLET_SIZE - 1) / CELL_SIZE;
        int row = ((int)(b->pos.y)) / CELL_SIZE;
        checkBulletCols(b, startCol, endCol, row);
        return;
    }
    case DDown: {
        int startCol = ((int)b->pos.x) / CELL_SIZE;
        int endCol = ((int)b->pos.x + BULLET_SIZE - 1) / CELL_SIZE;
        int row = ((int)b->pos.y + BULLET_SIZE - 1) / CELL_SIZE;
        checkBulletCols(b, startCol, endCol, row);
        return;
    }
    }
}

void checkTankCollision(Tank *tank) {
    switch (tank->direction) {
    case DRight: {
        int startRow = ((int)tank->pos.y) / CELL_SIZE;
        int endRow = ((int)tank->pos.y + TANK_SIZE - 1) / CELL_SIZE;
        int col = ((int)tank->pos.x + TANK_SIZE - 1) / CELL_SIZE;
        for (int r = startRow; r <= endRow; r++) {
            CellType cellType = game.field[r][col].type;
            if (!game.cellSpecs[cellType].isPassable) {
                tank->isMoving = false;
                tank->pos.x = game.field[r][col].pos.x - TANK_SIZE;
            }
        }
        return;
    }
    case DLeft: {
        int startRow = ((int)tank->pos.y) / CELL_SIZE;
        int endRow = ((int)tank->pos.y + TANK_SIZE - 1) / CELL_SIZE;
        int col = ((int)tank->pos.x) / CELL_SIZE;
        for (int r = startRow; r <= endRow; r++) {
            CellType cellType = game.field[r][col].type;
            if (!game.cellSpecs[cellType].isPassable) {
                tank->isMoving = false;
                tank->pos.x = game.field[r][col].pos.x + CELL_SIZE;
            }
        }
        return;
    }
    case DUp: {
        int startCol = ((int)tank->pos.x) / CELL_SIZE;
        int endCol = ((int)tank->pos.x + TANK_SIZE - 1) / CELL_SIZE;
        int row = ((int)(tank->pos.y)) / CELL_SIZE;
        for (int c = startCol; c <= endCol; c++) {
            CellType cellType = game.field[row][c].type;
            if (!game.cellSpecs[cellType].isPassable) {
                tank->isMoving = false;
                tank->pos.y = game.field[row][c].pos.y + CELL_SIZE;
            }
        }
        return;
    }
    case DDown: {
        int startCol = ((int)tank->pos.x) / CELL_SIZE;
        int endCol = ((int)tank->pos.x + TANK_SIZE - 1) / CELL_SIZE;
        int row = ((int)tank->pos.y + TANK_SIZE - 1) / CELL_SIZE;
        for (int c = startCol; c <= endCol; c++) {
            CellType cellType = game.field[row][c].type;
            if (!game.cellSpecs[cellType].isPassable) {
                tank->isMoving = false;
                tank->pos.y = game.field[row][c].pos.y - TANK_SIZE;
            }
        }
        return;
    }
    }
}

void updateTankState(float time, Tank *t) {
    if (!t->isMoving)
        return;
    t->texColOffset = (t->texColOffset + 1) % 2;
    switch (t->direction) {
    case DLeft: {
        int toSnap = ((int)t->pos.x) % SNAP_TO;
        int toMove = time * -t->speed.x;
        if (toSnap && toSnap <= toMove) {
            t->pos.x -= toSnap;
            t->isMoving = false;
        } else {
            t->pos.x -= toMove;
        }
        break;
    }
    case DRight: {
        int toSnap = SNAP_TO - ((int)t->pos.x) % SNAP_TO;
        int toMove = time * t->speed.x;
        if (toSnap <= toMove) {
            t->pos.x += toSnap;
            t->isMoving = false;
        } else {
            t->pos.x += toMove;
        }
        break;
    }
    case DUp: {
        int toSnap = ((int)t->pos.y) % SNAP_TO;
        int toMove = time * -t->speed.y;
        if (toSnap && toSnap <= toMove) {
            t->pos.y -= toSnap;
            t->isMoving = false;
        } else {
            t->pos.y -= toMove;
        }
        break;
    }
    case DDown: {
        int toSnap = SNAP_TO - ((int)t->pos.y) % SNAP_TO;
        int toMove = time * t->speed.y;
        if (toSnap <= toMove) {
            t->pos.y += toSnap;
            t->isMoving = false;
        } else {
            t->pos.y += toMove;
        }
        break;
    }
    }
    checkTankCollision(&game.tanks[0]);
}

void updateBulletsState(float time) {
    for (int i = 0; i < MAX_BULLET_COUNT; i++) {
        Bullet *b = &game.bullets[i];
        if (b->type == BTNone)
            continue;
        b->pos.x += (b->speed.x * time);
        b->pos.y += (b->speed.y * time);
        checkBulletCollision(b);
    }
}

void updateExplosionsState(float time) {
    for (int i = 0; i < MAX_EXPLOSION_COUNT; i++) {
        if (game.explosions[i].ttl > 0) {
            game.explosions[i].ttl -= time;
        }
    }
}

void updateGameState(float time) {
    updateExplosionsState(time);
    updateBulletsState(time);
    updateTankState(time, &game.tanks[0]);
}

int main(void) {

    SetTraceLogLevel(LOG_NONE);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Battle City 4000");

    initGame();

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        handleInput();
        updateGameState(GetFrameTime());
        BeginDrawing();
        ClearBackground(BLACK);
        drawGame();
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
