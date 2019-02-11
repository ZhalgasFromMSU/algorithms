#include <vector>
#include <iostream>

struct Point {
    int x;
    int y;
    Point()
        :   x(0)
        ,   y(0)
    {}

    Point(int a, int b)
        :   x(a)
        ,   y(b)
    {}
};
using Dir = Point;

bool on_the_line(size_t i, size_t j, const Dir& dir,
    const std::vector<Point>& points)
{
    if (dir.x == 0 && points[i].x != points[j].x) {
        return false;
    } else if (dir.y == 0 && points[i].y != points[j].y) {
        return false;
    } else if (dir.x == 0) {
        return true;
    } else if (dir.y == 0) {
        return true;
    }
    return (points[i].x - points[j].x) % dir.x == 0 &&
        (points[i].y - points[j].y) % dir.y == 0;
}

int max_points(std::vector<Point>& points) {
    Dir dir;
    int max(0);
    for (size_t i = 0; i < points.size(); ++i) {
        std::vector<bool> used(points.size(), false);
        for (size_t j = 0; j < points.size(); ++j) {
            if (i == j || used[j]) {
                continue;
            }
            dir.x = points[j].x - points[i].x;
            dir.y = points[j].y - points[i].y;
            int tmp(1);
            for (size_t k = 0; k < points.size(); ++k) {
                if (k == i || used[k]) {
                    continue;
                }
                if (on_the_line(i, k, dir, points)) {
                    ++tmp;
                    used[k] = true;
                    if (tmp > max) {
                        max = tmp;
                    }
                }
            }
        }
    }
    return max;
}

int main() {
    size_t n;
    std::cin >> n;
    std::vector<Point> points(n);
    for (size_t i = 0; i < n; ++i) {
        std::cin >> points[i].x >> points[i].y;
    }
    std::cout << max_points(points) << '\n';
    return 0;
}
