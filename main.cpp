#include <iostream>
#include <raylib.h>
#include <deque>
#include <raymath.h>
#include <string>
using namespace std;

const Color my_background = {200, 255, 255, 45};
const Color food_color = {50, 25, 255, 150};
const Color snake_color = {10, 190, 255, 150};
const Color border_color = {14, 10, 5, 169};
const Color score_bar_color = {50, 100, 150, 255};
const Color score_text_color = {255, 255, 255, 255};
const Color credit_bar_color = {35, 70, 105, 255};

const int cell_size = 30;
const int cell_count = 25;
const int top_bar_height = 50;     // space reserved for the score bar
const int bottom_bar_height = 30;  // space reserved for the credit bar
const int border_thickness = 4;    // thickness of the play-area border

double last_update_time = 0;

bool eventTrigger(double interval) {
    double currentTime = GetTime();
    if (currentTime - last_update_time >= interval) {
        last_update_time = currentTime;
        return true;
    }
    return false;
}

bool isElementInDeque(Vector2 element, deque<Vector2> body) {
    for (int i = 0; i < body.size(); i++) {
        if (Vector2Equals(element, body[i])) {
            return true;
        }
    }
    return false;
}

class Food {
    public:
    Vector2 position = {5, 6};

    Food(deque<Vector2> snakeBody) {
        position = GenerateCoordinates(snakeBody);
    }

    void Draw() {
        // +top_bar_height shifts everything down to make room for the score bar
        DrawRectangle(position.x*cell_size, position.y*cell_size+top_bar_height, cell_size, cell_size, food_color);
    }

    Vector2 generateRandom() {
        float x = GetRandomValue(0, cell_count-1);
        float y = GetRandomValue(0, cell_count-1);
        return Vector2{x, y};
    }

    Vector2 GenerateCoordinates(deque<Vector2> snakeBody) {
        Vector2 Position = generateRandom();

        while (isElementInDeque(Position, snakeBody)) {
            Position = generateRandom();
        }

        return Position;
    }
};

class Snake {
    public:
    deque<Vector2> body = {Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9}};
    Vector2 direction = {1, 0};
    bool isSegment = false;

    void Draw() {
        for (int i = 0; i < body.size(); i++) {
            float x = body[i].x;
            float y = body[i].y;
            Rectangle segment = Rectangle{x*cell_size, y*cell_size+top_bar_height, (float)cell_size, (float)cell_size};
            DrawRectangleRounded(segment, 0.66, 10, snake_color);
        }
    }

    void Move() {
        body.push_front(Vector2Add(body[0], direction));

        if (isSegment) {
            isSegment = false;
        } else {
            body.pop_back();
        }
    }

    void reset() {
        body = {Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9}};
        direction = {1, 0};
    }
};

class Game {
    public:
    Snake snake = Snake();
    Food food = Food(snake.body);
    bool running = true;
    int score = 0;

    void Draw() {
        DrawBorder();
        food.Draw();
        snake.Draw();
        DrawScoreBar();
        DrawCreditBar();

        if (!running) {
            DrawGameOverMessage();
        }
    }

    void Update() {
        if (running) {
            snake.Move();
            isCollisionFood(snake.body);
            isCollisionWall();
            isCollisionBody();
        }
    }

    void isCollisionFood(deque<Vector2> snakeBody) {
        if (Vector2Equals(snake.body[0], food.position)) {
            food.position = food.GenerateCoordinates(snakeBody);
            snake.isSegment = true;
            score++;
        }
    }

    void isCollisionWall() {
        if (snake.body[0].x == cell_count || snake.body[0].x == -1) {
            GameOver();
        }
        if (snake.body[0].y == cell_count || snake.body[0].y == -1) {
            GameOver();
        }
    }

    void isCollisionBody() {
        deque<Vector2> headlessBody = snake.body;
        headlessBody.pop_front();
        if (isElementInDeque(snake.body[0], headlessBody)) {
            GameOver();
        }
    }

    void GameOver() {
        snake.reset();
        food.position = food.GenerateCoordinates(snake.body);
        running = false;
        score = 0;
    }

    void DrawBorder() {
        Rectangle playArea = {0, (float)top_bar_height, (float)cell_size*cell_count, (float)cell_size*cell_count};
        DrawRectangleLinesEx(playArea, border_thickness, border_color);
    }

    void DrawScoreBar() {
        DrawRectangle(0, 0, cell_size*cell_count, top_bar_height, score_bar_color);
        DrawText("SNAKE GAME", 15, 13, 24, score_text_color);

        string scoreText = "Score: " + to_string(score);
        int textWidth = MeasureText(scoreText.c_str(), 24);
        DrawText(scoreText.c_str(), cell_size * cell_count - textWidth - 15, 13, 24, score_text_color);
    }

    void DrawCreditBar() {
        int barY = top_bar_height + cell_size * cell_count;
        DrawRectangle(0, barY, cell_size * cell_count, bottom_bar_height, credit_bar_color);

        const char* credit = "Developed by RamaNeeraj";
        int fontSize = 16;
        int textWidth = MeasureText(credit, fontSize);
        int x = (cell_size * cell_count - textWidth) / 2;
        int y = barY + (bottom_bar_height - fontSize) / 2;
        DrawText(credit, x, y, fontSize, score_text_color);
    }

    void DrawGameOverMessage() {
        const char* msg = "GAME OVER - Press an arrow key to restart";
        int fontSize = 20;
        int textWidth = MeasureText(msg, fontSize);
        int x = (cell_size * cell_count - textWidth) / 2;
        int y = top_bar_height + (cell_size * cell_count) / 2 - fontSize / 2;

        // small backing box so the text stays readable over the snake/border
        DrawRectangle(x - 10, y - 8, textWidth + 20, fontSize + 16, Color{0, 0, 0, 180});
        DrawText(msg, x, y, fontSize, RAYWHITE);
    }
};

int main() {
    InitWindow(cell_size * cell_count, cell_size * cell_count + top_bar_height + bottom_bar_height, "Snake Game");
    SetTargetFPS(60); // render at 60 FPS; movement speed is controlled separately below

    Game game = Game();

    while (WindowShouldClose() == false) {
        BeginDrawing();

        if (eventTrigger(0.1)) {
            game.Update();
        }

        if (IsKeyPressed(KEY_UP) && game.snake.direction.y != 1) {
            game.snake.direction = {0, -1};
            game.running = true;
        }
        if (IsKeyPressed(KEY_DOWN) && game.snake.direction.y != -1) {
            game.snake.direction = {0, 1};
            game.running = true;
        }
        if (IsKeyPressed(KEY_RIGHT) && game.snake.direction.x != -1) {
            game.snake.direction = {1, 0};
            game.running = true;
        }
        if (IsKeyPressed(KEY_LEFT) && game.snake.direction.x != 1) {
            game.snake.direction = {-1, 0};
            game.running = true;
        }

        ClearBackground(my_background);
        game.Draw();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}