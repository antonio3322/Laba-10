#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <utility>
#include <string>
#include <sstream>
#include "glut.h"

const double E = 1e-9;
const double PI = 3.14159265358979323846;

enum ApplicationMode {
    MODE_SELECT_FIRST,
    MODE_SELECT_SECOND,
    MODE_ADD_POINT,
    MODE_ADD_LINE,
    MODE_ADD_POLYGON,
    MODE_ADD_ELLIPSE,
    MODE_ADD_CIRCLE,
    MODE_ADD_RECTANGLE,
    MODE_ADD_SQUARE,
    MODE_ADD_TRIANGLE,
    MODE_TRANSFORM_ROTATE,
    MODE_TRANSFORM_REFLECT_POINT,
    MODE_TRANSFORM_REFLECT_LINE,
    MODE_TRANSFORM_SCALE,
    MODE_CHECK_POINT_INSIDE
};

struct Point {
    double x, y;

    Point(double xCoord = 0, double yCoord = 0) : x(xCoord), y(yCoord) {}

    bool operator==(const Point& other) const {
        return std::abs(x - other.x) < E && std::abs(y - other.y) < E;
    }

    bool operator!=(const Point& other) const {
        return !(*this == other);
    }

    Point operator+(const Point& other) const {
        return Point(x + other.x, y + other.y);
    }

    Point operator-(const Point& other) const {
        return Point(x - other.x, y - other.y);
    }

    Point operator*(double scalar) const {
        return Point(x * scalar, y * scalar);
    }

    Point operator/(double scalar) const {
        return Point(x / scalar, y / scalar);
    }

    double dot(const Point& other) const {
        return x * other.x + y * other.y;
    }

    double cross(const Point& other) const {
        return x * other.y - y * other.x;
    }

    double distanceTo(const Point& other) const {
        return std::hypot(x - other.x, y - other.y);
    }
};

class Line;

class Shape {
public:
    virtual ~Shape() = default;


    virtual double getArea() const = 0;
    virtual double getPerimeter() const = 0;
    virtual Point getCenter() const = 0;
    virtual std::string getTypeName() const = 0;

    virtual bool isEqualTo(const Shape& other) const = 0;
    virtual bool isCongruentTo(const Shape& other) const = 0;
    virtual bool isSimilarTo(const Shape& other) const = 0;
    virtual bool containsPoint(Point point) const = 0;

    virtual void rotateAroundPoint(Point center, double angleDegrees) = 0;
    virtual void reflectOverPoint(Point center) = 0;
    virtual void reflectOverLine(const Line& axis) = 0;
    virtual void scaleFromPoint(Point center, double coefficient) = 0;

    virtual void draw(bool isFirstSelected, bool isSecondSelected) const = 0;
};

class Line : public Shape {
private:
    void updateLineEquation() {
        A = p1.y - p2.y;
        B = p2.x - p1.x;
        C = p1.x * p2.y - p2.x * p1.y;
    }

public:
    Point p1, p2;
    double A, B, C;  // Коэффициенты уравнения Ax + By + C = 0

    Line(Point point1, Point point2) : p1(point1), p2(point2) {
        updateLineEquation();
    }

    Line(double slope, double intercept) : p1(0, intercept), p2(1, slope + intercept) {
        updateLineEquation();
    }

    Line(Point point, double slope) : p1(point), p2(point.x + 1, point.y + slope) {
        updateLineEquation();
    }

    Line(double a, double b, double c) : A(a), B(b), C(c) {
        if (std::abs(b) > E) {
            p1 = Point(0, -c / b);
            p2 = Point(1, (-c - a) / b);
        }
        else {
            p1 = Point(-c / a, 0);
            p2 = Point(-c / a, 1);
        }
    }

    // Реализация виртуальных методов
    double getArea() const override { return 0; }
    double getPerimeter() const override { return 0; }

    bool isEqualTo(const Shape& other) const override {
        const Line* linePtr = dynamic_cast<const Line*>(&other);
        if (!linePtr) return false;
        return std::abs(A * linePtr->B - linePtr->A * B) < E &&
            std::abs(A * linePtr->C - linePtr->A * C) < E;
    }

    bool isCongruentTo(const Shape& other) const override {
        return dynamic_cast<const Line*>(&other) != nullptr;
    }

    bool isSimilarTo(const Shape& other) const override {
        return isCongruentTo(other);
    }

    bool containsPoint(Point point) const override {
        double distance = std::abs(A * point.x + B * point.y + C) / std::hypot(A, B);
        return distance < E;
    }

    void rotateAroundPoint(Point center, double angleDegrees) override {
        double radians = angleDegrees * PI / 180.0;
        double cosAngle = std::cos(radians);
        double sinAngle = std::sin(radians);

        auto rotatePoint = [&](Point p) {
            double dx = p.x - center.x;
            double dy = p.y - center.y;
            return Point(center.x + dx * cosAngle - dy * sinAngle,
                center.y + dx * sinAngle + dy * cosAngle);
            };

        p1 = rotatePoint(p1);
        p2 = rotatePoint(p2);
        updateLineEquation();
    }

    void reflectOverPoint(Point center) override {
        p1 = Point(2 * center.x - p1.x, 2 * center.y - p1.y);
        p2 = Point(2 * center.x - p2.x, 2 * center.y - p2.y);
        updateLineEquation();
    }

    void reflectOverLine(const Line& axis) override {
        double denominator = axis.A * axis.A + axis.B * axis.B;
        auto reflectPoint = [&](Point p) {
            double factor = 2 * (axis.A * p.x + axis.B * p.y + axis.C) / denominator;
            return Point(p.x - axis.A * factor, p.y - axis.B * factor);
            };

        p1 = reflectPoint(p1);
        p2 = reflectPoint(p2);
        updateLineEquation();
    }

    void scaleFromPoint(Point center, double coefficient) override {
        p1 = Point(center.x + coefficient * (p1.x - center.x),
            center.y + coefficient * (p1.y - center.y));
        p2 = Point(center.x + coefficient * (p2.x - center.x),
            center.y + coefficient * (p2.y - center.y));
        updateLineEquation();
    }

    void draw(bool isFirstSelected, bool isSecondSelected) const override {
        if (isFirstSelected) glColor3f(1.0f, 0.0f, 0.0f);
        else if (isSecondSelected) glColor3f(0.0f, 0.8f, 0.0f);
        else glColor3f(0.0f, 0.0f, 1.0f);
        glLineWidth((isFirstSelected || isSecondSelected) ? 3.0f : 1.0f);

        double dx = p2.x - p1.x;
        double dy = p2.y - p1.y;
        glBegin(GL_LINES);
        glVertex2f(p1.x - dx * 1000, p1.y - dy * 1000);
        glVertex2f(p2.x + dx * 1000, p2.y + dy * 1000);
        glEnd();

        glLineWidth(1.0f);
    }

    std::string getTypeName() const override { return "Line"; }
    Point getCenter() const override { return Point((p1.x + p2.x) / 2, (p1.y + p2.y) / 2); }
};

class PointShape : public Shape {
public:
    Point position;

    PointShape(Point pt) : position(pt) {}

    double getArea() const override { return 0; }
    double getPerimeter() const override { return 0; }

    bool isEqualTo(const Shape& other) const override {
        const PointShape* pointPtr = dynamic_cast<const PointShape*>(&other);
        return pointPtr && position == pointPtr->position;
    }

    bool isCongruentTo(const Shape& other) const override {
        return isEqualTo(other);
    }

    bool isSimilarTo(const Shape& other) const override {
        return dynamic_cast<const PointShape*>(&other) != nullptr;
    }

    bool containsPoint(Point point) const override {
        return position == point;
    }

    void rotateAroundPoint(Point center, double angleDegrees) override {
        double radians = angleDegrees * PI / 180.0;
        double cosAngle = std::cos(radians);
        double sinAngle = std::sin(radians);

        double dx = position.x - center.x;
        double dy = position.y - center.y;
        position.x = center.x + dx * cosAngle - dy * sinAngle;
        position.y = center.y + dx * sinAngle + dy * cosAngle;
    }

    void reflectOverPoint(Point center) override {
        position.x = 2 * center.x - position.x;
        position.y = 2 * center.y - position.y;
    }

    void reflectOverLine(const Line& axis) override {
        double denominator = axis.A * axis.A + axis.B * axis.B;
        double factor = 2 * (axis.A * position.x + axis.B * position.y + axis.C) / denominator;
        position.x -= axis.A * factor;
        position.y -= axis.B * factor;
    }

    void scaleFromPoint(Point center, double coefficient) override {
        position.x = center.x + coefficient * (position.x - center.x);
        position.y = center.y + coefficient * (position.y - center.y);
    }

    void draw(bool isFirstSelected, bool isSecondSelected) const override {
        if (isFirstSelected) glColor3f(1.0f, 0.0f, 0.0f);
        else if (isSecondSelected) glColor3f(0.0f, 0.8f, 0.0f);
        else glColor3f(0.0f, 0.0f, 1.0f);

        glPointSize(8.0f);
        glBegin(GL_POINTS);
        glVertex2f(position.x, position.y);
        glEnd();
        glPointSize(1.0f);
    }

    std::string getTypeName() const override { return "Point"; }
    Point getCenter() const override { return position; }
};

class Polygon : public Shape {
protected:
    std::vector<Point> vertices;

    // Проверка на самопересечение
    bool isSelfIntersecting() const {
        size_t vertexCount = vertices.size();
        if (vertexCount < 4) return false;

        for (size_t i = 0; i < vertexCount; ++i) {
            Point a1 = vertices[i];
            Point a2 = vertices[(i + 1) % vertexCount];

            for (size_t j = i + 2; j < vertexCount; ++j) {
                if (j == (i + 1) % vertexCount) continue;

                Point b1 = vertices[j];
                Point b2 = vertices[(j + 1) % vertexCount];

                // Проверка пересечения отрезков
                double d1 = (b2 - b1).cross(a1 - b1);
                double d2 = (b2 - b1).cross(a2 - b1);
                double d3 = (a2 - a1).cross(b1 - a1);
                double d4 = (a2 - a1).cross(b2 - a1);

                if (d1 * d2 < -E && d3 * d4 < -E) return true;
            }
        }
        return false;
    }

public:
    Polygon() {}
    Polygon(const std::vector<Point>& points) : vertices(points) {
        if (isSelfIntersecting()) {
            std::cout << "Warning: Self-intersecting polygon created!" << std::endl;
        }
    }

    template<typename... Points>
    Polygon(Points... points) {
        (vertices.push_back(points), ...);
        if (isSelfIntersecting()) {
            std::cout << "Warning: Self-intersecting polygon created!" << std::endl;
        }
    }

    size_t getVertexCount() const { return vertices.size(); }
    const std::vector<Point>& getVertices() const { return vertices; }

    // Проверка выпуклости
    bool isConvex() const {
        if (vertices.size() < 3) return false;

        int sign = 0;
        size_t vertexCount = vertices.size();

        for (size_t i = 0; i < vertexCount; ++i) {
            Point a = vertices[i];
            Point b = vertices[(i + 1) % vertexCount];
            Point c = vertices[(i + 2) % vertexCount];

            double crossProduct = (b - a).cross(c - b);
            if (std::abs(crossProduct) < E) continue;

            int currentSign = (crossProduct > 0) ? 1 : -1;
            if (sign == 0) sign = currentSign;
            else if (sign != currentSign) return false;
        }
        return true;
    }

    // Геометрические характеристики
    double getPerimeter() const override {
        double perimeter = 0;
        size_t vertexCount = vertices.size();

        for (size_t i = 0; i < vertexCount; ++i) {
            perimeter += vertices[i].distanceTo(vertices[(i + 1) % vertexCount]);
        }
        return perimeter;
    }

    double getArea() const override {
        double area = 0;
        size_t vertexCount = vertices.size();

        for (size_t i = 0; i < vertexCount; ++i) {
            area += vertices[i].x * vertices[(i + 1) % vertexCount].y -
                vertices[(i + 1) % vertexCount].x * vertices[i].y;
        }
        return std::abs(area) / 2.0;
    }

    // Сравнение
    bool isEqualTo(const Shape& other) const override {
        const Polygon* polygonPtr = dynamic_cast<const Polygon*>(&other);
        if (!polygonPtr || vertices.size() != polygonPtr->vertices.size()) return false;

        int vertexCount = vertices.size();

        // Проверка совпадения с учетом порядка обхода
        for (int shift = 0; shift < vertexCount; ++shift) {
            bool matchForward = true;
            bool matchBackward = true;

            for (int i = 0; i < vertexCount; ++i) {
                if (vertices[i] != polygonPtr->vertices[(i + shift) % vertexCount])
                    matchForward = false;
                if (vertices[i] != polygonPtr->vertices[(vertexCount - 1 - i + shift) % vertexCount])
                    matchBackward = false;
            }

            if (matchForward || matchBackward) return true;
        }
        return false;
    }

    bool isCongruentTo(const Shape& other) const override {
        const Polygon* polygonPtr = dynamic_cast<const Polygon*>(&other);
        if (!polygonPtr || vertices.size() != polygonPtr->vertices.size()) return false;

        std::vector<double> sides1, sides2;
        size_t vertexCount = vertices.size();

        for (size_t i = 0; i < vertexCount; ++i) {
            sides1.push_back(vertices[i].distanceTo(vertices[(i + 1) % vertexCount]));
            sides2.push_back(polygonPtr->vertices[i].distanceTo(
                polygonPtr->vertices[(i + 1) % polygonPtr->vertices.size()]));
        }

        std::sort(sides1.begin(), sides1.end());
        std::sort(sides2.begin(), sides2.end());

        for (size_t i = 0; i < sides1.size(); ++i) {
            if (std::abs(sides1[i] - sides2[i]) > E) return false;
        }
        return true;
    }

    bool isSimilarTo(const Shape& other) const override {
        const Polygon* polygonPtr = dynamic_cast<const Polygon*>(&other);
        if (!polygonPtr || vertices.size() != polygonPtr->vertices.size()) return false;

        double areaRatio = getArea() / polygonPtr->getArea();
        double perimeterRatio = getPerimeter() / polygonPtr->getPerimeter();

        return std::abs(areaRatio - perimeterRatio * perimeterRatio) < E;
    }

    bool containsPoint(Point point) const override {
        int vertexCount = vertices.size();
        bool inside = false;

        for (int i = 0, j = vertexCount - 1; i < vertexCount; j = i++) {
            if (((vertices[i].y > point.y) != (vertices[j].y > point.y)) &&
                (point.x < (vertices[j].x - vertices[i].x) * (point.y - vertices[i].y) /
                    (vertices[j].y - vertices[i].y) + vertices[i].x)) {
                inside = !inside;
            }
        }
        return inside;
    }

    void rotateAroundPoint(Point center, double angleDegrees) override {
        double radians = angleDegrees * PI / 180.0;
        double cosAngle = std::cos(radians);
        double sinAngle = std::sin(radians);

        for (auto& vertex : vertices) {
            double dx = vertex.x - center.x;
            double dy = vertex.y - center.y;
            vertex.x = center.x + dx * cosAngle - dy * sinAngle;
            vertex.y = center.y + dx * sinAngle + dy * cosAngle;
        }
    }

    void reflectOverPoint(Point center) override {
        for (auto& vertex : vertices) {
            vertex.x = 2 * center.x - vertex.x;
            vertex.y = 2 * center.y - vertex.y;
        }
    }

    void reflectOverLine(const Line& axis) override {
        double denominator = axis.A * axis.A + axis.B * axis.B;

        for (auto& vertex : vertices) {
            double factor = 2 * (axis.A * vertex.x + axis.B * vertex.y + axis.C) / denominator;
            vertex.x -= axis.A * factor;
            vertex.y -= axis.B * factor;
        }
    }

    void scaleFromPoint(Point center, double coefficient) override {
        for (auto& vertex : vertices) {
            vertex.x = center.x + coefficient * (vertex.x - center.x);
            vertex.y = center.y + coefficient * (vertex.y - center.y);
        }
    }

    void draw(bool isFirstSelected, bool isSecondSelected) const override {
        if (isFirstSelected) glColor3f(1.0f, 0.0f, 0.0f);
        else if (isSecondSelected) glColor3f(0.0f, 0.8f, 0.0f);
        else glColor3f(0.0f, 0.0f, 1.0f);

        glLineWidth((isFirstSelected || isSecondSelected) ? 3.0f : 1.0f);

        glBegin(GL_LINE_LOOP);
        for (const auto& vertex : vertices) {
            glVertex2f(vertex.x, vertex.y);
        }
        glEnd();

        glLineWidth(1.0f);
    }

    std::string getTypeName() const override { return "Polygon"; }

    Point getCenter() const override {
        if (vertices.empty()) return Point(0, 0);

        Point center(0, 0);
        for (const auto& vertex : vertices) {
            center.x += vertex.x;
            center.y += vertex.y;
        }
        return Point(center.x / vertices.size(), center.y / vertices.size());
    }
};

class Ellipse : public Shape {
protected:
    Point focus1, focus2;
    double distanceSum;  // Сумма расстояний от любой точки эллипса до фокусов

    double getSemiMajorAxis() const { return distanceSum / 2.0; }
    double getFocalDistance() const { return focus1.distanceTo(focus2) / 2.0; }
    double getSemiMinorAxis() const {
        double a = getSemiMajorAxis();
        double c = getFocalDistance();
        return std::sqrt(std::max(0.0, a * a - c * c));
    }

public:
    Ellipse(const Point& f1, const Point& f2, double sumOfDistances)
        : focus1(f1), focus2(f2), distanceSum(sumOfDistances) {
    }

    Point getCenter() const override {
        return Point((focus1.x + focus2.x) / 2, (focus1.y + focus2.y) / 2);
    }

    double getEccentricity() const {
        return (distanceSum < E) ? 0 : (focus1.distanceTo(focus2) / distanceSum);
    }

    std::pair<Point, Point> getFoci() const {
        return { focus1, focus2 };
    }

    std::pair<Line, Line> getDirectrices() const {
        double a = getSemiMajorAxis();
        double e = getEccentricity();

        if (e < E) {
            return { Line(1.0, 0.0, 0.0), Line(1.0, 0.0, 0.0) };
        }

        Point center = getCenter();
        double d = a / e;
        double dx = focus2.x - focus1.x;
        double dy = focus2.y - focus1.y;
        double dist = std::hypot(dx, dy);

        Point dirPoint1(center.x + d * dx / dist, center.y + d * dy / dist);
        Point dirPoint2(center.x - d * dx / dist, center.y - d * dy / dist);

        return { Line(dx, dy, -(dx * dirPoint1.x + dy * dirPoint1.y)),
                Line(dx, dy, -(dx * dirPoint2.x + dy * dirPoint2.y)) };
    }

    double getPerimeter() const override {
        double a = getSemiMajorAxis();
        double b = getSemiMinorAxis();
        return PI * (3 * (a + b) - std::sqrt((3 * a + b) * (a + 3 * b)));
    }

    double getArea() const override {
        return PI * getSemiMajorAxis() * getSemiMinorAxis();
    }

    bool isEqualTo(const Shape& other) const override {
        const Ellipse* ellipsePtr = dynamic_cast<const Ellipse*>(&other);
        if (!ellipsePtr) return false;

        return std::abs(distanceSum - ellipsePtr->distanceSum) < E &&
            ((focus1 == ellipsePtr->focus1 && focus2 == ellipsePtr->focus2) ||
                (focus1 == ellipsePtr->focus2 && focus2 == ellipsePtr->focus1));
    }

    bool isCongruentTo(const Shape& other) const override {
        const Ellipse* ellipsePtr = dynamic_cast<const Ellipse*>(&other);
        if (!ellipsePtr) return false;

        return std::abs(getSemiMajorAxis() - ellipsePtr->getSemiMajorAxis()) < E &&
            std::abs(getSemiMinorAxis() - ellipsePtr->getSemiMinorAxis()) < E;
    }

    bool isSimilarTo(const Shape& other) const override {
        const Ellipse* ellipsePtr = dynamic_cast<const Ellipse*>(&other);
        if (!ellipsePtr) return false;

        return std::abs(getEccentricity() - ellipsePtr->getEccentricity()) < E;
    }

    bool containsPoint(Point point) const override {
        return focus1.distanceTo(point) + focus2.distanceTo(point) <= distanceSum + E;
    }

    void rotateAroundPoint(Point center, double angleDegrees) override {
        double radians = angleDegrees * PI / 180.0;
        double cosAngle = std::cos(radians);
        double sinAngle = std::sin(radians);

        auto rotatePoint = [&](Point p) {
            double dx = p.x - center.x;
            double dy = p.y - center.y;
            return Point(center.x + dx * cosAngle - dy * sinAngle,
                center.y + dx * sinAngle + dy * cosAngle);
            };

        focus1 = rotatePoint(focus1);
        focus2 = rotatePoint(focus2);
    }

    void reflectOverPoint(Point center) override {
        focus1 = Point(2 * center.x - focus1.x, 2 * center.y - focus1.y);
        focus2 = Point(2 * center.x - focus2.x, 2 * center.y - focus2.y);
    }

    void reflectOverLine(const Line& axis) override {
        double denominator = axis.A * axis.A + axis.B * axis.B;

        auto reflectPoint = [&](Point p) {
            double factor = 2 * (axis.A * p.x + axis.B * p.y + axis.C) / denominator;
            return Point(p.x - axis.A * factor, p.y - axis.B * factor);
            };

        focus1 = reflectPoint(focus1);
        focus2 = reflectPoint(focus2);
    }

    void scaleFromPoint(Point center, double coefficient) override {
        focus1 = Point(center.x + coefficient * (focus1.x - center.x),
            center.y + coefficient * (focus1.y - center.y));
        focus2 = Point(center.x + coefficient * (focus2.x - center.x),
            center.y + coefficient * (focus2.y - center.y));
        distanceSum *= std::abs(coefficient);
    }

    void draw(bool isFirstSelected, bool isSecondSelected) const override {
        if (isFirstSelected) glColor3f(1.0f, 0.0f, 0.0f);
        else if (isSecondSelected) glColor3f(0.0f, 0.8f, 0.0f);
        else glColor3f(0.0f, 0.0f, 1.0f);

        glLineWidth((isFirstSelected || isSecondSelected) ? 3.0f : 1.0f);

        Point center = getCenter();
        double a = getSemiMajorAxis();
        double b = getSemiMinorAxis();
        double angle = std::atan2(focus2.y - focus1.y, focus2.x - focus1.x);

        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < 100; ++i) {
            double theta = 2.0 * PI * i / 100;
            double dx = a * std::cos(theta);
            double dy = b * std::sin(theta);

            glVertex2f(center.x + dx * std::cos(angle) - dy * std::sin(angle),
                center.y + dx * std::sin(angle) + dy * std::cos(angle));
        }
        glEnd();

        glLineWidth(1.0f);
    }

    std::string getTypeName() const override { return "Ellipse"; }
};

// Круг 
class Circle : public Ellipse {
public:
    Circle(Point center, double radius) : Ellipse(center, center, 2 * radius) {}

    double getRadius() const { return distanceSum / 2.0; }
    std::string getTypeName() const override { return "Circle"; }

    bool isSimilarTo(const Shape& other) const override {
        return dynamic_cast<const Circle*>(&other) != nullptr;  // Все круги подобны
    }

    bool isCongruentTo(const Shape& other) const override {
        const Circle* circlePtr = dynamic_cast<const Circle*>(&other);
        return circlePtr && std::abs(getRadius() - circlePtr->getRadius()) < E;
    }
};

// Прямоугольник 
class Rectangle : public Polygon {
public:
    Rectangle(Point vertex1, Point vertex2, double sideRatio) {
        if (sideRatio < E) sideRatio = 1.0;

        Point center = (vertex1 + vertex2) / 2.0;
        double dx = vertex2.x - vertex1.x;
        double dy = vertex2.y - vertex1.y;

        double diagonalLength = std::hypot(dx, dy);
        double width = diagonalLength / std::sqrt(sideRatio * sideRatio + 1.0);
        double length = sideRatio * width;

        double angle = std::atan2(dy, dx);
        double alpha = std::atan(1.0 / sideRatio);

        Point v1(length / 2 * std::cos(angle - alpha),
            length / 2 * std::sin(angle - alpha));
        Point v2(width / 2 * std::cos(angle - alpha + PI / 2),
            width / 2 * std::sin(angle - alpha + PI / 2));

        vertices = {
            center + v1 + v2,
            center + v1 - v2,
            center - v1 - v2,
            center - v1 + v2
        };
    }

    std::pair<Line, Line> getDiagonals() const {
        return { Line(vertices[0], vertices[2]), Line(vertices[1], vertices[3]) };
    }

    std::string getTypeName() const override { return "Rectangle"; }

    bool isSimilarTo(const Shape& other) const override {
        const Rectangle* rectPtr = dynamic_cast<const Rectangle*>(&other);
        if (!rectPtr) return false;

        double ratio1 = vertices[0].distanceTo(vertices[1]) / vertices[1].distanceTo(vertices[2]);
        double ratio2 = rectPtr->vertices[0].distanceTo(rectPtr->vertices[1]) /
            rectPtr->vertices[1].distanceTo(rectPtr->vertices[2]);

        return std::abs(ratio1 - ratio2) < E;
    }
};

// Квадрат 
class Square : public Rectangle {
public:
    Square(Point vertex1, Point vertex2) : Rectangle(vertex1, vertex2, 1.0) {}

    Circle getInscribedCircle() const {
        return Circle(getCenter(), vertices[0].distanceTo(vertices[1]) / 2.0);
    }

    Circle getCircumscribedCircle() const {
        return Circle(getCenter(), getCenter().distanceTo(vertices[0]));
    }

    std::string getTypeName() const override { return "Square"; }

    bool isSimilarTo(const Shape& other) const override {
        return dynamic_cast<const Square*>(&other) != nullptr;  // Все квадраты подобны
    }

    bool isCongruentTo(const Shape& other) const override {
        const Square* squarePtr = dynamic_cast<const Square*>(&other);
        if (!squarePtr) return false;

        double side1 = vertices[0].distanceTo(vertices[1]);
        double side2 = squarePtr->vertices[0].distanceTo(squarePtr->vertices[1]);

        return std::abs(side1 - side2) < E;
    }
};

// Треугольник 
class Triangle : public Polygon {
private:
    // Вспомогательный метод для получения вершин из любой фигуры
    std::vector<Point> getVerticesFromShape(const Shape& shape) const {
        // Если это Triangle
        if (const Triangle* tri = dynamic_cast<const Triangle*>(&shape)) {
            return tri->getVertices();
        }
        // Если это Polygon с 3 вершинами
        if (const Polygon* poly = dynamic_cast<const Polygon*>(&shape)) {
            if (poly->getVertexCount() == 3) {
                return poly->getVertices();
            }
        }
        return {};
    }

    // Сравнение углов двух треугольников
    bool compareAngles(const std::vector<Point>& verts1, const std::vector<Point>& verts2) const {
        std::vector<double> angles1, angles2;

        for (int i = 0; i < 3; ++i) {
            // Угол 1
            Point a1 = verts1[i];
            Point b1 = verts1[(i + 1) % 3];
            Point c1 = verts1[(i + 2) % 3];

            double ab1 = b1.distanceTo(a1);
            double bc1 = c1.distanceTo(b1);
            double angle1 = std::acos(((b1 - a1).dot(c1 - b1)) / (ab1 * bc1));
            angles1.push_back(angle1);

            // Угол 2
            Point a2 = verts2[i];
            Point b2 = verts2[(i + 1) % 3];
            Point c2 = verts2[(i + 2) % 3];

            double ab2 = b2.distanceTo(a2);
            double bc2 = c2.distanceTo(b2);
            double angle2 = std::acos(((b2 - a2).dot(c2 - b2)) / (ab2 * bc2));
            angles2.push_back(angle2);
        }

        std::sort(angles1.begin(), angles1.end());
        std::sort(angles2.begin(), angles2.end());

        for (int i = 0; i < 3; ++i) {
            if (std::abs(angles1[i] - angles2[i]) > E) return false;
        }
        return true;
    }

    // Сравнение сторон двух треугольников
    bool compareSides(const std::vector<Point>& verts1, const std::vector<Point>& verts2) const {
        std::vector<double> sides1, sides2;

        for (int i = 0; i < 3; ++i) {
            sides1.push_back(verts1[i].distanceTo(verts1[(i + 1) % 3]));
            sides2.push_back(verts2[i].distanceTo(verts2[(i + 1) % 3]));
        }

        std::sort(sides1.begin(), sides1.end());
        std::sort(sides2.begin(), sides2.end());

        for (int i = 0; i < 3; ++i) {
            if (std::abs(sides1[i] - sides2[i]) > E) return false;
        }
        return true;
    }

public:
    Triangle(Point v1, Point v2, Point v3) {
        vertices = { v1, v2, v3 };
    }

    Point getCentroid() const {
        return (vertices[0] + vertices[1] + vertices[2]) / 3.0;
    }

    Point getOrthocenter() const {
        Point a = vertices[0], b = vertices[1], c = vertices[2];

        double A1 = c.x - b.x, B1 = c.y - b.y;
        double C1 = -(A1 * a.x + B1 * a.y);

        double A2 = c.x - a.x, B2 = c.y - a.y;
        double C2 = -(A2 * b.x + B2 * b.y);

        double denominator = A1 * B2 - A2 * B1;
        if (std::abs(denominator) < E) return getCentroid();

        double x = (B1 * C2 - B2 * C1) / denominator;
        double y = (A2 * C1 - A1 * C2) / denominator;

        return Point(x, y);
    }

    Line getEulerLine() const {
        return Line(getCentroid(), getOrthocenter());
    }

    Circle getCircumscribedCircle() const {
        Point a = vertices[0], b = vertices[1], c = vertices[2];

        double D = 2 * (a.x * (b.y - c.y) + b.x * (c.y - a.y) + c.x * (a.y - b.y));
        if (std::abs(D) < E) return Circle(a, 0);

        double Ux = (a.x * a.x + a.y * a.y) * (b.y - c.y) +
            (b.x * b.x + b.y * b.y) * (c.y - a.y) +
            (c.x * c.x + c.y * c.y) * (a.y - b.y);

        double Uy = (a.x * a.x + a.y * a.y) * (c.x - b.x) +
            (b.x * b.x + b.y * b.y) * (a.x - c.x) +
            (c.x * c.x + c.y * c.y) * (b.x - a.x);

        Point center(Ux / D, Uy / D);
        return Circle(center, center.distanceTo(a));
    }

    Circle getInscribedCircle() const {
        Point a = vertices[0], b = vertices[1], c = vertices[2];

        double sideA = b.distanceTo(c);
        double sideB = a.distanceTo(c);
        double sideC = a.distanceTo(b);
        double perimeter = sideA + sideB + sideC;

        Point center = (a * sideA + b * sideB + c * sideC) / perimeter;
        double radius = 2.0 * getArea() / perimeter;

        return Circle(center, radius);
    }

    Circle getNinePointsCircle() const {
        Point midAB = (vertices[0] + vertices[1]) / 2.0;
        Point midBC = (vertices[1] + vertices[2]) / 2.0;
        Point midCA = (vertices[2] + vertices[0]) / 2.0;

        return Triangle(midAB, midBC, midCA).getCircumscribedCircle();
    }

    std::string getTypeName() const override { return "Triangle"; }

    // Сравнение на ПОДОБИЕ (с поддержкой Polygon с 3 вершинами)
    bool isSimilarTo(const Shape& other) const override {
        // Получаем вершины другой фигуры
        std::vector<Point> otherVertices;

        // Если это Triangle
        if (const Triangle* trianglePtr = dynamic_cast<const Triangle*>(&other)) {
            otherVertices = trianglePtr->getVertices();
        }
        // Если это Polygon с 3 вершинами
        else if (const Polygon* polygonPtr = dynamic_cast<const Polygon*>(&other)) {
            if (polygonPtr->getVertexCount() == 3) {
                otherVertices = polygonPtr->getVertices();
            }
            else {
                return false;
            }
        }
        else {
            return false;
        }

        return compareAngles(vertices, otherVertices);
    }

    bool isCongruentTo(const Shape& other) const override {
        std::vector<Point> otherVertices;

        if (const Triangle* trianglePtr = dynamic_cast<const Triangle*>(&other)) {
            otherVertices = trianglePtr->getVertices();
        }
        else if (const Polygon* polygonPtr = dynamic_cast<const Polygon*>(&other)) {
            if (polygonPtr->getVertexCount() == 3) {
                otherVertices = polygonPtr->getVertices();
            }
            else {
                return false;
            }
        }
        else {
            return false;
        }

        return compareSides(vertices, otherVertices);
    }

    bool isEqualTo(const Shape& other) const override {
        std::vector<Point> otherVertices;

        if (const Triangle* trianglePtr = dynamic_cast<const Triangle*>(&other)) {
            otherVertices = trianglePtr->getVertices();
        }
        else if (const Polygon* polygonPtr = dynamic_cast<const Polygon*>(&other)) {
            if (polygonPtr->getVertexCount() == 3) {
                otherVertices = polygonPtr->getVertices();
            }
            else {
                return false;
            }
        }
        else {
            return false;
        }
        int n = vertices.size();
        for (int shift = 0; shift < n; ++shift) {
            bool matchForward = true;
            bool matchBackward = true;

            for (int i = 0; i < n; ++i) {
                if (vertices[i] != otherVertices[(i + shift) % n])
                    matchForward = false;
                if (vertices[i] != otherVertices[(n - 1 - i + shift) % n])
                    matchBackward = false;
            }

            if (matchForward || matchBackward) return true;
        }
        return false;
    }
};

std::vector<Shape*> shapes;
Shape* selectedShape1 = nullptr;
Shape* selectedShape2 = nullptr;
ApplicationMode currentMode = MODE_SELECT_FIRST;
std::vector<Point> tempPoints;

int windowWidth = 800, windowHeight = 600;
double dynamicValue = 1.0;
double zoomScale = 1.0;
bool gridSnap = true;
std::string statusMessage = "UI Panel is on the right. Draw shapes!";
Point currentMousePos;

struct UIButton {
    std::string label;
    int actionId;
};

std::vector<UIButton> uiMenu = {
    {"--- ADD SHAPE ---", -1},
    {"Point", 7},
    {"Line", 8},
    {"Polygon", 1},
    {"Ellipse", 2},
    {"Circle", 3},
    {"Rectangle", 4},
    {"Square", 5},
    {"Triangle", 6},
    {"--- SELECT ---", -1},
    {"Shape 1 (Red)", 10},
    {"Shape 2 (Green)", 11},
    {"--- INFO / COMPARE ---", -1},
    {"Area & Perimeter", 20},
    {"Get Center", 29},
    {"Get Radius", 28},
    {"Polygon Data", 43},
    {"Ellipse Info", 42},
    {"Ellipse Center & Ecc", 44},
    {"Check Equality (==)", 21},
    {"Check Congruent", 22},
    {"Check Similar", 23},
    {"Check Point Inside", 24},
    {"--- AUTO SHAPES ---", -1},
    {"Add Inscribed Circle", 25},
    {"Add Circumscribed Circle", 26},
    {"Add Diagonals", 27},
    {"Triangle Euler Line & Circle", 45},
    {"--- TRANSFORM ---", -1},
    {"Reflect over Point", 32},
    {"Reflect over Line", 33},
    {"--- SYSTEM ---", -1},
    {"Delete Shape 1", 40},
    {"Clear All", 41}
};

int uiPanelWidth = 240;

Point readPoint() {
    double x, y;
    std::cout << "Введите координаты точки (x y): ";
    std::cin >> x >> y;
    return Point(x, y);
}

void addCircleFromConsole() {
    std::cout << "\n=== ДОБАВЛЕНИЕ КРУГА ===" << std::endl;
    Point center = readPoint();
    double radius;
    std::cout << "Введите радиус: ";
    std::cin >> radius;
    shapes.push_back(new Circle(center, radius));
    std::cout << "Круг добавлен!" << std::endl;
}

void addRectangleFromConsole() {
    std::cout << "\n=== ДОБАВЛЕНИЕ ПРЯМОУГОЛЬНИКА ===" << std::endl;
    std::cout << "Первая вершина:" << std::endl;
    Point p1 = readPoint();
    std::cout << "Вторая вершина (противоположная):" << std::endl;
    Point p2 = readPoint();
    double ratio;
    std::cout << "Введите отношение сторон (длина/ширина): ";
    std::cin >> ratio;
    shapes.push_back(new Rectangle(p1, p2, ratio));
    std::cout << "Прямоугольник добавлен!" << std::endl;
}

void addSquareFromConsole() {
    std::cout << "\n=== ДОБАВЛЕНИЕ КВАДРАТА ===" << std::endl;
    std::cout << "Первая вершина:" << std::endl;
    Point p1 = readPoint();
    std::cout << "Вторая вершина (противоположная):" << std::endl;
    Point p2 = readPoint();
    shapes.push_back(new Square(p1, p2));
    std::cout << "Квадрат добавлен!" << std::endl;
}

void addTriangleFromConsole() {
    std::cout << "\n=== ДОБАВЛЕНИЕ ТРЕУГОЛЬНИКА ===" << std::endl;
    std::cout << "Вершина 1:" << std::endl;
    Point p1 = readPoint();
    std::cout << "Вершина 2:" << std::endl;
    Point p2 = readPoint();
    std::cout << "Вершина 3:" << std::endl;
    Point p3 = readPoint();
    shapes.push_back(new Triangle(p1, p2, p3));
    std::cout << "Треугольник добавлен!" << std::endl;
}

void addEllipseFromConsole() {
    std::cout << "\n=== ДОБАВЛЕНИЕ ЭЛЛИПСА ===" << std::endl;
    std::cout << "Фокус 1:" << std::endl;
    Point f1 = readPoint();
    std::cout << "Фокус 2:" << std::endl;
    Point f2 = readPoint();
    double sum;
    std::cout << "Введите сумму расстояний от точки эллипса до фокусов: ";
    std::cin >> sum;
    shapes.push_back(new Ellipse(f1, f2, sum));
    std::cout << "Эллипс добавлен!" << std::endl;
}

void addPolygonFromConsole() {
    std::cout << "\n=== ДОБАВЛЕНИЕ МНОГОУГОЛЬНИКА ===" << std::endl;
    int n;
    std::cout << "Введите количество вершин: ";
    std::cin >> n;
    if (n < 3) {
        std::cout << "Многоугольник должен иметь минимум 3 вершины!" << std::endl;
        return;
    }
    std::vector<Point> vertices;
    for (int i = 0; i < n; ++i) {
        std::cout << "Вершина " << (i + 1) << ":" << std::endl;
        vertices.push_back(readPoint());
    }
    shapes.push_back(new Polygon(vertices));
    std::cout << "Многоугольник добавлен!" << std::endl;
}

void addLineFromConsole() {
    std::cout << "\n=== ДОБАВЛЕНИЕ ЛИНИИ ===" << std::endl;
    std::cout << "Точка 1:" << std::endl;
    Point p1 = readPoint();
    std::cout << "Точка 2:" << std::endl;
    Point p2 = readPoint();
    shapes.push_back(new Line(p1, p2));
    std::cout << "Линия добавлена!" << std::endl;
}

void addPointFromConsole() {
    std::cout << "\n=== ДОБАВЛЕНИЕ ТОЧКИ ===" << std::endl;
    Point p = readPoint();
    shapes.push_back(new PointShape(p));
    std::cout << "Точка добавлена!" << std::endl;
}

void consoleMenu() {
    std::cout << "\n" << std::endl;
    std::cout << "                   КОНСОЛЬНОЕ МЕНЮ ФИГУР                      " << std::endl;
    std::cout << "  1. Добавить КРУГ                                           " << std::endl;
    std::cout << "  2. Добавить ПРЯМОУГОЛЬНИК                                  " << std::endl;
    std::cout << "  3. Добавить КВАДРАТ                                        " << std::endl;
    std::cout << "  4. Добавить ТРЕУГОЛЬНИК                                    " << std::endl;
    std::cout << "  5. Добавить ЭЛЛИПС                                         " << std::endl;
    std::cout << "  6. Добавить МНОГОУГОЛЬНИК                                  " << std::endl;
    std::cout << "  7. Добавить ЛИНИЮ                                          " << std::endl;
    std::cout << "  8. Добавить ТОЧКУ                                          " << std::endl;
    std::cout << "  0. ЗАВЕРШИТЬ ВВОД И ЗАПУСТИТЬ ГРАФИКУ                      " << std::endl;
    std::cout << "Ваш выбор: ";
}

void showAllShapesInfo() {
    if (shapes.empty()) {
        std::cout << "\nФигур пока нет!" << std::endl;
        return;
    }
    std::cout << "\n=== СПИСОК ВСЕХ ФИГУР ===" << std::endl;
    for (size_t i = 0; i < shapes.size(); ++i) {
        std::cout << (i + 1) << ". " << shapes[i]->getTypeName()
            << " | Центр: (" << shapes[i]->getCenter().x
            << ", " << shapes[i]->getCenter().y << ")"
            << " | Площадь: " << shapes[i]->getArea()
            << " | Периметр: " << shapes[i]->getPerimeter() << std::endl;
    }
}

Point screenToWorld(int x, int y) {
    float aspect = static_cast<float>(windowWidth) / windowHeight;
    double worldX = (2.0f * x / windowWidth - 1.0f) * 10.0f * aspect * zoomScale;
    double worldY = (1.0f - 2.0f * y / windowHeight) * 10.0f * zoomScale;

    if (gridSnap && currentMode != MODE_TRANSFORM_ROTATE &&
        currentMode != MODE_TRANSFORM_SCALE && currentMode != MODE_CHECK_POINT_INSIDE) {
        worldX = std::round(worldX * 2.0) / 2.0;
        worldY = std::round(worldY * 2.0) / 2.0;
    }
    return Point(worldX, worldY);
}

// Отрисовка текста
void drawText(float x, float y, const std::string& text, void* font = GLUT_BITMAP_9_BY_15) {
    glRasterPos2f(x, y);
    for (char c : text) {
        glutBitmapCharacter(font, c);
    }
}


void triggerMenuAction(int option) {
    switch (option) {
    case 1:
        currentMode = MODE_ADD_POLYGON;
        tempPoints.clear();
        statusMessage = "Click points. Press Enter to finish.";
        break;
    case 2:
        currentMode = MODE_ADD_ELLIPSE;
        tempPoints.clear();
        statusMessage = "Click Focus1, Focus2, then edge point.";
        break;
    case 3:
        currentMode = MODE_ADD_CIRCLE;
        tempPoints.clear();
        statusMessage = "Click center, move mouse, click to set radius.";
        break;
    case 4:
        currentMode = MODE_ADD_RECTANGLE;
        tempPoints.clear();
        statusMessage = "Click p1, p2, move mouse for ratio, click to fix.";
        break;
    case 5:
        currentMode = MODE_ADD_SQUARE;
        tempPoints.clear();
        statusMessage = "Click opposite corners.";
        break;
    case 6:
        currentMode = MODE_ADD_TRIANGLE;
        tempPoints.clear();
        statusMessage = "Click 3 vertices.";
        break;
    case 7:
        currentMode = MODE_ADD_POINT;
        tempPoints.clear();
        statusMessage = "Click anywhere to place a Point.";
        break;
    case 8:
        currentMode = MODE_ADD_LINE;
        tempPoints.clear();
        statusMessage = "Click 2 points to draw a Line.";
        break;

    case 10:
        currentMode = MODE_SELECT_FIRST;
        statusMessage = "Select Shape 1 (Red outline)";
        break;
    case 11:
        currentMode = MODE_SELECT_SECOND;
        statusMessage = "Select Shape 2 (Green outline)";
        break;

    case 20:
        if (selectedShape1) {
            char buf[100];
            std::snprintf(buf, sizeof(buf), "Area: %.2f, Perimeter: %.2f",
                selectedShape1->getArea(), selectedShape1->getPerimeter());
            statusMessage = buf;
        }
        else {
            statusMessage = "Select Shape 1 first!";
        }
        break;

    case 21:
        if (selectedShape1 && selectedShape2) {
            statusMessage = selectedShape1->isEqualTo(*selectedShape2) ?
                "Shapes are EQUAL" : "Shapes are NOT equal";
        }
        else {
            statusMessage = "Select both shapes first!";
        }
        break;

    case 22:
        if (selectedShape1 && selectedShape2) {
            statusMessage = selectedShape1->isCongruentTo(*selectedShape2) ?
                "Shapes are CONGRUENT" : "Shapes are NOT congruent";
        }
        else {
            statusMessage = "Select both shapes first!";
        }
        break;

    case 23:
        if (selectedShape1 && selectedShape2) {
            statusMessage = selectedShape1->isSimilarTo(*selectedShape2) ?
                "Shapes are SIMILAR" : "Shapes are NOT similar";
        }
        else {
            statusMessage = "Select both shapes first!";
        }
        break;

    case 24:
        currentMode = MODE_CHECK_POINT_INSIDE;
        statusMessage = "Click anywhere to check if point is INSIDE Shape 1";
        break;

    case 28: {
        if (!selectedShape1) {
            statusMessage = "Select Shape 1 first!";
            break;
        }
        Circle* circlePtr = dynamic_cast<Circle*>(selectedShape1);
        if (circlePtr) {
            char buf[100];
            std::snprintf(buf, sizeof(buf), "Circle Radius: %.2f", circlePtr->getRadius());
            statusMessage = buf;
        }
        else {
            statusMessage = "Shape 1 must be a Circle!";
        }
        break;
    }

    case 29: {
        if (!selectedShape1) {
            statusMessage = "Select Shape 1 first!";
            break;
        }
        Point center = selectedShape1->getCenter();
        shapes.push_back(new PointShape(center));
        char buf[128];
        std::snprintf(buf, sizeof(buf), "%s Center: (%.2f, %.2f). Point added!",
            selectedShape1->getTypeName().c_str(), center.x, center.y);
        statusMessage = buf;
        break;
    }

    case 42: {
        if (!selectedShape1) {
            statusMessage = "Select Shape 1 first!";
            break;
        }
        Ellipse* ellipsePtr = dynamic_cast<Ellipse*>(selectedShape1);
        if (ellipsePtr) {
            auto foci = ellipsePtr->getFoci();
            auto dirs = ellipsePtr->getDirectrices();
            shapes.push_back(new PointShape(foci.first));
            shapes.push_back(new PointShape(foci.second));
            if (ellipsePtr->getEccentricity() > E) {
                shapes.push_back(new Line(dirs.first));
                shapes.push_back(new Line(dirs.second));
            }
            statusMessage = "Foci (Points) and Directrices (Lines) added!";
        }
        else {
            statusMessage = "Shape 1 must be an Ellipse!";
        }
        break;
    }

    case 44: {
        if (!selectedShape1) {
            statusMessage = "Select Shape 1 first!";
            break;
        }
        Ellipse* ellipsePtr = dynamic_cast<Ellipse*>(selectedShape1);
        if (ellipsePtr) {
            double ecc = ellipsePtr->getEccentricity();
            Point c = ellipsePtr->getCenter();
            shapes.push_back(new PointShape(c));
            char buf[128];
            std::snprintf(buf, sizeof(buf), "Center: (%.2f, %.2f) | Eccentricity: %.3f",
                c.x, c.y, ecc);
            statusMessage = buf;
        }
        else {
            statusMessage = "Shape 1 must be an Ellipse!";
        }
        break;
    }

    case 43: {
        if (!selectedShape1) {
            statusMessage = "Select Shape 1 first!";
            break;
        }
        Polygon* polygonPtr = dynamic_cast<Polygon*>(selectedShape1);
        if (polygonPtr) {
            int vertexCount = polygonPtr->getVertexCount();
            std::stringstream ss;
            ss << "Vertices:" << vertexCount << ", Convex:"
                << (polygonPtr->isConvex() ? "Yes" : "No") << ". Points:";
            auto verts = polygonPtr->getVertices();
            for (int i = 0; i < std::min(vertexCount, 4); ++i) {
                ss << "(" << std::round(verts[i].x * 10) / 10
                    << "," << std::round(verts[i].y * 10) / 10 << ") ";
            }
            if (vertexCount > 4) ss << "...";
            statusMessage = ss.str();
        }
        else {
            statusMessage = "Shape 1 must be a Polygon!";
        }
        break;
    }

    case 25: {
        if (!selectedShape1) {
            statusMessage = "Select Shape 1 first!";
            break;
        }
        Triangle* trianglePtr = dynamic_cast<Triangle*>(selectedShape1);
        if (trianglePtr) {
            shapes.push_back(new Circle(trianglePtr->getInscribedCircle()));
            statusMessage = "Inscribed circle added to Triangle.";
            break;
        }
        Square* squarePtr = dynamic_cast<Square*>(selectedShape1);
        if (squarePtr) {
            shapes.push_back(new Circle(squarePtr->getInscribedCircle()));
            statusMessage = "Inscribed circle added to Square.";
            break;
        }
        statusMessage = "Shape must be Triangle or Square!";
        break;
    }

    case 26: {
        if (!selectedShape1) {
            statusMessage = "Select Shape 1 first!";
            break;
        }
        Triangle* trianglePtr = dynamic_cast<Triangle*>(selectedShape1);
        if (trianglePtr) {
            shapes.push_back(new Circle(trianglePtr->getCircumscribedCircle()));
            statusMessage = "Circumscribed circle added to Triangle.";
            break;
        }
        Square* squarePtr = dynamic_cast<Square*>(selectedShape1);
        if (squarePtr) {
            shapes.push_back(new Circle(squarePtr->getCircumscribedCircle()));
            statusMessage = "Circumscribed circle added to Square.";
            break;
        }
        statusMessage = "Shape must be Triangle or Square!";
        break;
    }

    case 27: {
        if (!selectedShape1) {
            statusMessage = "Select Shape 1 first!";
            break;
        }
        Polygon* polygonPtr = dynamic_cast<Polygon*>(selectedShape1);
        if (polygonPtr) {
            auto vertices = polygonPtr->getVertices();
            int n = vertices.size(), addedCount = 0;
            for (int i = 0; i < n - 2; ++i) {
                for (int j = i + 2; j < n; ++j) {
                    if (i == 0 && j == n - 1) continue;
                    shapes.push_back(new Line(vertices[i], vertices[j]));
                    addedCount++;
                }
            }
            statusMessage = addedCount > 0 ? "Added diagonals." : "No diagonals found.";
        }
        else {
            statusMessage = "Shape 1 must be a Polygon!";
        }
        break;
    }

    case 45: {
        if (!selectedShape1) {
            statusMessage = "Select Shape 1 first!";
            break;
        }
        Triangle* trianglePtr = dynamic_cast<Triangle*>(selectedShape1);
        if (trianglePtr) {
            Point centroid = trianglePtr->getCentroid();
            Point orthocenter = trianglePtr->getOrthocenter();
            shapes.push_back(new PointShape(centroid));
            shapes.push_back(new PointShape(orthocenter));
            shapes.push_back(new Line(trianglePtr->getEulerLine()));
            shapes.push_back(new Circle(trianglePtr->getNinePointsCircle()));

            char buf[128];
            std::snprintf(buf, sizeof(buf),
                "Centroid(%.1f,%.1f), Orthocenter(%.1f,%.1f). Euler Line & Circle added!",
                centroid.x, centroid.y, orthocenter.x, orthocenter.y);
            statusMessage = buf;
        }
        else {
            statusMessage = "Shape 1 must be a Triangle!";
        }
        break;
    }

    case 32:
        currentMode = MODE_TRANSFORM_REFLECT_POINT;
        statusMessage = "Click point to reflect Shape 1 over it";
        break;

    case 33:
        currentMode = MODE_TRANSFORM_REFLECT_LINE;
        tempPoints.clear();
        statusMessage = "Click 2 points for line of reflection";
        break;

    case 40:
        if (selectedShape1) {
            shapes.erase(std::remove(shapes.begin(), shapes.end(), selectedShape1), shapes.end());
            delete selectedShape1;
            selectedShape1 = nullptr;
            statusMessage = "Shape 1 deleted.";
        }
        else {
            statusMessage = "No shape selected!";
        }
        break;

    case 41:
        for (auto shape : shapes) delete shape;
        shapes.clear();
        selectedShape1 = selectedShape2 = nullptr;
        statusMessage = "All shapes cleared.";
        break;
    }
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float aspect = static_cast<float>(windowWidth) / windowHeight;
    glOrtho(-10 * aspect * zoomScale, 10 * aspect * zoomScale,
        -10 * zoomScale, 10 * zoomScale, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glColor3f(0.9f, 0.9f, 0.9f);
    glBegin(GL_LINES);
    for (int i = -50; i <= 50; i++) {
        glVertex2f(i, -50);
        glVertex2f(i, 50);
        glVertex2f(-50, i);
        glVertex2f(50, i);
    }
    glEnd();

    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_LINES);
    glVertex2f(-50, 0);
    glVertex2f(50, 0);
    glVertex2f(0, -50);
    glVertex2f(0, 50);
    glEnd();

    for (const auto& shape : shapes) {
        shape->draw(shape == selectedShape1, shape == selectedShape2);
    }

    if (!tempPoints.empty()) {
        glColor3f(0.5f, 0.5f, 0.5f);
        glPointSize(5.0f);
        glBegin(GL_POINTS);
        for (const auto& point : tempPoints) {
            glVertex2f(point.x, point.y);
        }
        glEnd();

        if (currentMode == MODE_ADD_POLYGON) {
            glBegin(GL_LINE_STRIP);
            for (const auto& point : tempPoints) {
                glVertex2f(point.x, point.y);
            }
            glEnd();
        }
        else if (currentMode == MODE_ADD_CIRCLE && tempPoints.size() == 1) {
            Circle(tempPoints[0], dynamicValue).draw(false, false);
        }
        else if (currentMode == MODE_ADD_RECTANGLE && tempPoints.size() == 2) {
            Rectangle(tempPoints[0], tempPoints[1], dynamicValue).draw(false, false);
        }
        else if (currentMode == MODE_ADD_LINE && tempPoints.size() == 1) {
            glBegin(GL_LINES);
            glVertex2f(tempPoints[0].x, tempPoints[0].y);
            glVertex2f(currentMousePos.x, currentMousePos.y);
            glEnd();
        }
    }

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, windowWidth, windowHeight, 0);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.9f, 0.9f, 0.9f, 0.95f);
    glBegin(GL_QUADS);
    glVertex2f(windowWidth - uiPanelWidth, 0);
    glVertex2f(windowWidth, 0);
    glVertex2f(windowWidth, windowHeight);
    glVertex2f(windowWidth - uiPanelWidth, windowHeight);
    glEnd();
    glDisable(GL_BLEND);

    glColor3f(0.5f, 0.5f, 0.5f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex2f(windowWidth - uiPanelWidth, 0);
    glVertex2f(windowWidth - uiPanelWidth, windowHeight);
    glEnd();
    glLineWidth(1.0f);

    for (size_t i = 0; i < uiMenu.size(); ++i) {
        int yPos = 14 + i * 14;
        if (uiMenu[i].actionId == -1) {
            glColor3f(0.4f, 0.4f, 0.4f);
            drawText(windowWidth - uiPanelWidth + 10, yPos, uiMenu[i].label, GLUT_BITMAP_8_BY_13);
        }
        else {
            glColor3f(0.0f, 0.0f, 0.0f);
            drawText(windowWidth - uiPanelWidth + 20, yPos, uiMenu[i].label, GLUT_BITMAP_9_BY_15);
        }
    }

    glColor3f(0.0f, 0.0f, 0.0f);
    drawText(10, 20, "Status: " + statusMessage);
    if (gridSnap) drawText(10, 40, "Grid Snap: ON (Press 'G' to toggle)");
    drawText(10, windowHeight - 20, "Hotkeys: 1/2 = Scale, A/D = Rotate. W/S = Zoom. Right-Click = Cancel");

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glutSwapBuffers();
}

void mouse(int button, int state, int x, int y) {
    if (state != GLUT_DOWN) return;

    if (button == GLUT_RIGHT_BUTTON) {
        currentMode = MODE_SELECT_FIRST;
        tempPoints.clear();
        statusMessage = "Action Cancelled. Mode: Select Shape 1";
        glutPostRedisplay();
        return;
    }

    if (button != GLUT_LEFT_BUTTON) return;

    if (x > windowWidth - uiPanelWidth) {
        int buttonIndex = (y - 4) / 14;
        if (buttonIndex >= 0 && buttonIndex < (int)uiMenu.size() &&
            uiMenu[buttonIndex].actionId != -1) {
            triggerMenuAction(uiMenu[buttonIndex].actionId);
        }
        glutPostRedisplay();
        return;
    }

    Point worldPosition = screenToWorld(x, y);

    if (currentMode == MODE_SELECT_FIRST || currentMode == MODE_SELECT_SECOND) {
        Shape* clickedShape = nullptr;
        for (auto& shape : shapes) {
            if (shape->getTypeName() == "Point") {
                if (shape->getCenter().distanceTo(worldPosition) < 0.3 * zoomScale) {
                    clickedShape = shape;
                }
            }
            else if (shape->containsPoint(worldPosition)) {
                clickedShape = shape;
            }
        }
        if (currentMode == MODE_SELECT_FIRST) {
            selectedShape1 = clickedShape;
        }
        else {
            selectedShape2 = clickedShape;
        }
        statusMessage = "Shape selected.";
    }
    else if (currentMode == MODE_CHECK_POINT_INSIDE) {
        if (!selectedShape1) {
            statusMessage = "Please select Shape 1 first!";
        }
        else {
            bool inside = selectedShape1->containsPoint(worldPosition);
            char buf[100];
            std::snprintf(buf, sizeof(buf), "Point (%.1f, %.1f) is %s Shape 1",
                worldPosition.x, worldPosition.y, inside ? "INSIDE" : "OUTSIDE");
            statusMessage = buf;
            currentMode = MODE_SELECT_FIRST;
        }
    }
    else if (currentMode == MODE_ADD_POINT) {
        shapes.push_back(new PointShape(worldPosition));
        statusMessage = "Point added.";
        currentMode = MODE_SELECT_FIRST;
    }
    else if (currentMode == MODE_ADD_LINE) {
        tempPoints.push_back(worldPosition);
        if (tempPoints.size() == 2) {
            shapes.push_back(new Line(tempPoints[0], tempPoints[1]));
            tempPoints.clear();
            currentMode = MODE_SELECT_FIRST;
            statusMessage = "Line added.";
        }
    }
    else if (currentMode == MODE_ADD_POLYGON) {
        tempPoints.push_back(worldPosition);
    }
    else if (currentMode == MODE_ADD_TRIANGLE) {
        tempPoints.push_back(worldPosition);
        if (tempPoints.size() == 3) {
            shapes.push_back(new Triangle(tempPoints[0], tempPoints[1], tempPoints[2]));
            tempPoints.clear();
            currentMode = MODE_SELECT_FIRST;
            statusMessage = "Triangle added.";
        }
    }
    else if (currentMode == MODE_ADD_SQUARE) {
        tempPoints.push_back(worldPosition);
        if (tempPoints.size() == 2) {
            shapes.push_back(new Square(tempPoints[0], tempPoints[1]));
            tempPoints.clear();
            currentMode = MODE_SELECT_FIRST;
            statusMessage = "Square added.";
        }
    }
    else if (currentMode == MODE_ADD_CIRCLE) {
        if (tempPoints.empty()) {
            tempPoints.push_back(worldPosition);
            statusMessage = "Move mouse to set radius, click to confirm.";
        }
        else {
            shapes.push_back(new Circle(tempPoints[0], dynamicValue));
            tempPoints.clear();
            currentMode = MODE_SELECT_FIRST;
            statusMessage = "Circle added.";
        }
    }
    else if (currentMode == MODE_ADD_RECTANGLE) {
        if (tempPoints.size() < 2) {
            tempPoints.push_back(worldPosition);
            statusMessage = "Click opposite corner.";
        }
        else {
            shapes.push_back(new Rectangle(tempPoints[0], tempPoints[1], dynamicValue));
            tempPoints.clear();
            currentMode = MODE_SELECT_FIRST;
            statusMessage = "Rectangle added.";
        }
    }
    else if (currentMode == MODE_ADD_ELLIPSE) {
        tempPoints.push_back(worldPosition);
        if (tempPoints.size() == 3) {
            shapes.push_back(new Ellipse(tempPoints[0], tempPoints[1],
                tempPoints[2].distanceTo(tempPoints[0]) +
                tempPoints[2].distanceTo(tempPoints[1])));
            tempPoints.clear();
            currentMode = MODE_SELECT_FIRST;
            statusMessage = "Ellipse added.";
        }
    }
    else if (currentMode == MODE_TRANSFORM_REFLECT_POINT && selectedShape1) {
        selectedShape1->reflectOverPoint(worldPosition);
        currentMode = MODE_SELECT_FIRST;
        statusMessage = "Shape reflected over point.";
    }
    else if (currentMode == MODE_TRANSFORM_REFLECT_LINE && selectedShape1) {
        tempPoints.push_back(worldPosition);
        if (tempPoints.size() == 2) {
            selectedShape1->reflectOverLine(Line(tempPoints[0], tempPoints[1]));
            tempPoints.clear();
            currentMode = MODE_SELECT_FIRST;
            statusMessage = "Shape reflected over line.";
        }
        else {
            statusMessage = "Click second point for reflection line.";
        }
    }
    glutPostRedisplay();
}

void motion(int x, int y) {
    Point worldPosition = screenToWorld(x, y);

    if (currentMode == MODE_ADD_CIRCLE && !tempPoints.empty()) {
        dynamicValue = tempPoints[0].distanceTo(worldPosition);
    }
    else if (currentMode == MODE_ADD_RECTANGLE && tempPoints.size() == 2) {
        dynamicValue = std::max(0.1, std::abs(worldPosition.y - tempPoints[0].y));
    }
    glutPostRedisplay();
}

void passiveMotion(int x, int y) {
    currentMousePos = screenToWorld(x, y);
    motion(x, y);
}

void keyboard(unsigned char key, int x, int y) {
    if (key == 'w' || key == 'W') {
        zoomScale *= 0.9;
        statusMessage = "Zoomed in";
    }
    else if (key == 's' || key == 'S') {
        zoomScale *= 1.1;
        statusMessage = "Zoomed out";
    }
    else if (key == 'g' || key == 'G') {
        gridSnap = !gridSnap;
        statusMessage = gridSnap ? "Grid Snap Enabled" : "Grid Snap Disabled";
    }
    else if (key == 13 && currentMode == MODE_ADD_POLYGON && tempPoints.size() >= 3) {
        shapes.push_back(new Polygon(tempPoints));
        tempPoints.clear();
        currentMode = MODE_SELECT_FIRST;
        statusMessage = "Polygon added.";
    }

    if (selectedShape1) {
        Point center = selectedShape1->getCenter();
        if (key == '1') {
            selectedShape1->scaleFromPoint(center, 0.95);
            statusMessage = "Scaled down.";
        }
        else if (key == '2') {
            selectedShape1->scaleFromPoint(center, 1.05);
            statusMessage = "Scaled up.";
        }
        else if (key == 'a' || key == 'A') {
            selectedShape1->rotateAroundPoint(center, 5.0);
            statusMessage = "Rotated left.";
        }
        else if (key == 'd' || key == 'D') {
            selectedShape1->rotateAroundPoint(center, -5.0);
            statusMessage = "Rotated right.";
        }
    }
    glutPostRedisplay();
}

void reshape(int w, int h) {
    windowWidth = w;
    windowHeight = h;
    glViewport(0, 0, w, h);
    glutPostRedisplay();
}

int main(int argc, char** argv) {
    setlocale(LC_ALL, "RU");
    int choice;
    do {
        consoleMenu();
        std::cin >> choice;

        switch (choice) {
        case 1: addCircleFromConsole(); break;
        case 2: addRectangleFromConsole(); break;
        case 3: addSquareFromConsole(); break;
        case 4: addTriangleFromConsole(); break;
        case 5: addEllipseFromConsole(); break;
        case 6: addPolygonFromConsole(); break;
        case 7: addLineFromConsole(); break;
        case 8: addPointFromConsole(); break;
        case 0:
            break;
        default:
            std::cout << "Неверный выбор! Попробуйте снова." << std::endl;
            break;
        }

        if (choice >= 1 && choice <= 8) {
            showAllShapesInfo();
        }

    } while (choice != 0);

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("Geometry CAD - Complete Application");

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutPassiveMotionFunc(passiveMotion);
    glutKeyboardFunc(keyboard);

    glutMainLoop();

    for (auto shape : shapes) {
        delete shape;
    }

    return 0;
}
