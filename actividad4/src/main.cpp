#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <algorithm>
#include <fstream>
#include <string>
#include <iomanip>

using namespace std; 

const double PI = 3.141592653589793;

// Estructura para representar cada fuente de alimento (las flores con néctar)
struct Fuente {
    vector<double> coords; // Xi: Posicion en el mapa (x, y)
    double res;        // Valor real de la funcion Eggholder, es el costo que queremos minimizar
    double fitness;          // Calidad del nectar (transformado para que mas sea mejor)
    int intentos;          // To: Contador para saber si la fuente se agoto (estancamiento)
    double p;              // Probabilidad de que una observadora elija esta fuente
};



class BeeColony {
private:
    int dimensiones = 2; // Problema 2D (x, y)
    double lim_inf, lim_sup;
    int pop, n_obreras, n_observadoras;
    int limite_L;    // L: Numero maximo de fallos permitidos antes de abandonar una fuente
    int iteraciones; // Generaciones totales del algoritmo
    
    vector<Fuente> fuentes; // Conjunto de soluciones actuales (fuentes de néctar)
    mt19937 motor;          // Generador de numeros aleatorios Mersenne Twister
    Fuente best;            // La mejor fuente encontrada en toda la ejecucion

public:
    BeeColony(int n_abejas, int L, double min, double max, int gens) {
        pop = n_abejas;
        limite_L = L;
        lim_inf = min;
        lim_sup = max;
        iteraciones = gens;
        
        // La colonia se divide: la mitad explota y la otra mitad observa el baile
        n_obreras = pop / 2;
        n_observadoras = pop / 2;

        random_device rd;
        motor = mt19937(rd());
        best.res = 1e9; // Valor inicial muy alto para buscar el minimo
    }

    // Funcion Eggholder: El paisaje donde las abejas buscan el punto mas bajo
    double get_eggholder(double x, double y) {
        double t1 = -(y + 47.0) * sin(sqrt(abs(x / 2.0 + (y + 47.0))));
        double t2 = -x * sin(sqrt(abs(x - (y + 47.0))));
        return t1 + t2;
    }

    // El fitness (néctar) debe ser positivo. Esta formula asegura que 
    // a menor costo (valor mas negativo), mayor sea el fitness para la ruleta.
    double calc_fitness(double f) {
        return (f >= 0) ? (1.0 / (1.0 + f)) : (1.0 + abs(f));
    }

    // Inicializacion: Las abejas salen por primera vez a buscar comida al azar
    void init() {
        fuentes.clear();
        uniform_real_distribution<double> dist(lim_inf, lim_sup);

        for (int i = 0; i < n_obreras; i++) {
            Fuente f;
            f.coords = {dist(motor), dist(motor)}; // Ubicacion aleatoria [-512, 512]
            f.res = get_eggholder(f.coords[0], f.coords[1]);
            f.fitness = calc_fitness(f.res);
            f.intentos = 0; // Fuente nueva, tiene 0 fallos acumulados
            fuentes.push_back(f);
            
            if (f.res < best.res) best = f;
        }
    }

    // Fase de obreras: Busqueda local. Cada abeja intenta mejorar su propia fuente.
    void faseObreras() {
        uniform_int_distribution<int> d_vecino(0, n_obreras - 1);
        uniform_int_distribution<int> d_dim(0, dimensiones - 1);
        uniform_real_distribution<double> d_phi(-1.0, 1.0); 

        for (int i = 0; i < n_obreras; i++) {
            // Se elige una abeja vecina k al azar para comparar (k != i)
            int k = d_vecino(motor);
            while (k == i) k = d_vecino(motor);

            int j = d_dim(motor); // Se altera solo una dimension (x o y)
            Fuente nueva = fuentes[i];
            double phi = d_phi(motor); // El factor de mutacion (S)
            
            // Formula Zi = Xi + phi * (Xi - Yk): Se mueve un poco hacia o lejos del vecino
            nueva.coords[j] = fuentes[i].coords[j] + phi * (fuentes[i].coords[j] - fuentes[k].coords[j]);
            
            // Verificacion de bordes
            if (nueva.coords[j] < lim_inf) nueva.coords[j] = lim_inf;
            if (nueva.coords[j] > lim_sup) nueva.coords[j] = lim_sup;

            nueva.res = get_eggholder(nueva.coords[0], nueva.coords[1]);
            nueva.fitness = calc_fitness(nueva.res);

            // Seleccion codiciosa: solo nos quedamos con el cambio si es mejor
            if (nueva.fitness > fuentes[i].fitness) {
                fuentes[i] = nueva;
                fuentes[i].intentos = 0; // Exito: reiniciamos contador de estancamiento
            } else {
                fuentes[i].intentos++; // Fallo: la fuente se acerca a ser abandonada
            }
        }
    }

    // Fase de observadoras: Eligen las mejores fuentes mediante una ruleta de probabilidad
    void faseObservadoras() {
        double total_fitness = 0;
        for (int i = 0; i < n_obreras; i++) total_fitness += fuentes[i].fitness;
        // Calculo de probabilidad: a mas nectar, mas abejas atrae la fuente
        for (int i = 0; i < n_obreras; i++) fuentes[i].p = fuentes[i].fitness / total_fitness;

        uniform_real_distribution<double> d_rand(0, 1); // Probabilidad para la ruleta
        uniform_int_distribution<int> d_vecino(0, n_obreras - 1);
        uniform_int_distribution<int> d_dim(0, dimensiones - 1);
        uniform_real_distribution<double> d_phi(-1.0, 1.0);

        int count = 0;
        // Las observadoras se asignan a las fuentes mas prometedoras
        while (count < n_observadoras) {
            for (int i = 0; i < n_obreras && count < n_observadoras; i++) {
                // Metodo de ruleta
                if (d_rand(motor) < fuentes[i].p) {
                    count++;
                    
                    int k = d_vecino(motor);
                    while (k == i) k = d_vecino(motor);

                    int j = d_dim(motor);
                    Fuente nueva = fuentes[i];
                    double phi = d_phi(motor); //S en la lección
                    
                    // Busqueda local intensiva en las mejores zonas
                    nueva.coords[j] = fuentes[i].coords[j] + phi * (fuentes[i].coords[j] - fuentes[k].coords[j]);
                    
                    if (nueva.coords[j] < lim_inf) nueva.coords[j] = lim_inf;
                    if (nueva.coords[j] > lim_sup) nueva.coords[j] = lim_sup;

                    nueva.res = get_eggholder(nueva.coords[0], nueva.coords[1]);
                    nueva.fitness = calc_fitness(nueva.res);

                    if (nueva.fitness > fuentes[i].fitness) {
                        fuentes[i] = nueva;
                        fuentes[i].intentos = 0; 
                    } else {
                        fuentes[i].intentos++; 
                    }
                }
            }
        }
    }

    // Fase de exploradoras: Si una fuente no mejora tras L intentos, se abandona
    void faseExploradoras() {
        uniform_real_distribution<double> dist(lim_inf, lim_sup);
        for (int i = 0; i < n_obreras; i++) {
            if (fuentes[i].intentos > limite_L) {
                // La abeja olvida la fuente estancada y busca una nueva al azar (Exploracion Global)
                fuentes[i].coords = {dist(motor), dist(motor)};
                fuentes[i].res = get_eggholder(fuentes[i].coords[0], fuentes[i].coords[1]);
                fuentes[i].fitness = calc_fitness(fuentes[i].res);
                fuentes[i].intentos = 0; 
            }
        }
    }

    // Actualiza el record del mejor valor encontrado hasta el momento
    void check_best() {
        for(int i = 0; i < n_obreras; i++) {
            if (fuentes[i].res < best.res) best = fuentes[i];
        }
    }

    void run() {
        init(); // Generacion 0
        
        // Exportacion de datos para graficar en Python
        ofstream f1("../results/csv/fit_Eggholder_ABC.csv");
        ofstream f2("../results/csv/pob_Eggholder_ABC.csv");
        
        f1 << "Gen,MejorCosto" << endl;
        f2 << "Gen,X,Y,Costo" << endl;

        for (int g = 0; g < iteraciones; g++) {
            // Guardamos muestras de la poblacion (inicial, media, final)
            if (g == 0 || g == iteraciones / 2 || g == iteraciones - 1) {
                for (int i = 0; i < n_obreras; i++) {
                    f2 << g << "," << fuentes[i].coords[0] << "," << fuentes[i].coords[1] << "," << fuentes[i].res << endl;
                }
            }

            faseObreras();      // Explotacion individual
            faseObservadoras(); // Explotacion colectiva/selectiva
            check_best();       // Guardar record
            faseExploradoras(); // Salir de minimos locales (Exploracion)

            f1 << g << "," << best.res << endl;
        }
        
        f1.close(); f2.close();
        
        cout << "Optimo Eggholder encontrado: " << fixed << setprecision(6) << best.res << endl;
        cout << "Coordenadas del hallazgo: (" << best.coords[0] << ", " << best.coords[1] << ")" << endl;
    }
};

int main() {
    // Preparacion de carpetas
    system("mkdir -p ../results/csv ../results/plots");

    cout << "--- Algoritmo de Colonia de Abejas Artificiales ---" << endl;

    // Configuracion: 100 abejas, Limite L=100, Rango [-512, 512], 500 iteraciones
    BeeColony colmena(100, 100, -512.0, 512.0, 500);
    colmena.run();
    
    // Ejecucion automatica del script de visualizacion
    cout << "Generando visualizaciones..." << endl;
    system("cd ../results/csv && python3 ../../src/graficar_abc.py");
    
    return 0;
}