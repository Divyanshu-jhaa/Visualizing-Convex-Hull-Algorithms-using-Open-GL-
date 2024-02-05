#include <GL/freeglut.h>
#include <iostream>
#include <vector>
#include <stack>
#include <algorithm>
#include <cmath>

// Global variables
int windowWidth = 800;
int windowHeight = 600;
int numPoints = 0;
std::vector<std::pair<int, int>> points;
std::vector<std::pair<int, int>> convexHull;

enum Algorithm
{
    GRAHAMS_SCAN,
    JARVIS_MARCH,
    CHANS_ALGORITHM,
    ANDREWS_MONOTONE_CHAIN,
    CLEAR_SCREEN
};
Algorithm selectedAlgorithm = GRAHAMS_SCAN;

// Function prototypes
void display();
void reshape(int width, int height);
void mouse(int button, int state, int x, int y);
void menu(int value);
int orientation(std::pair<int, int> a, std::pair<int, int> b, std::pair<int, int> c);
bool comparePoints(std::pair<int, int> p1, std::pair<int, int> p2);
void grahamScan();
void jarvisMarch();
void chansAlgorithm();
std::vector<std::pair<int, int>> andrewsMonotoneChain(std::vector<std::pair<int, int>> points);

int main(int argc, char **argv)
{
    // Get the number of points from the user
    std::cout << "Enter the number of points: ";
    std::cin >> numPoints;

    if (numPoints <= 0)
    {
        std::cerr << "Invalid number of points. Exiting." << std::endl;
        return -1;
    }

    // Initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("Convex Hull Algorithms");

    // Set callback functions
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);

    // Create menu
    glutCreateMenu(menu);
    glutAddMenuEntry("Graham's Scan", GRAHAMS_SCAN);
    glutAddMenuEntry("Jarvis March", JARVIS_MARCH);
    // glutAddMenuEntry("Chan's Algorithm", CHANS_ALGORITHM);
    glutAddMenuEntry("Andrew's Monotone Chain", ANDREWS_MONOTONE_CHAIN);
    glutAddMenuEntry("Clear Screen", CLEAR_SCREEN);
    glutAttachMenu(GLUT_RIGHT_BUTTON);

    // Set OpenGL parameters
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glPointSize(5.0);
    glColor3f(0.0, 0.0, 0.0);

    // Enter the GLUT event loop
    glutMainLoop();

    return 0;
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw the existing points
    glBegin(GL_POINTS);
    for (const auto &point : points)
    {
        glVertex2i(point.first, point.second);
    }
    glEnd();

    // Draw the convex hull
    glColor3f(1.0, 0.0, 0.0); // Set color to red for the convex hull
    glBegin(GL_LINE_LOOP);
    for (const auto &point : convexHull)
    {
        glVertex2i(point.first, point.second);
    }
    glEnd();

    glFlush();
}

void reshape(int width, int height)
{
    windowWidth = width;
    windowHeight = height;

    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, width, height, 0);

    glMatrixMode(GL_MODELVIEW);
}

void mouse(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && points.size() < numPoints)
    {
        // Add the clicked point to the vector
        points.emplace_back(x, y);

        // Redraw the scene
        glutPostRedisplay();

        // If enough points are entered, perform selected convex hull algorithm and update convex hull
        if (points.size() == numPoints)
        {
            switch (selectedAlgorithm)
            {
            case GRAHAMS_SCAN:
                grahamScan();
                break;
            case JARVIS_MARCH:
                jarvisMarch();
                break;
            // case CHANS_ALGORITHM:
            // chansAlgorithm();
            // break;
            case ANDREWS_MONOTONE_CHAIN:
                convexHull = andrewsMonotoneChain(points);
                break;
            case CLEAR_SCREEN:
                points.clear();
                convexHull.clear();
                glutPostRedisplay();
                break;
            }
        }
    }
}

void menu(int value)
{
    switch (value)
    {
    case GRAHAMS_SCAN:
    case JARVIS_MARCH:
    case CHANS_ALGORITHM:
    case ANDREWS_MONOTONE_CHAIN:
        selectedAlgorithm = static_cast<Algorithm>(value);
        points.clear();
        convexHull.clear();
        glutPostRedisplay();
        break;
    case CLEAR_SCREEN:
        points.clear();
        convexHull.clear();
        glutPostRedisplay();
        break;
    }
}

int orientation(std::pair<int, int> p, std::pair<int, int> q, std::pair<int, int> r)
{
    int val = (q.second - p.second) * (r.first - q.first) - (q.first - p.first) * (r.second - q.second);
    if (val == 0)
        return 0;             // Collinear
    return (val > 0) ? 1 : 2; // Clockwise or Counterclockwise
}

bool comparePoints(std::pair<int, int> p1, std::pair<int, int> p2)
{
    int o = orientation(points[0], p1, p2);
    if (o == 0)
        return (std::pow((p1.first - points[0].first), 2) + std::pow((p1.second - points[0].second), 2)) < (std::pow((p2.first - points[0].first), 2) + std::pow((p2.second - points[0].second), 2));
    return (o == 2);
}

void grahamScan()
{
    // Find the point with the lowest y-coordinate (and leftmost if ties)
    int minY = points[0].second;
    int minIndex = 0;
    for (int i = 1; i < numPoints; i++)
    {
        int y = points[i].second;
        if ((y < minY) || (y == minY && points[i].first < points[minIndex].first))
        {
            minY = y;
            minIndex = i;
        }
    }

    // Swap the lowest point with the first point
    std::swap(points[0], points[minIndex]);

    // Sort the remaining points based on polar angle with respect to the lowest point
    std::sort(points.begin() + 1, points.end(), comparePoints);

    // Initialize the convex hull
    convexHull.clear();
    convexHull.push_back(points[0]);
    convexHull.push_back(points[1]);

    // Process the sorted points to find the convex hull
    for (int i = 2; i < numPoints; i++)
    {
        while (convexHull.size() > 1 && orientation(convexHull[convexHull.size() - 2], convexHull.back(), points[i]) != 2)
        {
            convexHull.pop_back();
        }
        convexHull.push_back(points[i]);
    }
}

void jarvisMarch()
{
    // Initialize result
    int n = points.size();
    if (n < 3)
        return;

    std::vector<std::pair<int, int>> hull;

    // Find the leftmost point
    int leftmost = 0;
    for (int i = 1; i < n; i++)
    {
        if (points[i].first < points[leftmost].first)
        {
            leftmost = i;
        }
    }

    int p = leftmost, q;
    do
    {
        hull.push_back(points[p]);
        q = (p + 1) % n;
        for (int i = 0; i < n; i++)
        {
            if (orientation(points[p], points[i], points[q]) == 2)
            {
                q = i;
            }
        }
        p = q;
    } while (p != leftmost);

    convexHull = hull;
}

void chansAlgorithm()
{
    // Implementation of Chan's Algorithm goes here
    // ...
}

bool isRightTurn(std::pair<int, int> a, std::pair<int, int> b, std::pair<int, int> c)
{
    return ((b.first - a.first) * (c.second - a.second) - (b.second - a.second) * (c.first - a.first)) < 0;
}

std::vector<std::pair<int, int>> andrewsMonotoneChain(std::vector<std::pair<int, int>> points)
{
    int n = points.size();
    if (n < 3)
    {
        std::cerr << "Andrew's Monotone Chain requires at least 3 points. Exiting." << std::endl;
        return {};
    }

    // Sort points based on x-coordinate and then y-coordinate in case of ties
    std::sort(points.begin(), points.end());

    std::vector<std::pair<int, int>> lowerHull, upperHull;

    // Construct lower hull
    for (int i = 0; i < n; ++i)
    {
        while (lowerHull.size() >= 2 && !isRightTurn(lowerHull[lowerHull.size() - 2], lowerHull[lowerHull.size() - 1], points[i]))
        {
            lowerHull.pop_back();
        }
        lowerHull.push_back(points[i]);
    }

    // Construct upper hull
    for (int i = n - 1; i >= 0; --i)
    {
        while (upperHull.size() >= 2 && !isRightTurn(upperHull[upperHull.size() - 2], upperHull[upperHull.size() - 1], points[i]))
        {
            upperHull.pop_back();
        }
        upperHull.push_back(points[i]);
    }

    // Combine lower and upper hulls
    std::vector<std::pair<int, int>> convexHull;
    convexHull.insert(convexHull.end(), lowerHull.begin(), lowerHull.end() - 1);
    convexHull.insert(convexHull.end(), upperHull.begin(), upperHull.end() - 1);

    return convexHull;
}
