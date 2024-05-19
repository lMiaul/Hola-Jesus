#include <iostream>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <ctime>

using namespace std;

class BTreeNode {
    int t;  // Grado mínimo (define el rango de número de claves)
    vector<int> keys; // Vector de claves
    vector<BTreeNode*> children; // Vector de punteros a los hijos
    int n;     // Número actual de claves
    bool leaf; // Es verdadero cuando el nodo es hoja
public:
    BTreeNode(int _t, bool _leaf);

    // Función para recorrer todos los nodos en un subárbol enraizado con este nodo
    void traverse();

    // Función para insertar una nueva clave en el subárbol enraizado con este nodo
    void insertNonFull(int k);

    // Función para dividir el hijo y de este nodo. i es el índice de y en el vector children
    void splitChild(int i, BTreeNode* y);

    friend class BTree;
};

class BTree {
    BTreeNode* root;
    int t;  // Grado mínimo
public:
    BTree(int _t) {
        root = nullptr;
        t = _t;
    }

    void traverse() {
        if (root != nullptr) root->traverse();
    }

    void insert(int k);
};

BTreeNode::BTreeNode(int t1, bool leaf1) {
    t = t1;
    leaf = leaf1;
    n = 0;
    keys.resize(2*t-1);
    children.resize(2*t);
}

void BTreeNode::traverse() {
    int i;
    for (i = 0; i < n; i++) {
        // Si este nodo no es hoja, entonces antes de imprimir la clave[i],
        // recorre el subárbol enraizado con el hijo children[i].
        if (!leaf) children[i]->traverse();
        cout << " " << keys[i];
    }

    // Imprime el subárbol enraizado con el último hijo
    if (!leaf) children[i]->traverse();
}

void BTreeNode::insertNonFull(int k) {
    // Inicializa i con el índice del elemento más a la derecha
    int i = n-1;

    // Si este es un nodo hoja
    if (leaf) {
        // El siguiente bucle hace dos cosas:
        // a) Encuentra la ubicación de la nueva clave a insertar
        // b) Mueve todas las claves mayores un lugar hacia adelante
        while (i >= 0 && keys[i] > k) {
            keys[i+1] = keys[i];
            i--;
        }

        // Inserta la nueva clave en la ubicación encontrada
        keys[i+1] = k;
        n = n+1;
    } else { // Si este nodo no es hoja
        // Encuentra el hijo que va a tener la nueva clave
        while (i >= 0 && keys[i] > k) i--;

        // Ver si el hijo encontrado está lleno
        if (children[i+1]->n == 2*t-1) {
            // Si el hijo está lleno, entonces divídelo
            splitChild(i+1, children[i+1]);

            // Después de dividir, la clave del medio de children[i] sube y children[i]
            // se divide en dos. Ver cuál de los dos va a tener la nueva clave
            if (keys[i+1] < k) i++;
        }
        children[i+1]->insertNonFull(k);
    }
}

void BTreeNode::splitChild(int i, BTreeNode* y) {
    // Crea un nuevo nodo que va a almacenar (t-1) claves de y
    BTreeNode* z = new BTreeNode(y->t, y->leaf);
    z->n = t - 1;

    // Copia las últimas (t-1) claves de y a z
    for (int j = 0; j < t-1; j++)
        z->keys[j] = y->keys[j+t];

    // Copia los últimos t hijos de y a z
    if (!y->leaf) {
        for (int j = 0; j < t; j++)
            z->children[j] = y->children[j+t];
    }

    // Reduce el número de claves en y
    y->n = t - 1;

    // Dado que este nodo va a tener un nuevo hijo,
    // crea espacio para el nuevo hijo
    for (int j = n; j >= i+1; j--)
        children[j+1] = children[j];

    // Enlaza el nuevo hijo a este nodo
    children[i+1] = z;

    // Una clave de y se moverá a este nodo. Mueve todas las claves mayores
    // un lugar hacia adelante
    for (int j = n-1; j >= i; j--)
        keys[j+1] = keys[j];

    // Copia la clave del medio de y a este nodo
    keys[i] = y->keys[t-1];

    // Incrementa el conteo de claves en este nodo
    n = n + 1;
}

void BTree::insert(int k) {
    // Si el árbol está vacío
    if (root == nullptr) {
        // Asigna memoria para la raíz
        root = new BTreeNode(t, true);
        root->keys[0] = k;  // Inserta la clave
        root->n = 1;  // Actualiza el número de claves en la raíz
    } else { // Si el árbol no está vacío
        // Si la raíz está llena, entonces el árbol crece en altura
        if (root->n == 2*t-1) {
            // Asigna memoria para la nueva raíz
            BTreeNode* s = new BTreeNode(t, false);

            // Hace la antigua raíz como hijo de la nueva raíz
            s->children[0] = root;

            // Divide la antigua raíz y mueve 1 clave a la nueva raíz
            s->splitChild(0, root);

            // La nueva raíz tiene dos hijos ahora. Decide cuál de los
            // dos hijos va a tener la nueva clave
            int i = 0;
            if (s->keys[0] < k) i++;
            s->children[i]->insertNonFull(k);

            // Cambia la raíz
            root = s;
        } else { // Si la raíz no está llena, llama a insertNonFull para la raíz
            root->insertNonFull(k);
        }
    }
}

// Programa de prueba
int main() {
    BTree t(15); // Un árbol B con grado mínimo 100
    const int tam = 33000000; // Tamaño del árbol
    auto start = std::chrono::high_resolution_clock::now(); // Comenzar a medir el tiempo

    for(int i = 0; i < tam; i++){
        int num = rand() % 90000000 + 10000000; // Generar un número aleatorio
        t.insert(num);
    }
    
    auto stop = std::chrono::high_resolution_clock::now(); // Detener la medición del tiempo

    auto duration = std::chrono::duration_cast<std::chrono::seconds>(stop - start); // Calcular la duración
    
    std::cout << "Tiempo de ejecución: " << duration.count() << " segundos" << std::endl;


    return 0;
}