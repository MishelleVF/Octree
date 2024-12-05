#include <iostream>
#include <vector>
#include <cmath>
#define capacity 2
#define child_num 8

using namespace std; 

struct Point {
    double x, y, z;

    Point() : x(0), y(0), z(0) {}
    Point(double x, double y, double z) : x(x), y(y), z(z) {}

    bool operator==(const Point &other) const {
        return x == other.x && y == other.y && z == other.z;
    }

    double distance(const Point &other) const {
        return sqrt(pow(x - other.x, 2) + pow(y - other.y, 2) + pow(z - other.z, 2));
    }
};

struct Node {
    Point points[capacity + 1];
    Node *children[child_num]{};
    bool is_leaf = true;
    int num_points = 0;
    Point center;
    double half_size;

    explicit Node(Point center = Point(), double half_size = 0) : center(center), half_size(half_size) {
        for (auto &child : children) {
            child = nullptr;
        }
    }

    Node *split() {
        is_leaf = false;
        double quarter = half_size / 2;

        for (int i = 0; i < child_num; i++) {
            double offsetX = (i & 1) ? quarter : -quarter;
            double offsetY = (i & 2) ? quarter : -quarter;
            double offsetZ = (i & 4) ? quarter : -quarter;

            Point child_center(center.x + offsetX, center.y + offsetY, center.z + offsetZ);
            children[i] = new Node(child_center, quarter);
        }

        for (int i = 0; i < num_points; i++) {
            int child_index = find_child_to_insert(points[i]);
            children[child_index]->insert(points[i]);
        }

        num_points = 0;
        return this;
    }

    int find_child_to_insert(const Point &point) const {
        int index = 0;
        index |= (point.x >= center.x) ? 1 : 0;
        index |= (point.y >= center.y) ? 2 : 0;
        index |= (point.z >= center.z) ? 4 : 0;
        return index;
    }

    bool insert(const Point &point) {
        if (is_leaf) {
            for (int i = 0; i < num_points; i++) {
                if (points[i] == point) return false;
            }
            points[num_points++] = point;
            if (num_points > capacity) split();
            return true;
        }

        int child_index = find_child_to_insert(point);
        return children[child_index]->insert(point);
    }

    bool remove(const Point &point) {
        if (is_leaf) {
            for (int i = 0; i < num_points; i++) {
                if (points[i] == point) {
                    points[i] = points[num_points - 1]; // Reemplazar con el último punto
                    num_points--;
                    return true;
                }
            }
            return false;
        }

        int child_index = find_child_to_insert(point);
        if (children[child_index] && children[child_index]->remove(point)) {
            // Verificar si el hijo quedó vacío
            if (children[child_index]->is_leaf && children[child_index]->num_points == 0) {
                delete children[child_index];
                children[child_index] = nullptr;
            }

            // Convertir el nodo a hoja si todos los hijos están vacíos
            bool all_children_empty = true;
            for (Node *child : children) {
                if (child != nullptr) {
                    all_children_empty = false;
                    break;
                }
            }

            if (all_children_empty) {
                is_leaf = true;
            }
            return true;
        }
        return false;
    }

    bool search(const Point &point) {
        if (is_leaf) {
            for (int i = 0; i < num_points; i++) {
                if (points[i] == point) return true;
            }
            return false;
        }

        int child_index = find_child_to_insert(point);
        return children[child_index]->search(point);
    }
};

class Octree {
private:
    Node *root = nullptr;
    Point inf, sup;

public:
    explicit Octree(Point inf, Point sup) : inf(inf), sup(sup) {
        Point center((inf.x + sup.x) / 2, (inf.y + sup.y) / 2, (inf.z + sup.z) / 2);
        double half_size = max(max(sup.x - inf.x, sup.y - inf.y), sup.z - inf.z) / 2;
        root = new Node(center, half_size);
    }

    bool insert(Point point) {
        return root->insert(point);
    }

    bool remove(Point point) {
        return root->remove(point);
    }

    bool search(Point point) {
        return root->search(point);
    }

    // retorna todos los puntos que estan dentro de la esfera formada
    // por la query y su radio
    vector<Point> radius_query(Point query, double radius) {

    }

    // retorna el punto mas cercano a la query
    vector<Point> _1nn_search(Point query) {

    }
};

int main() {
    Point inf(0, 0, 0);
    Point sup(10, 10, 10);
    Octree octree(inf, sup);

    octree.insert(Point(1, 1, 1));
    octree.insert(Point(2, 2, 2));
    octree.insert(Point(3, 3, 3));

    if (octree.search(Point(1, 1, 1))) {
        std::cout << "Point (1, 1, 1) encontrado" << std::endl;
    }

    std::cout << "Eliminando Point (1, 1, 1)" << std::endl;
    if (octree.remove(Point(1, 1, 1))) {
        std::cout << "Point (1, 1, 1) eliminado" << std::endl;
    }

    if (!octree.search(Point(1, 1, 1))) {
        std::cout << "Point (1, 1, 1) no está en el Octree." << std::endl;
    }

    return 0;
}

//hasta aqui todo funciona bien
