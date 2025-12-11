#include <iostream>
#include <fstream>
#include <string>
#include <math.h>   

using namespace std;

const int MAX_ANCHO = 256;
const int MAX_ALTO = 256;
const int MAX_CIRCULOS = 32;

struct Circulo {
    int color;

    int minX, maxX;
    int minY, maxY;


    int centroX, centroY;
    int diametro;
    int radio;
    int circunferencia; 

    double pi_local;
    int area_normalizada;
};

int N, M, T;
int ancho, alto, max_gris;
int imagen[MAX_ALTO][MAX_ANCHO];
bool visitado[MAX_ALTO][MAX_ANCHO];       
Circulo lista[MAX_CIRCULOS];
int total_encontrados = 0;   

bool mejorSeleccion[MAX_CIRCULOS];
int sumaMaxima = 0;
int menorDiferencia = 9999999;



void cargarImagen(string nombreArchivo) {
    ifstream archivo(nombreArchivo.c_str());

    string cabecera;
    archivo >> cabecera; 
    archivo >> ancho >> alto;
    archivo >> max_gris;

    for (int i = 0; i < alto; i++) {
        for (int j = 0; j < ancho; j++) {
            archivo >> imagen[i][j];
        }
    }
    archivo.close();
}

void Ver_vecinos(int x, int y, int colorBuscado, int i) {
    
    if (x < 0 || x >= ancho || y < 0 || y >= alto) return;

    if (visitado[y][x]) return;
    if (imagen[y][x] != colorBuscado) return;

    visitado[y][x] = true;

    if (x < lista[i].minX) lista[i].minX = x;
    if (x > lista[i].maxX) lista[i].maxX = x;
    if (y < lista[i].minY) lista[i].minY = y;
    if (y > lista[i].maxY) lista[i].maxY = y;

    Ver_vecinos(x + 1, y, colorBuscado, i); 
    Ver_vecinos(x - 1, y, colorBuscado, i); 
    Ver_vecinos(x, y + 1, colorBuscado, i); 
    Ver_vecinos(x, y - 1, colorBuscado, i); 
}

void procesarImagen() {

    for (int i = 0; i < alto; i++)
        for (int j = 0; j < ancho; j++)
            visitado[i][j] = false;

    total_encontrados = 0;
    int fondo = 0; 

    for (int y = 0; y < alto; y++) {
        for (int x = 0; x < ancho; x++) {
            int colorActual = imagen[y][x];

            if (colorActual != fondo && !visitado[y][x]) {

                if (total_encontrados >= MAX_CIRCULOS) break;

                lista[total_encontrados].color = colorActual;
                lista[total_encontrados].minX = MAX_ANCHO;
                lista[total_encontrados].maxX = -1;
                lista[total_encontrados].minY = MAX_ALTO;
                lista[total_encontrados].maxY = -1;

                Ver_vecinos(x, y, colorActual, total_encontrados);

                total_encontrados++;
            }
        }
    }
}



void calcularDatos() {
    double sumaTotalPi = 0.0;

    for (int i = 0; i < total_encontrados; i++) {
        Circulo& circuloActual = lista[i];

        circuloActual.centroX = (circuloActual.minX + circuloActual.maxX) / 2;
        circuloActual.centroY = (circuloActual.minY + circuloActual.maxY) / 2;

        int w = circuloActual.maxX - circuloActual.minX + 1;
        int h = circuloActual.maxY - circuloActual.minY + 1;

        if(w > h){
            circuloActual.diametro = w;
        } else {
            circuloActual.diametro = h;
        }

        circuloActual.radio = (circuloActual.diametro / 2) + 1;

        circuloActual.circunferencia = 0;
        for (int y = circuloActual.minY; y <= circuloActual.maxY; y++) {
            for (int x = circuloActual.minX; x <= circuloActual.maxX; x++) {
                if (imagen[y][x] == circuloActual.color) {
                    bool esBorde = false;

                    if (x == 0 || x == ancho - 1 || y == 0 || y == alto - 1) esBorde = true;
                    else if (imagen[y][x - 1] != circuloActual.color) esBorde = true;
                    else if (imagen[y][x + 1] != circuloActual.color) esBorde = true;
                    else if (imagen[y - 1][x] != circuloActual.color) esBorde = true;
                    else if (imagen[y + 1][x] != circuloActual.color) esBorde = true;

                    if (esBorde) circuloActual.circunferencia++;
                }
            }
        }

        if (circuloActual.diametro > 0)
            circuloActual.pi_local = (double)circuloActual.circunferencia / (double)circuloActual.diametro;
        else
            circuloActual.pi_local = 0;

        sumaTotalPi = sumaTotalPi+ circuloActual.pi_local;
    }

    double pi_promedio = 0;
    if (total_encontrados > 0)
        pi_promedio = sumaTotalPi / total_encontrados;

    for (int i = 0; i < total_encontrados; i++) {
        double areaExacta = pi_promedio * (double)(lista[i].radio * lista[i].radio);
        lista[i].area_normalizada = (int)ceil(areaExacta); 
    }
}

void ordenarPorColor() {
   
    for (int i = 0; i < total_encontrados - 1; i++) {
        for (int j = 0; j < total_encontrados - i - 1; j++) {
            if (lista[j].color > lista[j + 1].color) {
                Circulo temp = lista[j];
                lista[j] = lista[j + 1];
                lista[j + 1] = temp;
            }
        }
    }
}



void buscarMejorSuma(int i, int sumaActual, bool seleccionActual[]) {

    if (sumaActual > T) return;

    
    int diferencia = T - sumaActual;
    if (diferencia < menorDiferencia) {
        menorDiferencia = diferencia;
        sumaMaxima = sumaActual;

        for (int k = 0; k < M; k++) {
            mejorSeleccion[k] = seleccionActual[k];
        }
    }

    if (i == M) return;

    seleccionActual[i] = true;
    buscarMejorSuma(i + 1, sumaActual + lista[i].area_normalizada, seleccionActual);

    seleccionActual[i] = false;
    buscarMejorSuma(i + 1, sumaActual, seleccionActual);
}

void ArchivoSalida(string nombre) {
    ofstream archivo(nombre.c_str());
    if (!archivo.is_open()) return;

    archivo << "P2" << endl;
    archivo << ancho << " " << alto << endl;
    archivo << max_gris << endl;

    for (int y = 0; y < alto; y++) {
        for (int x = 0; x < ancho; x++) {
            int valorPixel = 0; 

            for (int k = 0; k < total_encontrados; k++) {
                Circulo& c = lista[k];

                if (x == c.centroX && y == c.centroY) {
                    valorPixel = max_gris;
                    break;
                }

                if (x >= c.minX && x <= c.maxX && y >= c.minY && y <= c.maxY) {
                    if (imagen[y][x] == c.color) {
  
                        bool esBorde = false;
                        if (x == 0 || x == ancho - 1 || y == 0 || y == alto - 1) esBorde = true;
                        else if (imagen[y][x - 1] != c.color) esBorde = true;
                        else if (imagen[y][x + 1] != c.color) esBorde = true;
                        else if (imagen[y - 1][x] != c.color) esBorde = true;
                        else if (imagen[y + 1][x] != c.color) esBorde = true;

                        if (esBorde) {
                            valorPixel = c.color;
                        }
                    }
                }
            }
            archivo << valorPixel << (x == ancho - 1 ? "" : " ");
        }
        archivo << endl;
    }
    archivo.close();
}

int main() {
    
    cin>>N; 
    cin>>M; 
    cin>>T;

    string nombreArchivo = "objectives" + to_string(N) + ".pgm";
    cargarImagen(nombreArchivo);

    procesarImagen();
    
    if (total_encontrados != M) {
        M = total_encontrados;
    }

    calcularDatos();
    ordenarPorColor();

    for (int i = 0; i < M; i++) {
        cout << lista[i].area_normalizada << "u" << endl;
    }
    cout << endl;

    bool seleccionTemp[MAX_CIRCULOS] = { false };
    buscarMejorSuma(0, 0, seleccionTemp);

    bool primero = true;
    for (int i = 0; i < M; i++) {
        if (mejorSeleccion[i]) {
            if (!primero) cout << ", ";
            cout << "Circulo" << (i + 1);
            primero = false;
        }
    }
    cout << endl;

    ArchivoSalida("sites.pgm");

    return 0;
}