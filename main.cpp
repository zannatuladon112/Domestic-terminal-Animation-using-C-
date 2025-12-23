#include <GL/glut.h>
#include <math.h>
#include <vector>
#include <string>

// --- Constants & Configuration ---
const int WINDOW_WIDTH = 1200;
const int WINDOW_HEIGHT = 750;
const float PI = 3.1415926535f;

// --- Structures ---
struct Building {
    float x;
    float width;
    float height;
    std::string name;
    float r, g, b;
};

struct Car {
    float x;
    float speed;
    float r, g, b;
    bool movingRight;
};

// --- Global State Variables ---
bool isDay = true;
bool isRedLight = false;

// Speed Transition Variables
float targetCarSpeedMult = 1.0f;
float currentCarSpeedMult = 1.0f;

float planeSpeed = 7.0f;
float metroSpeed = 5.0f;

// Plane Animation Variables
float planeX = -200;
float planeY = 320;
bool planeActive = false;
float gearOffset = 0.0f;
int airplaneTimerMs = 15000;

// Environment Variables
int blinkTimer = 0;
float metroX = -500;
float birdOffset = 0;

std::vector<Building> cityBuildings;
std::vector<Car> cars;

// --- Primitive Drawing Helpers ---

void drawRect(float x, float y, float w, float h, float r, float g, float b) {
    glColor3f(r, g, b);
    glRectf(x, y, x + w, y + h);
}

void drawCircle(float cx, float cy, float r, int segments) {
    glBegin(GL_POLYGON);
    for (int i = 0; i < segments; i++) {
        float theta = 2.0f * PI * float(i) / (float)segments;
        float px = r * cosf(theta);
        float py = r * sinf(theta);
        glVertex2f(cx + px, cy + py);
    }
    glEnd();
}

void drawText(float x, float y, std::string text) {
    glRasterPos2f(x, y);
    for (char c : text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);
    }
}

// --- Environment Components ---

void drawEnvironment() {
    if (isDay) {
        glClearColor(0.5f, 0.8f, 1.0f, 1.0f);
        glColor3f(1.0f, 0.9f, 0.0f);
        drawCircle(1100, 920, 50, 30);
        glColor3f(1.0f, 1.0f, 1.0f);
        for(int i = 200; i < 1200; i += 400) {
            drawCircle(i, 900, 30, 20);
            drawCircle(i + 35, 910, 40, 20);
        }
        glColor3f(0.0f, 0.0f, 0.0f);
        for(int i = 0; i < 3; i++) {
            float bx = 300 + i * 100 + birdOffset;
            float by = 850;
            glBegin(GL_LINE_STRIP);
            glVertex2f(bx, by);
            glVertex2f(bx + 10, by - 5);
            glVertex2f(bx + 20, by);
            glEnd();
        }
    } else {
        glClearColor(0.01f, 0.01f, 0.1f, 1.0f);
        glColor3f(1.0f, 1.0f, 1.0f);
        glBegin(GL_POINTS);
        for(int i = 0; i < 80; i++) {
            glVertex2f((i * 17) % 1200, (i * 23) % 300 + 700);
        }
        glEnd();
        glColor3f(0.9f, 0.9f, 0.9f);
        drawCircle(1100, 920, 40, 30);
    }
}

// --- Streetlight Components ---

void drawStreetlight(float x, float y) {
    drawRect(x, y, 4, 60, 0.2, 0.2, 0.2);
    drawRect(x - 10, y + 60, 14, 4, 0.2, 0.2, 0.2);
    if (!isDay) {
        glColor3f(1.0f, 1.0f, 0.6f);
        drawRect(x - 10, y + 56, 10, 4, 1.0, 1.0, 0.6);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(1.0f, 1.0f, 0.0f, 0.2f);
        glBegin(GL_TRIANGLES);
        glVertex2f(x - 5, y + 56);
        glVertex2f(x - 30, y);
        glVertex2f(x + 20, y);
        glEnd();
        glDisable(GL_BLEND);
    } else {
        drawRect(x - 10, y + 56, 10, 4, 0.1, 0.1, 0.1);
    }
}

// --- Building Components ---

void drawWindowFrames(float x, float y, float w, float h) {
    glColor3f(0.0f, 0.0f, 0.0f);
    for (float gx = x + 40; gx < x + w - 20; gx += 40) {
        glRectf(gx, y, gx + 2, y + h);
    }
}

void drawTerminalRoof(float bx, float bw, float bh) {
    glColor3f(0.2f, 0.2f, 0.25f);
    glBegin(GL_POLYGON);
    for (int i = 0; i <= 180; i += 10) {
        float rad = i * PI / 180.0f;
        float rx = bx + bw / 2.0f + (bw / 1.8f * cos(rad));
        float ry = 350.0f + bh + (40.0f * sin(rad));
        glVertex2f(rx, ry);
    }
    glEnd();
}

void drawModernTerminal() {
    Building& b = cityBuildings[0];
    drawRect(b.x, 350, b.width, b.height, 0.5, 0.5, 0.55);
    drawRect(b.x + 10, 360, b.width - 20, 80, 0.1, 0.3, 0.5);
    drawWindowFrames(b.x + 10, 360, b.width - 20, 80);
    drawTerminalRoof(b.x, b.width, b.height);
    drawRect(b.x + 20, 350 + b.height + 10, b.width - 40, 45, 0.1, 0.1, 0.1);
    glColor3f(1.0f, 1.0f, 0.0f);
    drawText(b.x + 110, 350 + b.height + 25, b.name);
    drawRect(b.x + 40, 350 + b.height, 30, 450, 0.6, 0.6, 0.65);
    drawRect(b.x + 15, 880, 80, 40, 0.05, 0.05, 0.1);
    if (!isDay && (blinkTimer % 40 < 20)) {
        glColor3f(1.0f, 0.0f, 0.0f);
        drawCircle(b.x + 55, 935, 8, 12);
    }
}

// --- Plane Sub-Components ---

void drawPlaneParts() {
    drawRect(0, 0, 140, 35, 0.95, 0.95, 0.95);
    if (!isDay) glColor3f(0.9f, 0.9f, 0.6f);
    else glColor3f(0.05f, 0.05f, 0.1f);
    glRectf(120, 18, 140, 30);
    glColor3f(0.95f, 0.95f, 0.95f);
    glBegin(GL_TRIANGLES);
    glVertex2f(140, 0); glVertex2f(140, 35); glVertex2f(160, 0);
    glEnd();
    if (!isDay) glColor3f(0.9f, 0.9f, 0.6f);
    else glColor3f(0.6f, 0.8f, 1.0f);
    for (float wx = 25; wx < 110; wx += 18) {
        drawCircle(wx, 22, 3, 10);
    }
}

void drawLandingGear() {
    if (gearOffset < 15.0f && planeY < 500) {
        glColor3f(0.3f, 0.3f, 0.3f);
        drawRect(30, -10 + gearOffset, 4, 10 - gearOffset, 0.3, 0.3, 0.3);
        drawCircle(32, -10 + gearOffset, 5, 10);
        drawRect(100, -10 + gearOffset, 4, 10 - gearOffset, 0.3, 0.3, 0.3);
        drawCircle(102, -10 + gearOffset, 5, 10);
    }
}

void drawPlane() {
    if (!planeActive) return;
    glPushMatrix();
    glTranslatef(planeX, planeY, 0);
    if (planeX > 100) glRotatef(30, 0, 0, 1);
    drawPlaneParts();
    drawLandingGear();
    glColor3f(0.0f, 0.4f, 0.0f);
    glBegin(GL_POLYGON);
    glVertex2f(0, 35); glVertex2f(30, 35); glVertex2f(10, 80); glVertex2f(-15, 80);
    glEnd();
    glColor3f(0.8f, 0.0f, 0.0f); drawCircle(5, 60, 10, 15);
    glColor3f(0, 0, 0); drawText(50, 5, "BIMAN");
    glPopMatrix();
}

// --- Main Scene ---

void drawFlyover() {
    drawRect(0, 300, 1200, 40, 0.18, 0.18, 0.18);
    for(int i = 150; i < 1200; i += 350) drawRect(i, 300, 30, 40, 0.4, 0.4, 0.4);
    drawRect(0, 340, 1200, 25, 0.3, 0.3, 0.35);
    for(int i = 80; i < 1200; i += 280) drawStreetlight(i, 365);
    drawRect(metroX, 365, 280, 40, 0.9, 0.9, 0.95);
    for(int i = 15; i < 260; i += 45) drawRect(metroX + i, 375, 30, 22, 0.1, 0.2, 0.4);
    if (!isDay) { glColor3f(1.0f, 1.0f, 0.7f); drawRect(metroX + 275, 375, 5, 12, 1, 1, 0.5); }
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    drawEnvironment();
    drawModernTerminal();

    for (size_t i = 1; i < cityBuildings.size(); i++) {
        Building& b = cityBuildings[i];
        drawRect(b.x, 350, b.width, b.height, b.r, b.g, b.b);
        for(float wy = 370; wy < 350 + b.height - 60; wy += 40) {
            for(float wx = b.x + 10; wx < b.x + b.width - 20; wx += 25) {
                if(!isDay) glColor3f(0.9f, 0.9f, 0.6f); else glColor3f(0.1f, 0.1f, 0.2f);
                glRectf(wx, wy, wx + 12, wy + 20);
            }
        }
        drawRect(b.x + 5, 350 + b.height, b.width - 10, 35, 0.1, 0.1, 0.1);
        if (!isDay) glColor3f(1.0f, 1.0f, 0.7f); else glColor3f(1.0f, 1.0f, 1.0f);
        drawText(b.x + 12, 350 + b.height + 15, b.name);
    }

    drawFlyover();
    drawRect(0, 0, 1200, 300, 0.18, 0.18, 0.18);
    glColor3f(1.0f, 1.0f, 1.0f); for(int i = 0; i < 1200; i += 80) drawRect(i, 148, 40, 4, 1, 1, 1);

    drawRect(980, 300, 120, 10, 0.3, 0.3, 0.3);
    drawRect(1160, 250, 10, 150, 0.1, 0.1, 0.1);
    drawRect(1145, 340, 40, 65, 0, 0, 0);
    if(isRedLight) glColor3f(1,0,0); else glColor3f(0.2,0,0); drawCircle(1165, 385, 8, 12);
    if(!isRedLight) glColor3f(0,1,0); else glColor3f(0,0.2,0); drawCircle(1165, 355, 8, 12);

    for(int i = 0; i < (int)cars.size(); i++) {
        float cy = (i % 2 == 0) ? 180 : 50;

        // Car Body
        drawRect(cars[i].x, cy, 90, 30, cars[i].r, cars[i].g, cars[i].b);
        drawRect(cars[i].x + 15, cy + 30, 60, 18, cars[i].r, cars[i].g, cars[i].b);

        // Car Windows
        drawRect(cars[i].x + 20, cy + 32, 20, 12, 0.6f, 0.8f, 1.0f);
        drawRect(cars[i].x + 50, cy + 32, 20, 12, 0.6f, 0.8f, 1.0f);

        // Car Lights (At Night)
        if (!isDay) {
            if (cars[i].movingRight) {
                drawRect(cars[i].x + 85, cy + 10, 5, 10, 1, 1, 0.8);
                drawRect(cars[i].x, cy + 10, 3, 10, 0.8, 0, 0);
            } else {
                drawRect(cars[i].x, cy + 10, 5, 10, 1, 1, 0.8);
                drawRect(cars[i].x + 87, cy + 10, 3, 10, 0.8, 0, 0);
            }
        }

        // Wheels
        glColor3f(0, 0, 0);
        drawCircle(cars[i].x + 20, cy, 12, 10);
        drawCircle(cars[i].x + 70, cy, 12, 10);
    }

    drawPlane();
    glutSwapBuffers();
}

// --- Initialization & Logic ---

void initScenario() {
    float curX = 0;
    cityBuildings.push_back({curX, 280, 150, "Domestic Terminal BD", 0.4, 0.4, 0.45}); curX += 280;
    cityBuildings.push_back({curX, 180, 480, "BIMAN HQ", 0.3, 0.35, 0.4}); curX += 180;
    cityBuildings.push_back({curX, 140, 320, "BFC FAST FOOD", 0.5, 0.3, 0.3}); curX += 140;
    cityBuildings.push_back({curX, 160, 520, "HSBC BANK", 0.3, 0.3, 0.45}); curX += 160;
    cityBuildings.push_back({curX, 150, 380, "NAVANA MOTORS", 0.35, 0.35, 0.35}); curX += 150;
    cityBuildings.push_back({curX, 160, 450, "TOYOTA", 0.45, 0.2, 0.2}); curX += 160;
    cityBuildings.push_back({curX, 160, 410, "NISSAN", 0.2, 0.2, 0.25});
    for (int i = 0; i < 4; i++) {
        cars.push_back({(float)(i * 350), 3.0f, (float)(rand()%10)/10.0f, (float)(rand()%10)/10.0f, 0.6f, true});
        cars.push_back({(float)(1200 - i * 350), 3.5f, 0.6f, (float)(rand()%10)/10.0f, (float)(rand()%10)/10.0f, false});
    }
}

void timer(int val) {
    blinkTimer++; birdOffset += 2.0f; if(birdOffset > 1100) birdOffset = -200;
    if (currentCarSpeedMult < targetCarSpeedMult) currentCarSpeedMult += 0.02f;
    if (currentCarSpeedMult > targetCarSpeedMult) currentCarSpeedMult -= 0.02f;
    airplaneTimerMs += 16;
    if (airplaneTimerMs >= 15000) {
        planeActive = true; planeX = -200; planeY = 320; gearOffset = 0.0f; airplaneTimerMs = 0;
    }
    if (planeActive) {
        planeX += planeSpeed;
        if (planeX > 100) {
            planeY += (planeSpeed * 0.577f);
            if (gearOffset < 15.0f) gearOffset += 0.25f;
        }
        if (planeX > 1400) planeActive = false;
    }
    metroX += metroSpeed; if (metroX > 1300) metroX = -400;
    if (!isRedLight) {
        for(auto& c : cars) {
            if (c.movingRight) { c.x += c.speed * currentCarSpeedMult; if(c.x > 1250) c.x = -150; }
            else { c.x -= c.speed * currentCarSpeedMult; if(c.x < -150) c.x = 1250; }
        }
    }
    glutPostRedisplay(); glutTimerFunc(16, timer, 0);
}

void keyboard(unsigned char key, int x, int y) {
    if (key == 'd' || key == 'D') isDay = true;
    if (key == 'n' || key == 'N') isDay = false;
    if (key == 'w' || key == 'W') planeSpeed += 1.0f;
    if (key == 'q' || key == 'Q') planeSpeed = (planeSpeed > 2) ? planeSpeed - 1 : 2;
    if (key == 'a' || key == 'A') metroSpeed += 0.5f;
    if (key == 's' || key == 'S') metroSpeed = (metroSpeed > 1) ? metroSpeed - 0.5f : 1;
}

void mouse(int button, int state, int x, int y) {
    if (state == GLUT_DOWN) {
        if (button == GLUT_LEFT_BUTTON) isRedLight = true;
        if (button == GLUT_RIGHT_BUTTON) isRedLight = false;
        if (button == 3) targetCarSpeedMult += 0.5f;
        if (button == 4) targetCarSpeedMult = (targetCarSpeedMult > 0.5f) ? targetCarSpeedMult - 0.5f : 0.5f;
    }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("Airport City Integrated Infrastructure");
    initScenario();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    gluOrtho2D(0, 1200, 0, 1000);
    glutDisplayFunc(display);
    glutTimerFunc(0, timer, 0);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutMainLoop();
    return 0;
}
