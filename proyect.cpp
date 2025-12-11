#include <iostream>
#include <fstream>
#include <string>
#include <cmath>

using namespace std;

const int ANCHO_MAX = 256;
const int ALTO_MAX = 256;
const int MAX_CIRCULOS = 32;

struct Circulo {
    int color;
    int min_x, max_x;
    int min_y, max_y;
    int centro_x, centro_y;
    int diametro;
    int radio;
    int perimetro;
    double pi_local;
    int area_normalizada;
};

int N, M, T;
int ancho, alto, max_gris;
int imagen[ALTO_MAX][ANCHO_MAX];
bool visitado[ALTO_MAX][ANCHO_MAX];
Circulo lista[MAX_CIRCULOS];
int total_encontrados = 0;

bool mejor_seleccion[MAX_CIRCULOS];
int suma_mejor = 0;
int menor_diferencia = 9999999;

void cargarImagen(const string &nombreArchivo) {
    ifstream in(nombreArchivo.c_str());
    string cabecera;
    in >> cabecera;
    in >> ancho >> alto >> max_gris;
    for (int r = 0; r < alto; ++r) {
        for (int c = 0; c < ancho; ++c) {
            in >> imagen[r][c];
        }
    }
    in.close();
}

void verVecinos(int x, int y, int colorBuscado, int idx) {
    if (x < 0 || x >= ancho || y < 0 || y >= alto) return;
    if (visitado[y][x]) return;
    if (imagen[y][x] != colorBuscado) return;

    visitado[y][x] = true;

    if (x < lista[idx].min_x) lista[idx].min_x = x;
    if (x > lista[idx].max_x) lista[idx].max_x = x;
    if (y < lista[idx].min_y) lista[idx].min_y = y;
    if (y > lista[idx].max_y) lista[idx].max_y = y;

    verVecinos(x + 1, y, colorBuscado, idx);
    verVecinos(x - 1, y, colorBuscado, idx);
    verVecinos(x, y + 1, colorBuscado, idx);
    verVecinos(x, y - 1, colorBuscado, idx);
}

void procesarImagen() {
    for (int r = 0; r < alto; ++r)
        for (int c = 0; c < ancho; ++c)
            visitado[r][c] = false;

    total_encontrados = 0;
    int fondo = 0;

    for (int y = 0; y < alto; ++y) {
        for (int x = 0; x < ancho; ++x) {
            int col = imagen[y][x];
            if (col != fondo && !visitado[y][x]) {
                if (total_encontrados >= MAX_CIRCULOS) break;

                lista[total_encontrados].color = col;
                lista[total_encontrados].min_x = ANCHO_MAX;
                lista[total_encontrados].max_x = -1;
                lista[total_encontrados].min_y = ALTO_MAX;
                lista[total_encontrados].max_y = -1;

                verVecinos(x, y, col, total_encontrados);
                total_encontrados++;
            }
        }
    }
}

void calcularMedidas() {
    double sumaPi = 0.0;
    for (int i = 0; i < total_encontrados; ++i) {
        Circulo &c = lista[i];
        c.centro_x = (c.min_x + c.max_x) / 2;
        c.centro_y = (c.min_y + c.max_y) / 2;

        int w = c.max_x - c.min_x + 1;
        int h = c.max_y - c.min_y + 1;
        c.diametro = (w > h) ? w : h;
        c.radio = (c.diametro / 2) + 1;

        c.perimetro = 0;
        for (int y = c.min_y; y <= c.max_y; ++y) {
            for (int x = c.min_x; x <= c.max_x; ++x) {
                if (imagen[y][x] == c.color) {
                    bool borde = false;
                    if (x == 0 || x == ancho - 1 || y == 0 || y == alto - 1) borde = true;
                    else if (imagen[y][x - 1] != c.color) borde = true;
                    else if (imagen[y][x + 1] != c.color) borde = true;
                    else if (imagen[y - 1][x] != c.color) borde = true;
                    else if (imagen[y + 1][x] != c.color) borde = true;

                    if (borde) c.perimetro++;
                }
            }
        }

        if (c.diametro > 0) c.pi_local = (double)c.perimetro / (double)c.diametro;
        else c.pi_local = 0.0;

        sumaPi += c.pi_local;
    }

    double pi_promedio = 0.0;
    if (total_encontrados > 0) pi_promedio = sumaPi / total_encontrados;

    for (int i = 0; i < total_encontrados; ++i) {
        double areaExacta = pi_promedio * (double)(lista[i].radio * lista[i].radio);
        lista[i].area_normalizada = (int)ceil(areaExacta);
    }
}

void ordenarPorColor() {
    for (int i = 0; i < total_encontrados - 1; ++i) {
        for (int j = 0; j < total_encontrados - i - 1; ++j) {
            if (lista[j].color > lista[j + 1].color) {
                Circulo tmp = lista[j];
                lista[j] = lista[j + 1];
                lista[j + 1] = tmp;
            }
        }
    }
}

void buscarMejorSuma(int idx, int sumaActual, bool seleccion[]) {
    if (sumaActual > T) return;

    int diferencia = T - sumaActual;
    if (diferencia < menor_diferencia) {
        menor_diferencia = diferencia;
        suma_mejor = sumaActual;
        for (int k = 0; k < M; ++k) mejor_seleccion[k] = seleccion[k];
    }

    if (idx == M) return;

    seleccion[idx] = true;
    buscarMejorSuma(idx + 1, sumaActual + lista[idx].area_normalizada, seleccion);
    seleccion[idx] = false;
    buscarMejorSuma(idx + 1, sumaActual, seleccion);
}

void escribirSalida(const string &nombre) {
    ofstream out(nombre.c_str());
    if (!out.is_open()) return;
    out << "P2" << endl;
    out << ancho << " " << alto << endl;
    out << max_gris << endl;

    for (int y = 0; y < alto; ++y) {
        for (int x = 0; x < ancho; ++x) {
            int valor = 0;
            for (int k = 0; k < total_encontrados; ++k) {
                Circulo &c = lista[k];
                if (x == c.centro_x && y == c.centro_y) { valor = max_gris; break; }
                if (x >= c.min_x && x <= c.max_x && y >= c.min_y && y <= c.max_y) {
                    if (imagen[y][x] == c.color) {
                        bool borde = false;
                        if (x == 0 || x == ancho - 1 || y == 0 || y == alto - 1) borde = true;
                        else if (imagen[y][x - 1] != c.color) borde = true;
                        else if (imagen[y][x + 1] != c.color) borde = true;
                        else if (imagen[y - 1][x] != c.color) borde = true;
                        else if (imagen[y + 1][x] != c.color) borde = true;

                        if (borde) valor = c.color;
                    }
                }
            }
            out << valor << (x == ancho - 1 ? "" : " ");
        }
        out << endl;
    }
    out.close();
}

int main() {
    cin >> N;
    cin >> M;
    cin >> T;

    string archivo = "objectives" + to_string(N) + ".pgm";
    cargarImagen(archivo);

    procesarImagen();
    if (total_encontrados != M) M = total_encontrados;

    calcularMedidas();
    ordenarPorColor();

    for (int i = 0; i < M; ++i) {
        cout << lista[i].area_normalizada << "u" << endl;
    }
    cout << endl;

    bool selTemp[MAX_CIRCULOS] = { false };
    buscarMejorSuma(0, 0, selTemp);

    bool primero = true;
    for (int i = 0; i < M; ++i) {
        if (mejor_seleccion[i]) {
            if (!primero) cout << ", ";
            cout << "Circulo" << (i + 1);
            primero = false;
        }
    }
    cout << endl;

    escribirSalida("sites.pgm");
    return 0;
}