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

// --- Funciones Objetivo ---

// 1. Michalewicz 2D (m=10 implícito en la potencia 20)
double michalewicz(const vector<double>& x) {
    double res = 0.0;
    for (int i = 0; i < 2; i++) {
        res -= sin(x[i]) * pow(sin((i + 1) * x[i] * x[i] / PI), 20);
    }
    return res;
}

// 2. Six-Hump Camel Function
double six_hump_camel(const vector<double>& x) {
    double x1 = x[0];
    double x2 = x[1];
    double term1 = (4.0 - 2.1 * x1 * x1 + pow(x1, 4.0) / 3.0) * x1 * x1;
    double term2 = x1 * x2;
    double term3 = (-4.0 + 4.0 * x2 * x2) * x2 * x2;
    return term1 + term2 + term3;
}

// --- Clase Differential Evolution (DE) ---
class DE {
private:
    int n_part, dims, iters;
    double F, CR;
    vector<double> lim_inf, lim_sup;
    
    vector<vector<double>> poblacion;
    vector<double> fitness_pob;
    mt19937 motor;
    
    vector<double> gbest_pos;
    double gbest_val;
    
    double (*func_obj)(const vector<double>&);
    string nombre_func;

public:
    DE(int np, int dimensiones, int iteraciones, vector<double> inf, vector<double> sup, double (*f)(const vector<double>&), string nombre) {
        n_part = np; 
        dims = dimensiones; 
        iters = iteraciones;
        lim_inf = inf; 
        lim_sup = sup; 
        func_obj = f; 
        nombre_func = nombre;
        
        F = 0.8;  // Factor de Mutación [0, 2]
        CR = 0.9; // Tasa de Cruza [0, 1]
        
        gbest_val = numeric_limits<double>::max();
        random_device rd; 
        motor = mt19937(rd());
    }

    void inicializar() {
        poblacion.resize(n_part, vector<double>(dims));
        fitness_pob.resize(n_part);
        
        for (int i = 0; i < n_part; i++) {
            for (int d = 0; d < dims; d++) {
                uniform_real_distribution<double> dist_pos(lim_inf[d], lim_sup[d]);
                poblacion[i][d] = dist_pos(motor);
            }
            fitness_pob[i] = func_obj(poblacion[i]);
            
            if (fitness_pob[i] < gbest_val) {
                gbest_val = fitness_pob[i];
                gbest_pos = poblacion[i];
            }
        }
    }

    void evolucionar(int gen) {
        vector<vector<double>> nueva_poblacion = poblacion;
        vector<double> nueva_fitness = fitness_pob;
        uniform_real_distribution<double> dist_r(0.0, 1.0);
        uniform_int_distribution<int> dist_int(0, n_part - 1);
        uniform_int_distribution<int> dist_dim(0, dims - 1);

        for (int i = 0; i < n_part; i++) {
            // 1. Elegir r1, r2, r3 distintos entre si y distintos de i
            int r1, r2, r3;
            do { r1 = dist_int(motor); } while (r1 == i);
            do { r2 = dist_int(motor); } while (r2 == i || r2 == r1);
            do { r3 = dist_int(motor); } while (r3 == i || r3 == r1 || r3 == r2);

            // 2. Mutación y Cruza
            vector<double> u_prueba = poblacion[i];
            int j_rand = dist_dim(motor); // Asegura que herede al menos una dimensión

            for (int d = 0; d < dims; d++) {
                if (dist_r(motor) <= CR || d == j_rand) {
                    double v_mutante = poblacion[r1][d] + F * (poblacion[r2][d] - poblacion[r3][d]);
                    // Clamping a los limites
                    v_mutante = max(lim_inf[d], min(lim_sup[d], v_mutante));
                    u_prueba[d] = v_mutante;
                }
            }

            // 3. Selección
            double fit_prueba = func_obj(u_prueba);
            if (fit_prueba <= fitness_pob[i]) {
                nueva_poblacion[i] = u_prueba;
                nueva_fitness[i] = fit_prueba;
                
                // Actualizar Global Best
                if (fit_prueba < gbest_val) {
                    gbest_val = fit_prueba;
                    gbest_pos = u_prueba;
                }
            }
        }
        
        poblacion = nueva_poblacion;
        fitness_pob = nueva_fitness;
    }

    void ejecutar() {
        string ffit = "../results/csv/fit_" + nombre_func + "_DE.csv";
        string fpob = "../results/csv/pob_" + nombre_func + "_DE.csv";

        ofstream f_fit(ffit); ofstream f_pob(fpob);
        f_fit << "Gen,MejorFitness\n"; f_pob << "Gen,X,Y,Fitness\n";

        inicializar();
        
        for (int i = 0; i < n_part; i++) f_pob << "0," << poblacion[i][0] << "," << poblacion[i][1] << "," << fitness_pob[i] << "\n";
        f_fit << "0," << fixed << setprecision(8) << gbest_val << "\n";

        for (int g = 1; g <= iters; g++) {
            evolucionar(g);
            f_fit << g << "," << fixed << setprecision(8) << gbest_val << "\n";
            
            if (g == iters / 2 || g == iters) {
                for (int i = 0; i < n_part; i++) {
                    f_pob << g << "," << poblacion[i][0] << "," << poblacion[i][1] << "," << fitness_pob[i] << "\n";
                }
            }
        }
        f_fit.close(); f_pob.close();
        cout << "[" << nombre_func << "] Mejor: " << fixed << setprecision(8) << gbest_val << endl;
    }
};

int main() {
    system("mkdir -p ../results/csv ../results/plots");
    cout << "=== Ejecutando Evolucion Diferencial (DE) ===" << endl;
    
    // 1. Michalewicz 
    vector<double> min_mich = {0.0, 0.0};
    vector<double> max_mich = {PI, PI};
    DE de_micha(50, 2, 200, min_mich, max_mich, michalewicz, "Michalewicz");
    de_micha.ejecutar();

    // 2. Six-Hump Camel
    vector<double> min_camel = {-3.0, -2.0};
    vector<double> max_camel = {3.0, 2.0};
    DE de_camel(50, 2, 200, min_camel, max_camel, six_hump_camel, "SixHumpCamel");
    de_camel.ejecutar();

    cout << "Generando graficas en ../results/plots/ ..." << endl;
    system("python3 graficar_de.py");
    cout << "¡Proceso terminado exitosamente!" << endl;
    
    return 0;
}