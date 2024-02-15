#include <iostream>
#include "raylib.h"
#include <vector>
#include <algorithm>

struct pacman_properties
{
    float x, y;
    float radius;
    float speed;
    Vector2 velocity;// vector - distanta + sens al miscarii; are ca parametrii un PosX si PosY
    /* - pentru a simula o miscare continua a lui pacman
       - pentru a evita ciocnirea/ suprapunerea cu peretii 
       (overlapping / object collision between a circle and a rectangle) */
};

struct wall_properties
{
    float x, y;
    float width, height;// vor fi specificate atat latimea, cat si inaltimea, chiar daca au aceeasi valoare
    Rectangle rect;// e o functie de tip rectangle: definire a pozitiei si dimensiunilor, specifica fiecarui perete
};

struct pellet_properties
{
    float x, y;
    float radius;
};

struct ghost_properties
{
    float x, y;
    float speed;
    Vector2 velocity;
    Rectangle body_rect;
    Vector2 head_center;
    float head_radius;
    /*Vector2 range;
    float range_radius;
    bool enter;
    bool at_junction;
    int current_direction;*/
};

int grid_map[21][21]
{
        {2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2},
        {2,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,2},
        {2,1,0,1,1,0,1,1,1,0,1,0,1,1,1,0,1,1,0,1,2},
        {2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2},
        {2,1,0,1,1,0,1,0,1,1,1,1,1,0,1,0,1,1,0,1,2},
        {2,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,2},
        {2,1,1,1,1,0,1,1,1,2,1,2,1,1,1,0,1,1,1,1,2},
        {2,2,2,2,1,0,1,2,2,2,2,2,2,2,1,0,1,2,2,2,2},
        {1,1,1,1,1,0,1,2,1,1,2,1,1,2,1,0,1,1,1,1,1},
        {2,2,2,2,2,0,2,2,1,2,2,2,1,2,2,0,2,2,2,2,2},
        {1,1,1,1,1,0,1,2,1,1,1,1,1,2,1,0,1,1,1,1,1},
        {2,2,2,2,1,0,1,2,2,2,2,2,2,2,1,0,1,2,2,2,2},
        {2,1,1,1,1,0,1,2,1,1,1,1,1,2,1,0,1,1,1,1,2},
        {2,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,2},
        {2,1,0,1,1,0,1,1,1,0,1,0,1,1,1,0,1,1,0,1,2},
        {2,1,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,1,2},
        {2,1,1,0,1,0,1,0,1,1,1,1,1,0,1,0,1,0,1,1,2},
        {2,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,2},
        {2,1,0,1,1,1,1,1,1,0,1,0,1,1,1,1,1,1,0,1,2},
        {2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2},
        {2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2}
};

void norm_velocity(float& velocity_x, float& velocity_y)
{
    /*  - normalizeaza vectorul viteza al lui pacman, astfel incat lungimea acestuia sa fie egala cu 1.
        - presupune impartirea componentelor x si y ale vectorului viteza la marimea acestuia, daca este mai mare decat 0 */
    float magnitude = sqrt(pow(velocity_x, 2) + pow(velocity_y, 2));
    if (magnitude > 0)
    {
        velocity_x /= magnitude;
        velocity_y /= magnitude;
    }
}
void check_collision(float& radius, float& x, float& y, Vector2& velocity, wall_properties* walls, const int& max_walls)
{
    /* evitare collision : referitor la traiectoria vectorului de velocity, se va determina o pozitie potentiala a
    centrului cercului proiectat pe o dreapta paralela cu Ox/ Oy pe un patrat */
    Vector2 potential_position = { x + velocity.x, y + velocity.y };// 2 coord: posX, posY

    // aflarea limitelor in care se poate deplasa pacman (boundaries)
    Rectangle boundary = { potential_position.x - radius, potential_position.y - radius, radius * 2, radius * 2 };

    // check for wall collision
    bool collision_detected = false;
    for (int i = 0; i < max_walls; i++)
    {
        Rectangle wall_rect = { walls[i].x, walls[i].y, walls[i].width, walls[i].height };
        if (CheckCollisionRecs(boundary, wall_rect))// functie integrata raylib
        {
            collision_detected = true;

            // actualizarea pozitiei vectorilor in sens invers
            Vector2 overlap = { 0, 0 };// partea de suprapunere
            if (velocity.x > 0) // compararea cu 0 indica sensul deplasarii pentru ambele axe
            {
                overlap.x = walls[i].x - radius - x;
            }
            else if (velocity.x < 0)
            {
                overlap.x = walls[i].x + walls[i].width + radius - x;
            }
            if (velocity.y > 0)
            {
                overlap.y = walls[i].y - radius - y;
            }
            else if (velocity.y < 0)
            {
                overlap.y = walls[i].y + walls[i].height + radius - y;
            }
            potential_position.x = -overlap.x;// se va sustrage valoarea determinata in variabila overlap
            potential_position.y = -overlap.y;

            // reinitializare velocity
            if (abs(overlap.x) > abs(overlap.y)) // compararea modulelor
            {
                velocity.x = 0;
            }
            else
            {
                velocity.y = 0;
            }
            break;// break out of the collision check loop
        }
    }

    // actualizarea pozitiei lui pacman doar daca nu a avut loc o ciocnire
    if (!collision_detected)
    {
        x = potential_position.x;
        y = potential_position.y;
    }
}

/*void update_ghost_position(ghost_properties& ghosts, pacman_properties& pacman, wall_properties* walls, const int& max_walls, Vector2 random_target)// CHASE MODE
{
    // a intrat in maze
    if (ghosts.enter)
    {
        Rectangle pacman_body = { pacman.x, pacman.y, pacman.radius, pacman.radius };

        // pacman e in raza de actiune: ghost.range_radius
        if (CheckCollisionCircleRec(ghosts.head_center, ghosts.range_radius, pacman_body))
        {
            // modificarea directiei de viteza in directia lui pacman, in fct de pozitia lui
            float dx = pacman.x - ghosts.x;
            float dy = pacman.y - ghosts.y;
            float distance = sqrt(dx * dx + dy * dy);
            ghosts.velocity.x = ghosts.speed * (dx / distance);
            ghosts.velocity.y = ghosts.speed * (dy / distance);
        }
        else
        {
            // fiecare fantoma va urmari un alt target, localizat in unul dintre cele patru colturi ale hartii
            float dx = random_target.x - ghosts.x;
            float dy = random_target.y - ghosts.y;
            float distance = sqrt(dx * dx + dy * dy);
            ghosts.velocity.x = ghosts.speed * (dx / distance);
            ghosts.velocity.y = ghosts.speed * (dy / distance);
        }
        norm_velocity(ghosts.velocity.x, ghosts.velocity.y);

        // wall collision SAU apel subprogram
        for (int i = 0; i < max_walls; i++)
        {
            if (CheckCollisionCircleRec({ ghosts.x, ghosts.y }, ghosts.head_radius, walls[i].rect))
            {
                ghosts.x -= ghosts.velocity.x;
                ghosts.y -= ghosts.velocity.y;
                ghosts.velocity.x = -ghosts.velocity.x;
                ghosts.velocity.y = -ghosts.velocity.y;
                norm_velocity(ghosts.velocity.x, ghosts.velocity.y);
                break;
            }
        }
    }
    else
    {
        // fantoma va intra in maze, inainte de CHASE MODE
        // update: gresit
        if (ghosts.y <= GetScreenHeight() / 2 - 30)
        {
            ghosts.enter = true;
        }
        if (ghosts.x < GetScreenWidth() / 2)
        { 
            if (ghosts.x != GetScreenWidth() / 2)
            {
                ghosts.velocity.x = ghosts.speed;// sens pozitiv
                ghosts.velocity.y = 0;
            }
        }
        else if (ghosts.x > GetScreenWidth() / 2)
        {
            if (ghosts.x != GetScreenWidth() / 2)
            {
                ghosts.velocity.x = -ghosts.speed;// sens negativ
                ghosts.velocity.y = 0;
            }
        }
        if (ghosts.y == GetScreenHeight() / 2 - 30)
        {
            if (ghosts.y != GetScreenHeight() / 2)
            {
                ghosts.velocity.x = 0;
                ghosts.velocity.y = -ghosts.speed;
            }
        }
        
    }
    // updating the body position
    ghosts.x += ghosts.velocity.x;
    ghosts.y += ghosts.velocity.y;
    ghosts.body_rect.x = ghosts.x - ghosts.body_rect.width / 2;
    ghosts.body_rect.y = ghosts.y - ghosts.body_rect.height / 2;
    ghosts.head_center = { ghosts.x, ghosts.y };
}*/
/*void current_dir(ghost_properties& ghosts)
{
    // directia curenta a fantomei
    if (ghosts.velocity.x > 0)// RIGHT
    { 
        ghosts.current_direction = 3;
    }
    else if (ghosts.velocity.x < 0)// LEFT
    { 
        ghosts.current_direction = 2;
    }
    else if (ghosts.velocity.y > 0)// DOWN
    { 
        ghosts.current_direction = 1;
    }
    else if (ghosts.velocity.y < 0)// UP
    {
        ghosts.current_direction = 0;
    }
}*/
/*void is_at_junction(ghost_properties& ghosts, int grid_map[21][21])// daca fantoma este la o intersectie (are 1 sau cel mult 4 optiuni de intoarcere)
{
    const Vector2 MOVEMENT_OPTIONS[4] = { {0, -1}, {0, 1}, {-1, 0}, {1, 0} };// UP, DOWN, LEFT, RIGHT
    int num_valid_directions = 0;

    for (int i = 0; i < 4; i++)
    {
        float check_x = ghosts.x + MOVEMENT_OPTIONS[i].x * ghosts.speed;
        float check_y = ghosts.y + MOVEMENT_OPTIONS[i].y * ghosts.speed;

        // convertirea pozitiei exprimate in pixeli in una coresp indicilor matricii grid_map[][]
        int check_gridX = (ghosts.x + 15) / 30;// 15 e un tile centre offset
        int check_gridY = (ghosts.y + 15) / 30;

        if (check_gridX >= 0 && check_gridX < 21 && check_gridY >= 0 && check_gridY < 21)
        {
            if (grid_map[check_gridY][check_gridX] == 0) // Walkable tile
            {
                num_valid_directions++;
            }
        }
    }

    if (num_valid_directions >= 2)// e la o intersectie pt cel putin 2 directii valide
    {
        ghosts.at_junction = true;
    }
    else ghosts.at_junction = false;
}*/
/*void ghost_move(ghost_properties& ghosts, wall_properties* walls, const int& max_walls, int grid_map[21][21])// randomized movement: FRIGHTENED MODE
{
    int direction=0;

    // cele 4 directii
    const int UP = 0;
    const int DOWN = 1;
    const int LEFT = 2;
    const int RIGHT = 3;
    const Vector2 MOVEMENT_OPTIONS[4] = { {0, -1}, {0, 1}, {-1, 0}, {1, 0} };

    is_at_junction(ghosts, grid_map);

    // in functie de localizare, fantoma fie va alege o singura directie din cele num_valid_directions directii (randomized), fie continua directia curenta pana la mom localizarii la un junction
    if (ghosts.at_junction)
    {
        // randomized direction
        std::vector<int> valid_directions;
        for (int i = 0; i < 4; i++)
        {
            float new_x = ghosts.x + MOVEMENT_OPTIONS[i].x * ghosts.speed;
            float new_y = ghosts.y + MOVEMENT_OPTIONS[i].y * ghosts.speed;
            int new_gridX = (ghosts.x + 15) / 30;// convertire indici grid_map[][]
            int new_gridY = (ghosts.y + 15) / 30;
            if (new_gridX >= 0 && new_gridX < 21 && new_gridY >= 0 && new_gridY < 21)
            {
                if (grid_map[new_gridY][new_gridX] == 0) // Wwalkable tile
                {
                    valid_directions.push_back(i);
                }
            }
        }
        do {
            direction = rand() % 4;// indice intre 0 si 3
            // daca indicele nu e valid, adica nu apartine vectorului de directii posibile pentru fiecare caz
            if (std::find(valid_directions.begin(), valid_directions.end(), direction) == valid_directions.end()) 
            {
                direction = -1;// directia e invalida
            }
        } while (direction == -1);// conditie oprire????
        ghosts.velocity.x = MOVEMENT_OPTIONS[direction].x * ghosts.speed;
        ghosts.velocity.y = MOVEMENT_OPTIONS[direction].y * ghosts.speed;
        ghosts.current_direction = direction;
    }
    else // continua directia curenta
    {
        current_dir(ghosts);
        if (ghosts.current_direction >= 0 && ghosts.current_direction < 4) 
        {
            ghosts.velocity.x = MOVEMENT_OPTIONS[ghosts.current_direction].x * ghosts.speed;
            ghosts.velocity.y = MOVEMENT_OPTIONS[ghosts.current_direction].y * ghosts.speed;
        }
    }

    // noua pozitie
    float new_x = ghosts.x + ghosts.velocity.x;
    float new_y = ghosts.y + ghosts.velocity.y;

    check_collision(ghosts.head_radius, new_x, new_y, ghosts.velocity, walls, max_walls);

    // body rectangle and head center new updated positions
    ghosts.body_rect.x = new_x - ghosts.body_rect.width / 2;
    ghosts.body_rect.y = new_y - ghosts.body_rect.height / 2;
    ghosts.head_center = { new_x, new_y };
}*/

void UP(ghost_properties& ghosts)
{
    ghosts.velocity.x = 0;
    ghosts.velocity.y = -ghosts.speed;
    ghosts.y += ghosts.velocity.y;
}
void DOWN(ghost_properties& ghosts)
{
    ghosts.velocity.x = 0;
    ghosts.velocity.y = ghosts.speed;
    ghosts.y += ghosts.velocity.y;
}
void LEFT(ghost_properties& ghosts)
{
    ghosts.velocity.x = -ghosts.speed;
    ghosts.velocity.y = 0;
    ghosts.x += ghosts.velocity.x;
}
void RIGHT(ghost_properties& ghosts)
{
    ghosts.velocity.x = ghosts.speed;
    ghosts.velocity.y = 0;
    ghosts.x += ghosts.velocity.x;
}
// fiecare fantoma are un path predefinit pe care il urmeaza: SCATTER MODE
void Inky(ghost_properties& ghosts)// indice fantoma 3
{
    if (ghosts.y == 225 && ghosts.x > 225 && ghosts.x <= 315)
    {
        LEFT(ghosts);
    }
    else if (ghosts.x == 225 && ghosts.y >= 225 && ghosts.y < 285)
    {
        DOWN(ghosts);
    }
    else if (ghosts.y == 285 && ghosts.x <= 225 && ghosts.x > 165)
    {
        LEFT(ghosts);
    }
    else if (ghosts.x == 165 && ghosts.y <= 285 && ghosts.y > 165)
    {
        UP(ghosts);
    }
    else if (ghosts.y == 165 && ghosts.x <= 165 && ghosts.x > 75)
    {
        LEFT(ghosts);
    }
    else if (ghosts.x == 75 && ghosts.y <= 165 && ghosts.y > 105)
    {
        UP(ghosts);
    }
    else if (ghosts.y == 105 && ghosts.x >= 75 && ghosts.x < 555)
    {
        RIGHT(ghosts);
    }
    else if (ghosts.x == 555 && ghosts.y >= 105 && ghosts.y < 165)
    {
        DOWN(ghosts);
    }
    else if (ghosts.y == 165 && ghosts.x <= 555 && ghosts.x > 465)
    {
        LEFT(ghosts);
    }
    else if (ghosts.x == 465 && ghosts.y >= 165 && ghosts.y < 285)
    {
        DOWN(ghosts);
    }
    else if (ghosts.y == 285 && ghosts.x <= 465 && ghosts.x > 405)
    {
        LEFT(ghosts);
    }
    else if (ghosts.x == 405 && ghosts.y <= 285 && ghosts.y > 225)
    {
        UP(ghosts);
    }
    else if (ghosts.y == 225 && ghosts.x <= 405 && ghosts.x > 315)
    {
        LEFT(ghosts);
    }

    ghosts.body_rect.x = ghosts.x - ghosts.body_rect.width / 2;
    ghosts.body_rect.y = ghosts.y - ghosts.body_rect.height / 2;
    ghosts.head_center = { ghosts.x, ghosts.y };
}
void Pinky(ghost_properties& ghosts)// indice fantoma 0
{
    if (ghosts.x == 315 && ghosts.y <= 285 && ghosts.y > 225)
    {
        UP(ghosts);
    }
    else if (ghosts.y == 225 && ghosts.x >= 315 && ghosts.x < 405)
    {
        RIGHT(ghosts);
    }
    else if (ghosts.x == 405 && ghosts.y >= 225 && ghosts.y < 405)
    {
        DOWN(ghosts);
    }
    else if (ghosts.y == 405 && ghosts.x >= 405 && ghosts.x < 555)
    {
        RIGHT(ghosts);
    }
    else if (ghosts.x == 555 && ghosts.y >= 405 && ghosts.y < 465)
    {
        DOWN(ghosts);
    }
    else if (ghosts.y == 465 && ghosts.x <= 555 && ghosts.x > 525)
    {
        LEFT(ghosts);
    }
    else if (ghosts.x == 525 && ghosts.y >= 465 && ghosts.y < 525)
    {
        DOWN(ghosts);
    }
    else if (ghosts.y == 525 && ghosts.x >= 525 && ghosts.x < 555)
    {
        RIGHT(ghosts);
    }
    else if (ghosts.x == 555 && ghosts.y >= 525 && ghosts.y < 585)
    {
        DOWN(ghosts);
    }
    else if (ghosts.y == 585 && ghosts.x <= 555 && ghosts.x > 345)
    {
        LEFT(ghosts);
    }
    else if (ghosts.x == 345 && ghosts.y <= 585 && ghosts.y > 525)
    {
        UP(ghosts);
    }
    else if (ghosts.y == 525 && ghosts.x >= 345 && ghosts.x < 405)
    {
        RIGHT(ghosts);
    }
    else if (ghosts.x == 405 && ghosts.y <= 525 && ghosts.y > 465)
    {
        UP(ghosts);
    }
    else if (ghosts.y == 465 && ghosts.x <= 405 && ghosts.x > 345)
    {
        LEFT(ghosts);
    } 
    else if (ghosts.x == 345 && ghosts.y <= 465 && ghosts.y > 405)
    {
        UP(ghosts);
    }
    else if (ghosts.y == 405 && ghosts.x >= 345 && ghosts.x < 405)
    {
        RIGHT(ghosts);
    }

    ghosts.body_rect.x = ghosts.x - ghosts.body_rect.width / 2;
    ghosts.body_rect.y = ghosts.y - ghosts.body_rect.height / 2;
    ghosts.head_center = { ghosts.x, ghosts.y };
}
void Blinky(ghost_properties& ghosts)// indice fantoma 1
{
    if (ghosts.y == 285 && ghosts.x <= 345 && ghosts.x > 315)
    {
        LEFT(ghosts);
    }
    else if (ghosts.x == 315 && ghosts.y <= 285 && ghosts.y > 225)
    {
        UP(ghosts);
    }
    else if (ghosts.y == 225 && ghosts.x <= 315 && ghosts.x > 285)
    {
        LEFT(ghosts);
    }
    else if (ghosts.x == 285 && ghosts.y <= 225 && ghosts.y > 165)
    {
        UP(ghosts);
    }
    else if (ghosts.y == 165 && ghosts.x <= 285 && ghosts.x > 225)
    {
        LEFT(ghosts);
    }
    else if (ghosts.x == 225 && ghosts.y <= 165 && ghosts.y > 105)
    {
        UP(ghosts);
    }
    else if (ghosts.y == 105 && ghosts.x <= 225 && ghosts.x > 165)
    {
        LEFT(ghosts);
    }
    else if (ghosts.x == 165 && ghosts.y <= 105 && ghosts.y > 45)
    {
        UP(ghosts);
    }
    else if (ghosts.y == 45 && ghosts.x >= 165 && ghosts.x < 285)
    {
        RIGHT(ghosts);
    }
    else if (ghosts.x == 285 && ghosts.y >= 45 && ghosts.y < 105)
    {
        DOWN(ghosts);
    }
    else if (ghosts.y == 105 && ghosts.x >= 285 && ghosts.x < 345)
    {
        RIGHT(ghosts);
    }
    else if (ghosts.x == 345 && ghosts.y <= 105 && ghosts.y > 45)
    {
        UP(ghosts);
    }
    else if (ghosts.y == 45 && ghosts.x >= 345 && ghosts.x < 465)
    {
        RIGHT(ghosts);
    }
    else if (ghosts.x == 465 && ghosts.y >= 45 && ghosts.y < 105)
    {
        DOWN(ghosts);
    }
    else if (ghosts.y == 105 && ghosts.x <= 465 && ghosts.x > 405)
    {
        LEFT(ghosts);
    }
    else if (ghosts.x == 405 && ghosts.y >= 105 && ghosts.y < 165)
    {
        DOWN(ghosts);
    }
    else if (ghosts.y == 165 && ghosts.x <= 405 && ghosts.x > 345)
    {
        LEFT(ghosts);
    }
    else if (ghosts.x == 345 && ghosts.y >= 165 && ghosts.y < 225)
    {
        DOWN(ghosts);
    }
    else if (ghosts.y == 225 && ghosts.x <= 345 && ghosts.x > 315)
    {
        LEFT(ghosts);
    }

    ghosts.body_rect.x = ghosts.x - ghosts.body_rect.width / 2;
    ghosts.body_rect.y = ghosts.y - ghosts.body_rect.height / 2;
    ghosts.head_center = { ghosts.x, ghosts.y };
}
void Clyde(ghost_properties& ghosts)// indice fantoma 2
{
    if (ghosts.y == 285 && ghosts.x >= 285 && ghosts.x < 315)
    {
        RIGHT(ghosts);
    }
    else if (ghosts.x == 315 && ghosts.y <= 285 && ghosts.y > 225)
    {
        UP(ghosts);
    }
    else if (ghosts.y == 225 && ghosts.x <= 315 && ghosts.x > 225)
    {
        LEFT(ghosts);
    }
    else if (ghosts.x == 225 && ghosts.y >= 225 && ghosts.y < 405)
    {
        DOWN(ghosts);
    }
    else if (ghosts.y == 405 && ghosts.x <= 225 && ghosts.x > 75)
    {
        LEFT(ghosts);
    }
    else if (ghosts.x == 75 && ghosts.y >= 405 && ghosts.y < 465)
    {
        DOWN(ghosts);
    }
    else if (ghosts.y == 465 && ghosts.x >= 75 && ghosts.x < 105)
    {
        RIGHT(ghosts);
    }
    else if (ghosts.x == 105 && ghosts.y >= 465 && ghosts.y < 525)
    {
        DOWN(ghosts);
    } 
    else if (ghosts.y == 525 && ghosts.x <= 105 && ghosts.x > 75)
    {
        LEFT(ghosts);
    }
    else if (ghosts.x == 75 && ghosts.y >= 525 && ghosts.y < 585)
    {
        DOWN(ghosts);
    }
    else if (ghosts.y == 585 && ghosts.x >= 75 && ghosts.x < 285)
    {
        RIGHT(ghosts);
    }
    else if (ghosts.x == 285 && ghosts.y <= 585 && ghosts.y > 525)
    {
        UP(ghosts);
    }
    else if (ghosts.y == 525 && ghosts.x <= 285 && ghosts.x > 225)
    {
        LEFT(ghosts);
    }
    else if (ghosts.x == 225 && ghosts.y <= 525 && ghosts.y > 465)
    {
        UP(ghosts);
    }
    else if (ghosts.y == 465 && ghosts.x >= 225 && ghosts.x < 285)
    {
        RIGHT(ghosts);
    }
    else if (ghosts.x == 285 && ghosts.y <= 465 && ghosts.y > 405)
    {
        UP(ghosts);
    }
    else if (ghosts.y == 405 && ghosts.x <= 285 && ghosts.x > 225)
    {
        LEFT(ghosts);
    }

    ghosts.body_rect.x = ghosts.x - ghosts.body_rect.width / 2;
    ghosts.body_rect.y = ghosts.y - ghosts.body_rect.height / 2;
    ghosts.head_center = { ghosts.x, ghosts.y };
}

int main()
{
    // crearea unei fererestre a jocului/ creating an application window
    InitWindow(630, 630, "PacMan");// width, height, title
    // am ales dimensiunile ferestrei in functie de numarul de pereti si a distantei dintre ei => 30*21 = 630

    // initializari, folosind structurile de date definite
    float start_time;
    float time_limit = 5.0f;// 5 secunde
    bool timed_out = false;
    bool ok = true;

    float game_start_running_time = GetTime(); float game_stop_running_time;

    int score = 0;
    bool won_game = false;
    bool lost_game = false;

    pacman_properties pacman;
    pacman.radius = 13;// o valoare a razei apropiata de 30/2 pentru o deplasare usoara in maze runner
    pacman.x = GetScreenWidth() / 2;
    pacman.y = GetScreenHeight() / 2 + 30;
    pacman.speed = 50;
    pacman.velocity.x = 0;
    pacman.velocity.y = 0;

    const int max_walls = 205;// numarul de pereti este indicat de valorile de 1 din matricea grid_map
    wall_properties walls[max_walls];
    
    const int max_pellets = 150;
    int num_eaten_pellets = 0;
    pellet_properties pellets[max_pellets];

    ghost_properties ghosts[4];
    Color body_color[4] = {PINK, RED, ORANGE, BLUE};

    ghosts[0].x = GetScreenWidth() / 2;
    ghosts[0].y = GetScreenHeight() / 2 - 30;
    ghosts[1].x = GetScreenWidth() / 2 + 30;
    ghosts[1].y = GetScreenHeight() / 2 - 30;
    ghosts[2].x = GetScreenWidth() / 2 - 30;
    ghosts[2].y = GetScreenHeight() / 2 - 30;
    ghosts[3].x = GetScreenWidth() / 2;
    ghosts[3].y = GetScreenHeight() / 2 - 90;

    for (int i = 0; i < 4; i++)
    {
        ghosts[i].head_radius = 15;
        ghosts[i].head_center = { ghosts[i].x, ghosts[i].y };
        ghosts[i].speed = 2;
        ghosts[i].velocity.x = 0;
        ghosts[i].velocity.y = 0;
        ghosts[i].body_rect = { ghosts[i].x - 15, ghosts[i].y, 30, 15 };
        /* ghosts[i].range = { ghosts[i].x, ghosts[i].y };
        ghosts[i].range_radius = 300;
        if (ghosts[i].y == GetScreenHeight() / 2 - 30)
        {
            ghosts[i].enter = false;
        }
        else ghosts[i].enter = true;*/
    }
    
    /*Vector2 random_target[4] = {
        {75, 45},// x, y
        {555, 45},
        {75, 585},
        {555, 585}
    };*/

    int k = 0, l = 0;// contoare ce reprezinta indicele fiecarui perete/ punct
    for (int i = 0; i < 21; i++)
    {
        for (int j = 0; j < 21; j++) 
        {
            if (grid_map[i][j] == 1) 
            {
                walls[k].x = j * 30;  
                walls[k].y = i * 30;
                walls[k].width = 30;
                walls[k].height = 30;
                walls[k].rect = { walls[k].x, walls[k].y, walls[k].width, walls[k].height };
                // atribuirea parametrilor unui patrat, ce difera numai referitor la pozitiile x si y (a nu se confunda cu desenarea figurilor)
                k++;
            } else if (grid_map[i][j] == 0) {  // inside the maze
                pellets[l].x = j * 30 + 15;
                pellets[l].y = i * 30 + 15;
                pellets[l].radius = 2;
                l++;
            }
        }
    }

    SetTargetFPS(60);

    while (!WindowShouldClose())// game loop
    {
        /* keyboard input - setarea tastelor ce controleaza miscarile lui pacman 
        si asigurarea unei viteze constante pentru fiecare directie de deplasare a lui pacman */
        if (IsKeyDown(KEY_RIGHT)) // deplasare Ox
        {
            pacman.velocity.x = pacman.speed;// sens pozitiv
            pacman.velocity.y = 0;
            // normalizare velocity: velocity = norm_velocity()
            norm_velocity(pacman.velocity.x, pacman.velocity.y);
            pacman.velocity.x = 2;
            // din cauza normalizarii, e inutil sa modifici pacman.speed pt o viteza mai mare si e nevoie de un ~boost~
        } else if (IsKeyDown(KEY_LEFT))
        {
            pacman.velocity.x = -pacman.speed;// sens negativ
            pacman.velocity.y = 0;
            norm_velocity(pacman.velocity.x, pacman.velocity.y);
            pacman.velocity.x = -2;
        } else if (IsKeyDown(KEY_UP)) // deplasare Oy
        {
            pacman.velocity.x = 0;
            pacman.velocity.y = -pacman.speed;
            norm_velocity(pacman.velocity.x, pacman.velocity.y);
            pacman.velocity.y = -2;
        } else if (IsKeyDown(KEY_DOWN))
        {
            pacman.velocity.x = 0;
            pacman.velocity.y = pacman.speed;
            norm_velocity(pacman.velocity.x, pacman.velocity.y);
            pacman.velocity.y = 2;
        }

        check_collision(pacman.radius, pacman.x, pacman.y, pacman.velocity, walls, max_walls);// wall collision intre pacman si pereti

        // posibilitatea teleportarii din partile in care nu exista pereti
        if (pacman.x <= 0)// stanga spre dreapta
        {
            pacman.x = walls[0].width * 21 - pacman.radius;
        } else if (pacman.x >= walls[0].width * 21)// dreapta spre stanga; valoarea e chiar 630
        {
            pacman.x = pacman.radius;
        }

        // check for pellet collisions
        for (int i = 0; i < max_pellets; i++)
        {
            if (CheckCollisionCircles({ pacman.x, pacman.y }, pacman.radius, { pellets[i].x, pellets[i].y }, pellets[i].radius))
            {
                // eliminarea punctului in momentul in care pacman il mananca
                if (lost_game == false)// scorul e incrementat doar daca jocul inca ruleaza
                {
                    pellets[i].x = -1;
                    pellets[i].y = -1;
                    score += 10;
                    num_eaten_pellets++;
                }
            }
        }
        
        /*for (int i = 0; i < 4; i++)
        {
            ghost_move(ghosts[i], walls, max_walls, grid_map);
        }*/

        Inky(ghosts[3]);
        Pinky(ghosts[0]);
        Blinky(ghosts[1]);
        Clyde(ghosts[2]);
        // std::cout << "ghost position: (" << ghosts[3].x << ", " << ghosts[3].y << ")" << std::endl;

        // check for pacman - ghost collision
        for (int i = 0; i < 4; i++)
        {
            if (CheckCollisionCircles({ pacman.x, pacman.y }, pacman.radius, { ghosts[i].x, ghosts[i].y }, ghosts[i].head_radius))
            {
                // resetarea pozitiei lui pacman in maze
                pacman.x = GetScreenWidth() / 2;
                pacman.y = GetScreenHeight() / 2 + 30;
                pacman.velocity.x = 0;
                pacman.velocity.y = 0;
                lost_game = true;
            }
        }

        if (num_eaten_pellets == max_pellets)// winning condition
        {
            won_game = true;
            start_time = GetTime();// memoreaza momentul de finalizare a jocului
            if (ok == true)
            {
                game_stop_running_time = GetTime();
                ok = false;
            }
        }
        
        BeginDrawing();// start the rendering process

        ClearBackground(BLANK);

        // desenarea peretilor
        for (int i = 0; i < max_walls; i++) 
        {
            DrawRectangle(walls[i].rect.x, walls[i].rect.y, walls[i].rect.width, walls[i].rect.height, PURPLE);
        }

        // desenarea lui pacman
        DrawCircle(pacman.x, pacman.y, pacman.radius, GOLD);

        // desenarea punctelor
        for (int i = 0; i < max_pellets; i++) 
        {
            DrawCircle(pellets[i].x, pellets[i].y, pellets[i].radius, GOLD);
        }

        // desenarea fantomelor
        for (int i = 0; i < 4; i++)
        {
            DrawCircle(ghosts[i].head_center.x, ghosts[i].head_center.y, ghosts[i].head_radius, body_color[i]);
            DrawRectangle(ghosts[i].x-15, ghosts[i].y, 30, 15, body_color[i]);
        }
        
        if (won_game) 
        {
            if (timed_out == false) 
            {
                DrawText("Congratulations!", 235, 275, 20, MAGENTA);
                DrawText(TextFormat("Start Time: %.2f", game_stop_running_time - game_start_running_time), 235, 100, 20, MAGENTA);
                if (ok == false)
                {
                    float current_time = GetTime();// se modifica constant pt a stabili daca s-a intrecut limita de timp alocata afisarii textului congrats 
                    float elapsed_time = current_time - start_time;
                    if (elapsed_time >= time_limit)
                    {
                        timed_out = true;
                    }
                }
            }
            else// dupa se afiseaza indicatia de a iesi din joc
            {
                DrawText("Please exit the game", 215, 275, 20, GREEN);
            }
        }
        else if (lost_game)
        {
            score = 0;// resetarea scorului

            // modificarea aspectului componentelor
            ClearBackground(BLACK);
            for (int i = 0; i < max_walls; i++)
            {
                DrawRectangle(walls[i].rect.x, walls[i].rect.y, walls[i].rect.width, walls[i].rect.height, RAYWHITE);
            }
            for (int i = 0; i < 4; i++)
            {
                DrawCircle(ghosts[i].head_center.x, ghosts[i].head_center.y, ghosts[i].head_radius, RAYWHITE);
                DrawRectangle(ghosts[i].x - 15, ghosts[i].y, 30, 15, RAYWHITE);
            }
            for (int i = 0; i < max_pellets; i++)
            {
                DrawCircle(pellets[i].x, pellets[i].y, pellets[i].radius, RAYWHITE);
            }
            DrawCircle(pacman.x, pacman.y, pacman.radius, RED);

            DrawText("Game over", 265, 275, 20, GREEN);
        }

        DrawText(TextFormat("Score: %04d", score), 510, 15, 20, WHITE);// format specifier pentru a afisa scorul de dimensiune 4 cifre

        DrawFPS(15, 15);// posX, posY - coord carteziane; (0, 0) - coltul din stanga sus
        EndDrawing();

    }// end of the game loop

    CloseWindow();
    return 0;
}