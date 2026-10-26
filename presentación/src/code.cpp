#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <algorithm>

using namespace std;

// ── Parámetros del algoritmo ──────────────────────────────────
const int    POP_SIZE  = 30;    // Número de alumnos (soluciones)
const int    N_VARS    = 5;     // Dimensiones del problema (materias)
const int    MAX_ITER  = 200;   // Iteraciones máximas
const double LB        = -5.0;  // Límite inferior del espacio de búsqueda
const double UB        =  5.0;  // Límite superior del espacio de búsqueda

// ── Función objetivo: Sphere  f(x) = Σ xᵢ²  (mínimo en x=0) ─
double sphere(const vector<double>& x) {
    double suma = 0.0;
    for (double xi : x)
        suma += xi * xi;    // Acumula xi²
    return suma;
}

// ── Auxiliar: número aleatorio en [a, b] ──────────────────────
double randBetween(double a, double b) {
    return a + (b - a) * (rand() / static_cast<double>(RAND_MAX));
}

// ══════════════════════════════════════════════════════════════
int main() {
    srand(static_cast<unsigned>(time(0)));  // Semilla aleatoria

    // ── 1. INICIALIZACIÓN: población aleatoria ────────────────
    // Cada alumno = un vector de N_VARS variables (solución candidata)
    vector<vector<double>> pop(POP_SIZE, vector<double>(N_VARS));
    for (auto& alumno : pop)
        for (auto& xi : alumno)
            xi = randBetween(LB, UB);   // Valor inicial aleatorio en [LB,UB]

    // ── Bucle principal ───────────────────────────────────────
    for (int iter = 0; iter < MAX_ITER; ++iter) {

        // ══ FASE 1: ENSEÑANZA (Teacher Phase) ════════════════
        // El mejor alumno actúa como Profesor.
        // Todos se acercan a él, alejándose de la media de la clase.

        // 1a. Identificar al Profesor (menor f = mejor solución)
        int idxProf = 0;
        for (int i = 1; i < POP_SIZE; ++i)
            if (sphere(pop[i]) < sphere(pop[idxProf]))
                idxProf = i;

        vector<double>& profesor = pop[idxProf];

        // 1b. Calcular la media de la clase M
        vector<double> media(N_VARS, 0.0);
        for (const auto& alumno : pop)
            for (int j = 0; j < N_VARS; ++j)
                media[j] += alumno[j];
        for (double& m : media)
            m /= POP_SIZE;

        // 1c. Actualizar cada alumno con la fórmula de enseñanza:
        //     x_nuevo = x + r * (x_prof - Tf * M)
        //     Tf ∈ {1, 2}: factor de enseñanza aleatorio
        for (auto& alumno : pop) {
            int Tf = (randBetween(0, 1) > 0.5) ? 2 : 1;
            vector<double> nuevo(N_VARS);
            for (int j = 0; j < N_VARS; ++j) {
                double r = randBetween(0, 1);
                nuevo[j] = alumno[j] + r * (profesor[j] - Tf * media[j]);
                nuevo[j] = max(LB, min(UB, nuevo[j]));  // Clamp a [LB,UB]
            }
            // Selección codiciosa: aceptar solo si mejora
            if (sphere(nuevo) < sphere(alumno))
                alumno = nuevo;
        }

        // ══ FASE 2: APRENDIZAJE (Learner Phase) ══════════════
        // Cada alumno compara con un compañero aleatorio distinto.
        // Se mueve hacia él si tiene mejor calificación, o se aleja si no.

        for (int i = 0; i < POP_SIZE; ++i) {
            // Elegir compañero j distinto de i
            int j;
            do { j = rand() % POP_SIZE; } while (j == i);

            double r = randBetween(0, 1);
            vector<double> nuevo(N_VARS);
            for (int k = 0; k < N_VARS; ++k) {
                if (sphere(pop[j]) < sphere(pop[i]))
                    // Compañero j es mejor → moverse HACIA j
                    nuevo[k] = pop[i][k] + r * (pop[j][k] - pop[i][k]);
                else
                    // Compañero j es peor → alejarse de j
                    nuevo[k] = pop[i][k] + r * (pop[i][k] - pop[j][k]);

                nuevo[k] = max(LB, min(UB, nuevo[k]));  // Clamp a [LB,UB]
            }
            // Selección codiciosa
            if (sphere(nuevo) < sphere(pop[i]))
                pop[i] = nuevo;
        }
    }

    // ── 4. RESULTADO FINAL ────────────────────────────────────
    int mejor = 0;
    for (int i = 1; i < POP_SIZE; ++i)
        if (sphere(pop[i]) < sphere(pop[mejor]))
            mejor = i;

    cout << "Mejor solución encontrada:\n";
    for (int j = 0; j < N_VARS; ++j)
        cout << "  x[" << j << "] = " << pop[mejor][j] << "\n";
    cout << "f(x) = " << sphere(pop[mejor]) << "\n";

    return 0;
}