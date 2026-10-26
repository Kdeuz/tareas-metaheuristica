#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <algorithm>
#include <fstream>
#include <string>
#include <iomanip>
#include <limits>

using namespace std;

const double PI = 3.141592653589793;

struct Particula {
    vector<double> pos;
    vector<double> vel;
    vector<double> pbest_pos;
    double pbest_val;
    double fitness;
};

// --- Funciones Objetivo ---
double rosenbrock(const vector<double>& x) {
    double res = 0.0;
    for (int i = 0; i < (int)x.size() - 1; i++) {
        double a = x[i + 1] - x[i] * x[i];
        double b = 1.0 - x[i];
        res += 100.0 * a * a + b * b;
    }
    return res;
}

double michalewicz(const vector<double>& x) {
    const int m = 10; 
    double res = 0.0;
    for (int i = 0; i < (int)x.size(); i++) {
        double sin_xi = sin(x[i]);
        double arg = (i + 1) * x[i] * x[i] / PI;
        res -= sin_xi * pow(sin(arg), 2 * m);
    }
    return res;
}

class PSO {
private:
    int n_part, dims, iters, k_vecinos;
    double w, w_max, w_min, c1, c2, v_max, lim_inf, lim_sup;
    vector<Particula> enjambre;
    mt19937 motor;
    vector<double> gbest_pos;
    double gbest_val;
    double (*func_obj)(const vector<double>&);
    string nombre_func;

public:
    PSO(int particulas, int dimensiones, int iteraciones, double inf, double sup, double (*f)(const vector<double>&), string nombre, int k = 3) {
        n_part = particulas; dims = dimensiones; iters = iteraciones;
        lim_inf = inf; lim_sup = sup; func_obj = f; nombre_func = nombre; k_vecinos = k;
        w_max = 0.9; w_min = 0.4; c1 = 2.05; c2 = 2.05;
        v_max = 0.20 * (lim_sup - lim_inf);
        gbest_val = numeric_limits<double>::max();
        random_device rd; motor = mt19937(rd());
    }

    void inicializar() {
        uniform_real_distribution<double> dist_pos(lim_inf, lim_sup);
        uniform_real_distribution<double> dist_vel(-v_max, v_max);
        enjambre.resize(n_part);
        for (int i = 0; i < n_part; i++) {
            enjambre[i].pos.resize(dims); enjambre[i].vel.resize(dims); enjambre[i].pbest_pos.resize(dims);
            for (int d = 0; d < dims; d++) {
                enjambre[i].pos[d] = dist_pos(motor);
                enjambre[i].vel[d] = dist_vel(motor);
            }
            enjambre[i].fitness = func_obj(enjambre[i].pos);
            enjambre[i].pbest_pos = enjambre[i].pos;
            enjambre[i].pbest_val = enjambre[i].fitness;
            if (enjambre[i].fitness < gbest_val) {
                gbest_val = enjambre[i].fitness; gbest_pos = enjambre[i].pos;
            }
        }
    }

    // ---------------------------------------------------------------------
    // Vecindario Dinámico basado en Fitness
    // ---------------------------------------------------------------------
    // --- PASO: Vecindario Dinámico basado en Fitness ---
    // Esta función permite que las partículas se comuniquen con individuos 
    // que tengan un desempeño similar, mejorando la diversidad.
    int mejor_vecino_dinamico(int i) {
        // Usamos un vector de pares para guardar la [Distancia en Fitness, Indice]
        vector<pair<double, int>> distancias;
        
        for (int j = 0; j < n_part; j++) {
            if (i == j) continue; // No compararse consigo misma
            // Calcular qué tan similar es el fitness de la partícula j al de la i
            double dist_fitness = abs(enjambre[i].fitness - enjambre[j].fitness);
            distancias.push_back({dist_fitness, j});
        }
        
        // Ordenar de menor a mayor distancia (los primeros son los más parecidos)
        sort(distancias.begin(), distancias.end());

        int mejor = i;
        double val = enjambre[i].pbest_val;

        // Evaluar solo a los 'k' vecinos dinámicos más parecidos en fitness
        for (int v = 0; v < k_vecinos; v++) {
            int j = distancias[v].second;
            if (enjambre[j].pbest_val < val) { 
                val = enjambre[j].pbest_val; 
                mejor = j; 
            }
        }
        return mejor;
    }
    // ---------------------------------------------------------------------

    void actualizar_enjambre(int gen) {
        w = w_max - (w_max - w_min) * (double)gen / (double)iters;
        uniform_real_distribution<double> dist_r(0.0, 1.0);
        for (int i = 0; i < n_part; i++) {
            
            // Llamamos a la nueva función dinámica
            int idx_sbest = mejor_vecino_dinamico(i);
            vector<double>& sbest = enjambre[idx_sbest].pbest_pos;
            
            for (int d = 0; d < dims; d++) {
                double phi_c = c1 * dist_r(motor), phi_s = c2 * dist_r(motor);
                enjambre[i].vel[d] = w * enjambre[i].vel[d] + phi_c * (enjambre[i].pbest_pos[d] - enjambre[i].pos[d]) + phi_s * (sbest[d] - enjambre[i].pos[d]);
                enjambre[i].vel[d] = max(-v_max, min(v_max, enjambre[i].vel[d]));
                enjambre[i].pos[d] += enjambre[i].vel[d];
                if (enjambre[i].pos[d] < lim_inf) { enjambre[i].pos[d] = lim_inf; enjambre[i].vel[d] *= -0.5; }
                if (enjambre[i].pos[d] > lim_sup) { enjambre[i].pos[d] = lim_sup; enjambre[i].vel[d] *= -0.5; }
            }
            enjambre[i].fitness = func_obj(enjambre[i].pos);
            if (enjambre[i].fitness < enjambre[i].pbest_val) { enjambre[i].pbest_val = enjambre[i].fitness; enjambre[i].pbest_pos = enjambre[i].pos; }
            if (enjambre[i].fitness < gbest_val) { gbest_val = enjambre[i].fitness; gbest_pos = enjambre[i].pos; }
        }
    }

    void ejecutar() {
        string ffit = "../results/csv/fit_" + nombre_func + "_PSO.csv";
        string fpob = "../results/csv/pob_" + nombre_func + "_PSO.csv";

        ofstream f_fit(ffit); ofstream f_pob(fpob);
        f_fit << "Gen,MejorFitness\n"; f_pob << "Gen,X,Y,Fitness\n";

        inicializar();
        for (int i = 0; i < n_part; i++) f_pob << "0," << enjambre[i].pos[0] << "," << enjambre[i].pos[1] << "," << enjambre[i].fitness << "\n";
        f_fit << "0," << gbest_val << "\n";

        for (int g = 1; g <= iters; g++) {
            actualizar_enjambre(g);
            f_fit << g << "," << gbest_val << "\n";
            if (g == iters / 2 || g == iters) {
                for (int i = 0; i < n_part; i++) f_pob << g << "," << enjambre[i].pos[0] << "," << enjambre[i].pos[1] << "," << enjambre[i].fitness << "\n";
            }
        }
        f_fit.close(); f_pob.close();
        cout << "[" << nombre_func << "] Mejor: " << fixed << setprecision(8) << gbest_val << endl;
    }
};

int main() {
    system("mkdir -p ../results/csv ../results/plots");

    cout << "--- Ejecutando PSO (Vecindario Dinamico basado en Fitness) ---" << endl;
    
    PSO pso_rosen(50, 2, 500, -5.0, 10.0, rosenbrock, "Rosenbrock", 4);
    pso_rosen.ejecutar();

    PSO pso_micha(50, 2, 500, 0.0, PI, michalewicz, "Michalewicz", 4);
    pso_micha.ejecutar();

    cout << "Generando graficas..." << endl;
    system("python3 ../src/graficar_pso.py");

    return 0;
}