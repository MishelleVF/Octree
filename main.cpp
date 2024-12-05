#include <iostream>
#include <vector>
#define capacity 2
#define child_num 8

struct Point {
    double x, y, z;

    explicit Point(const double x, const double y, const double z) : x(x), y(y), z(z) {}

    //comparar si dos puntos son iguales
    bool operator==(const Point &) const {
        return x == x && y == y && z == z;
    }
};

struct Node {
    Point points[capacity + 1];
    Node * children[child_num]{};
    bool is_leaf = true;
    int num_points = 0;

    explicit Node(): points{} {
        for (auto & i : children) {
            i = nullptr;
        }
        for (auto & point : points) {
            point.x = 0;
            point.y = 0;
            point.z = 0;
        }
    }

    Node *split() {

    }

    bool insert(Point point) {
        // caso 1. el nodo es hoja
        // si es hoja, lo pongo en el arreglo.

        if (is_leaf) {
            // buscar si ya esta en el arreglo
            points[num_points++] = point;
            return true;
        }

        // caso 2. el nodo es interno
        // encuentro en cual de mis 8 hijos voy a poner el punto
        // llamo a insert desde ese nodo
        int child_index = find_child_to_insert(point);

        children[child_index]->insert(point);

        if (children[child_index]->num_points == capacity + 1) {
            children[child_index] = children[child_index]->split();
        }

        return true;
    }
};

class Octree {
private:
    Node* root = nullptr;
    Point inf, sup; //esto determina el espacio del octree

public:
    explicit Octree(Point inf, Point sup): inf(inf), sup(sup) {}

    // trata de insertar el punto en el arbol
    // si ya existe, retorna false, cc retorna true y lo inserta
    bool insert(Point point) {
        if (root == nullptr) {
            root = new Node();
            root->is_leaf = true;
            root->points[0] = point;
            root->num_points = 1;
            return true;
        }
        //chequear funcion
        return root->insert(point);
    }

    // verifica si el punto esta en el arbol,
    // si lo encuentra, retorna true, cc retorna false
    bool search(Point point) {

    }

    // retorna todos los puntos que estan dentro de la esfera formada
    // por la query y su radio
    std::vector<Point> radius_query(Point query, double radius) {

    }

    // retorna el punto mas cercano a la query
    std::vector<Point> _1nn_search(Point query) {

    }

    // trata de remover el punto del arbol,
    // si no esta, retorna false, cc lo remueve y retorna true
    bool remove(Point point) {

    }
};

int main() {


    return 0;
}
