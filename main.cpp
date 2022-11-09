#include "raylib.h"
#include <cmath>
#include <vector>
#include <deque>
#include <iostream>

//--------------------------------------------------------------------------------------
// Defines
//--------------------------------------------------------------------------------------
#define _USE_MATH_DEFINES
#define MAX_BODIES 100
#define MAX_PATH_POINTS 5000

//--------------------------------------------------------------------------------------
// Variable Declaration
//--------------------------------------------------------------------------------------
const int screen_width = 0;
const int screen_height = 0;
const double gravitational_constant = 667.4;

float vel_distance;
Vector2 vel_direction;
Vector2 vel_line;

//--------------------------------------------------------------------------------------
// Classes & Objects
//--------------------------------------------------------------------------------------
class CelestialBody {
    public:
        double radius;
        bool resizing;
        bool active;
        bool gravity_affected = true;
        std::deque<Vector2> path_points;
//        double barycentre_length;
//        Vector2 barycentre;

        double mass;
        Vector2 velocity;
        Vector2 position;
        Vector2 acceleration;

        bool operator!=(const CelestialBody &rhs) {
            return (this->position.x != rhs.position.x and this->position.y != rhs.position.y);
        }

        void update_position(CelestialBody &current, double dt, double halfdt) {
            current.mass = M_PI * pow(current.radius, 2);

            current.position.x -= current.velocity.x * dt + current.acceleration.x * halfdt * dt;
            current.position.y -= current.velocity.y * dt + current.acceleration.y * halfdt * dt;
        }

        void update_velocity(CelestialBody &current, CelestialBody &other, double halfdt) {
/*
            current.barycentre_length = object_distance * (other.mass/(current.mass+other.mass));

            double bar_distance_x = distance.x * (current.barycentre_length/object_distance);
            double bar_distance_y = distance.y * (current.barycentre_length/object_distance);

            current.barycentre = Vector2{current.position.x-bar_distance_x, current.position.y-bar_distance_y};
*/
            current.velocity.x += current.acceleration.x * halfdt;
            current.velocity.y += current.acceleration.y * halfdt;

            Vector2 distance = Vector2{current.position.x - other.position.x, current.position.y - other.position.y};

            double object_distance = (sqrt(pow(distance.x, 2) + pow(distance.y, 2)));

            Vector2 force = Vector2{(1.0 / object_distance * distance.x) * gravitational_constant * ((current.mass*other.mass)/pow(object_distance, 2)), 
                                      (1.0 / object_distance * distance.y) * gravitational_constant * ((current.mass*other.mass)/pow(object_distance, 2))};

            current.acceleration = Vector2{force.x/current.mass, force.y/current.mass};

            current.velocity.x += current.acceleration.x * halfdt;
            current.velocity.y += current.acceleration.y * halfdt;
        }     

        void update_paths(CelestialBody &current) {
            if (current.path_points.size() > MAX_PATH_POINTS) {
                current.path_points.pop_front();
            }
            current.path_points.push_back(current.position);
        }
};

//--------------------------------------------------------------------------------------
// Program main entry point
//--------------------------------------------------------------------------------------
int main(void) {

    // Initialization
    //--------------------------------------------------------------------------------------
    InitWindow(screen_width, screen_height, "Gravity Simulator");

    std::vector<CelestialBody> bodies;
    CelestialBody body;

    //--------------------------------------------------------------------------------------

    //--------------------------------------------------------------------------------------
    // Main game loop
    //--------------------------------------------------------------------------------------
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //-------------------------------------------------------------------------------------        
        double dt = GetFrameTime();
        double halfdt = 0.5 * dt;

        if (IsMouseButtonPressed(0)) {
            body.position = GetMousePosition();
            body.resizing = true;
            body.velocity = Vector2{0, 0};
            body.radius = 10;
        }

        if (IsMouseButtonDown(0)) {
            vel_distance = sqrt(pow((body.position.x - GetMouseX()), 2) 
                              + pow((body.position.y - GetMouseY()), 2));
            vel_direction = Vector2{(1.0 / vel_distance) * (body.position.x - GetMouseX()), 
                                      (1.0 / vel_distance) * (body.position.y - GetMouseY())};
            if (body.radius <= 0) {body.radius = 1;}
            if (body.radius > 500) {body.radius = 500;}
            body.radius += 1 * GetMouseWheelMoveV().y;
            body.mass = M_PI * pow(body.radius, 2);
            if (vel_distance > body.radius) {
                vel_line = Vector2{(body.position.x)+(vel_distance*vel_direction.x/3), (body.position.y)+(vel_distance*vel_direction.y/3)};
            } else {
                vel_line = body.position;
            }
        }

        if (IsMouseButtonReleased(0)) {
            if (body.radius > 0) {
                body.active = true;
                body.resizing = false;
                if (body.radius >= 100) {
                    body.gravity_affected = false;
                } 
                if (vel_distance > body.radius) {
                    float initial_velocity = vel_distance - body.radius;
                    body.velocity.x -= initial_velocity * vel_direction.x;
                    body.velocity.y -= initial_velocity * vel_direction.y;
                }
                bodies.push_back(body);
            }
        }
        
        for (int i = 0; i < bodies.size(); i++) {
            body.update_position(bodies[i], dt, halfdt);
            body.update_paths(bodies[i]);
            for (int j = 0; j < bodies.size(); j++) {
                if (bodies[i] != bodies[j]) {
                    if (bodies[i].gravity_affected == true) {
                        body.update_velocity(bodies[i], bodies[j], halfdt); 
                    }
                    if (CheckCollisionCircles(bodies[i].position, bodies[i].radius, bodies[j].position, bodies[j].radius)) {
                        if (bodies[i].radius >= bodies[j].radius) {
                            bodies[i].radius += bodies[j].radius/2;
                            bodies.erase(bodies.begin() + j);
                        } else {
                            bodies[j].radius += bodies[i].radius/2;
                            bodies.erase(bodies.begin() + i);
                        }
                    }
                }
            }
        }

        //--------------------------------------------------------------------------------------

        // Draw
        //--------------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(BLACK);

            // For Debugging
            // DrawText(TextFormat("body count: %d", bodies.size()), 190, 200, 20, LIGHTGRAY); 
            
            if (body.resizing) {                
                DrawLineV(body.position, vel_line, YELLOW);
                DrawCircleV(body.position, body.radius, RED);
                DrawText(TextFormat("r = %f, m = %f", body.radius, body.mass), body.position.x-150, 
                    body.position.y-body.radius-25, 20, LIGHTGRAY);
                DrawText(TextFormat("position x: %f, position y: %f", body.position.x, body.position.y), 
                    body.position.x-220, body.position.y+body.radius+10, 20, LIGHTGRAY);
            }
        
            for (auto & shape : bodies) {
                if (shape.path_points.size() > 1) {
                    for (int i = 0; i < shape.path_points.size()-1; i++) {
                        DrawLineV(shape.path_points[i], shape.path_points[i+1], ORANGE);
                    }
                }    
                if (shape.radius > 0) {
                    DrawCircleV(shape.position, shape.radius, GREEN);
                    DrawText(TextFormat("r = %f, m = %f", shape.radius, shape.mass), shape.position.x-150, 
                        shape.position.y-shape.radius-25, 20, LIGHTGRAY);
                    DrawText(TextFormat("position x: %f, position y: %f", shape.position.x, shape.position.y), 
                        shape.position.x-220, shape.position.y+shape.radius+10, 20, LIGHTGRAY);
                }
            }
           
        EndDrawing();
        //--------------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}