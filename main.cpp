#include <iostream>
#include <vector>
#include <cmath>
#include <functional>
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

    vector<Point> radius_query(Point query, double radius) {
        vector<Point> result;
        function<void(Node *)> search_in_radius = [&](Node *node) {
            if (!node) return;

            // Verificar si el nodo está dentro de la esfera
            double distance_to_node = node->center.distance(query);
            if (distance_to_node > node->half_size + radius) {
                return; // Nodo fuera del alcance
            }

            if (node->is_leaf) {
                for (int i = 0; i < node->num_points; i++) {
                    if (query.distance(node->points[i]) <= radius) {
                        result.push_back(node->points[i]);
                    }
                }
            } else {
                for (auto &child : node->children) {
                    search_in_radius(child);
                }
            }
        };

        search_in_radius(root);
        return result;
    }

    vector<Point> _1nn_search(Point query) {
        Point closest_point;
        double min_distance = numeric_limits<double>::max();

        function<void(Node *)> search_nearest = [&](Node *node) {
            if (!node) return;

            // Calcular distancia aproximada al nodo
            double distance_to_node = node->center.distance(query);
            if (distance_to_node > node->half_size + min_distance) {
                return; // Nodo fuera de alcance
            }

            if (node->is_leaf) {
                for (int i = 0; i < node->num_points; i++) {
                    double dist = query.distance(node->points[i]);
                    if (dist < min_distance) {
                        min_distance = dist;
                        closest_point = node->points[i];
                    }
                }
            } else {
                for (auto &child : node->children) {
                    search_nearest(child);
                }
            }
        };

        search_nearest(root);
        return {closest_point};
    }

};

int main() {
    Point inf(0, 0, 0);
    Point sup(10, 10, 10);
    Octree octree(inf, sup);

    cout << "Octree1---------------------------------------" << endl;

    octree.insert(Point(1, 1, 1));
    octree.insert(Point(2, 2, 2));
    octree.insert(Point(3, 3, 3));

    if (octree.search(Point(1, 1, 1))) {
        cout << "Point (1, 1, 1) encontrado" << endl;
    }

    cout << "Eliminando Point (1, 1, 1)" << endl;
    if (octree.remove(Point(1, 1, 1))) {
        cout << "Point (1, 1, 1) eliminado" << endl;
    }

    if (!octree.search(Point(1, 1, 1))) {
        cout << "Point (1, 1, 1) no esta en el Octree." << endl;
    }

    cout << "Octree2---------------------------------------" << endl;
    Octree octree1(Point(0, 0, 0), Point(10, 10, 10));
    octree1.insert(Point(1, 1, 1));
    octree1.insert(Point(5, 5, 5));
    octree1.insert(Point(7, 8, 9));

    // Búsqueda de puntos en un radio de 3 alrededor de (5, 5, 5)
    vector<Point> points_in_radius = octree1.radius_query(Point(5, 5, 5), 3);
    for (auto &p : points_in_radius) {
        cout << p.x << ", " << p.y << ", " << p.z << endl;
    }

    // Búsqueda del punto más cercano a (6, 6, 6)
    vector<Point> nearest_point = octree1._1nn_search(Point(6, 6, 6));
    for (auto &p : nearest_point) {
        cout << p.x << ", " << p.y << ", " << p.z << endl;
    }

    cout << "Octree3---------------------------------------" << endl;
    Octree octree2(Point(0, 0, 0), Point(20, 20, 20));

    // Insertar varios puntos en el Octree
    octree2.insert(Point(1, 1, 1));
    octree2.insert(Point(5, 5, 5));
    octree2.insert(Point(7, 8, 9));
    octree2.insert(Point(10, 10, 10));
    octree2.insert(Point(15, 15, 15));
    octree2.insert(Point(18, 18, 18));
    octree2.insert(Point(3, 4, 3));

    // Búsqueda de puntos dentro de un radio de 5 unidades alrededor de (6, 6, 6)
    cout << "Puntos dentro de un radio de 5 alrededor de (6, 6, 6):" << endl;
    vector<Point> points_in_radius2 = octree2.radius_query(Point(6, 6, 6), 5);
    for (auto &p : points_in_radius2) {
        cout << "(" << p.x << ", " << p.y << ", " << p.z << ")" << endl;
    }

    // Búsqueda de puntos dentro de un radio de 10 unidades alrededor de (10, 10, 10)
    cout << "\nPuntos dentro de un radio de 10 alrededor de (10, 10, 10):" << endl;
    vector<Point> points_in_radius3 = octree2.radius_query(Point(10, 10, 10), 10);
    for (auto &p : points_in_radius3) {
        cout << "(" << p.x << ", " << p.y << ", " << p.z << ")" << endl;
    }

    // Búsqueda del punto más cercano a (6, 6, 6)
    cout << "\nPunto mas cercano a (6, 6, 6):" << endl;
    vector<Point> nearest_point3 = octree2._1nn_search(Point(6, 6, 6));
    for (auto &p : nearest_point3) {
        cout << "(" << p.x << ", " << p.y << ", " << p.z << ")" << endl;
    }

    // Búsqueda del punto más cercano a (12, 12, 12)
    cout << "\nPunto mas cercano a (12, 12, 12):" << endl;
    vector<Point> nearest_point2 = octree2._1nn_search(Point(12, 12, 12));
    for (auto &p : nearest_point2) {
        cout << "(" << p.x << ", " << p.y << ", " << p.z << ")" << endl;
    }

    return 0;
}
