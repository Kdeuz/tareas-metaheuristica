// 1. Entrada y Salida
#include <iostream>  // Permite imprimir en consola (cout) para ver el progreso de las generaciones y leer entrada (cin).

// 2. Estructuras de Datos
#include <vector>    // Contenedor principal para la población. Permite arreglos dinámicos (resize, push_back) de individuos.

// 3. Matemáticas
#include <cmath>     // Funciones matemáticas para la Función Ackley (exp, sqrt, cos, M_PI).

// 4. Aleatoriedad
#include <random>    // Generación de números aleatorios moderna (Mersenne Twister). Vital para la inicialización, cruza y mutación.

// 5. Algoritmos
#include <algorithm> // Herramientas para ordenar la población (sort) por fitness o encontrar el mejor individuo (min_element).

// 6. Archivos
#include <fstream>   // Manejo de archivos (File Stream). Se usa para guardar los datos (CSV/TXT) que el script de Python leerá después.

// 7. Cadenas de Texto
#include <string>    // Manipulación de texto. Útil para generar nombres de archivos dinámicos (ej: "gen_01.csv", "gen_02.csv").

// 8. Sistema
#include <cstdlib>   // Interacción con el sistema operativo. Permite usar 'system()' para ejecutar comandos de terminal (como llamar al script de Python).

// Constantes matemáticas necesarias porque no siempre están definidas en todos los compiladores
const double PI = 3.14159265358979323846;
const double E = 2.71828182845904523536;

// ESTRUCTURA DEL INDIVIDUO
// Representa una posible solución al problema.
struct Individuo {
    std::vector<int> genes;      // El "ADN" (Ceros y Unos). Ejemplo: [1, 0, 1, 1]
    std::vector<double> variables; // El valor real traducido. Ejemplo: [2.5]
    double fitness;              // La "calificación". Qué tan buen resultado da en la función.
};

class AlgoritmoGenetico {
private:
    int idFuncion;          // Cuál función estamos resolviendo (1, 2 o 3)
    int numBits;            // Resolución del genoma (4 u 8 bits)
    double tasaMutacion;    // Probabilidad de cambio genético (1%, 3%, 5%)
    int tamPoblacion;       // Cuántos individuos viven en cada generación
    int maxGeneraciones;    // Cuándo detener la evolución
    int numVariables;       // 1 variable (x) o 2 variables (x, y)
    double minRango, maxRango; // Límites del terreno de búsqueda [-5, 3] o [-5, 5]

    std::vector<Individuo> poblacion; // La comunidad actual de soluciones
    std::mt19937 rng; // Motor de números aleatorios avanzado (Mersenne Twister)

public:
    // CONSTRUCTOR: Configura el experimento inicial
    AlgoritmoGenetico(int func, int bits, double mut, int popSize, int gens)
        : idFuncion(func), numBits(bits), tasaMutacion(mut), tamPoblacion(popSize), maxGeneraciones(gens) {
        
        std::random_device rd;
        rng = std::mt19937(rd()); // Iniciamos la "semilla" aleatoria

        // Ajustamos las reglas del juego según la función elegida
        if (idFuncion == 3) { // Función Ackley (Compleja, 2 variables)
            numVariables = 2;
            minRango = -5.0;
            maxRango = 5.0;
        } else { // Funciones Polinómicas (Simples, 1 variable)
            numVariables = 1;
            minRango = -5.0;
            maxRango = 3.0;
        }
    }

    // DECODIFICADOR: El traductor de Binario a Decimal
    // Convierte el lenguaje de la máquina (bits) al lenguaje del problema (números reales)
    double decodificar(const std::vector<int>& genes, int inicio) {
        unsigned long long decimal = 0;
        
        // Paso 1: Convertir binario a entero usando desplazamiento de bits (Bitwise Shift)
        // Es más eficiente que usar potencias matemáticas.
        for (int i = 0; i < numBits; ++i) {
            decimal = (decimal << 1) | genes[inicio + i];
        }
        
        // Paso 2: Regla de tres para ajustar el entero al rango real [-5, 3]
        double maxPosible = std::pow(2, numBits) - 1; 
        return minRango + (decimal / maxPosible) * (maxRango - minRango);
    }

    // FUNCIÓN OBJETIVO:
    // Evalúa qué tan buena es una solución (x, y). Buscamos el valor más bajo (mínimo).
    double evaluar(double x, double y = 0) {
        if (idFuncion == 1) {
            return std::pow(x, 3) + 4 * std::pow(x, 2) - 4 * x + 1;
        } else if (idFuncion == 2) {
            return std::pow(x, 4) + 5 * std::pow(x, 3) + 4 * std::pow(x, 2) - 4 * x + 1;
        } else {
            // Función de Ackley: Muchos mínimos locales tramposos y un solo mínimo global.
            double t1 = -20 * std::exp(-20 * std::sqrt(0.5 * (x*x + y*y)));
            double t2 = -std::exp(0.5 * (std::cos(2*PI*x) + std::cos(2*PI*y)));
            return E + t1 + t2; 
        }
    }

    // GÉNESIS: Crea la primera generación
    void inicializar() {
        poblacion.clear();
        std::uniform_int_distribution<int> dist(0, 1); // Lanzar moneda: 0 o 1
        
        for (int i = 0; i < tamPoblacion; ++i) {
            Individuo ind;
            // Llenamos su ADN con bits al azar
            for (int b = 0; b < numBits * numVariables; ++b) {
                ind.genes.push_back(dist(rng)); 
            }
            calcularFitness(ind); // Evaluamos qué tan bueno nació
            poblacion.push_back(ind);
        }
    }

    // EVALUADOR INDIVIDUAL
    // Proceso completo: Decodificar ADN -> Obtener Variables -> Calcular Fitness
    void calcularFitness(Individuo& ind) {
        ind.variables.clear();
        double x = decodificar(ind.genes, 0); // Sacamos X
        ind.variables.push_back(x);
        
        double y = 0;
        if (numVariables > 1) { // Si es Ackley, sacamos Y también
            y = decodificar(ind.genes, numBits);
            ind.variables.push_back(y);
        }
        
        ind.fitness = evaluar(x, y);
    }

    // SELECCIÓN: Torneo
    // Enfrentamos a 2 individuos al azar y gana el que tenga MEJOR fitness (menor valor)
    Individuo torneo() {
        std::uniform_int_distribution<int> dist(0, tamPoblacion - 1);
        Individuo a = poblacion[dist(rng)];
        Individuo b = poblacion[dist(rng)];
        return (a.fitness < b.fitness) ? a : b; 
    }

    // EVOLUCIÓN: El ciclo de la vida
    void evolucionar() {
        std::vector<Individuo> nuevaPoblacion;
        
        // 1. ELITISMO
        // Ordenamos y pasamos al mejor individuo directamente para no perderlo.
        std::sort(poblacion.begin(), poblacion.end(), [](const Individuo& a, const Individuo& b) {
            return a.fitness < b.fitness;
        }); 
        nuevaPoblacion.push_back(poblacion[0]);

        // Probabilidades y distribuciones para cruza y mutación
        std::uniform_real_distribution<double> prob(0.0, 1.0);
        std::uniform_int_distribution<int> crossDist(1, (numBits * numVariables) - 1);

        // Llenamos el resto de la nueva población con hijos
        while (nuevaPoblacion.size() < tamPoblacion) {
            // A. Selección de Padres
            Individuo p1 = torneo();
            Individuo p2 = torneo();

            // Si los padres son idénticos, intentamos buscar un p2 diferente
            int intentos = 0;
            while (p1.genes == p2.genes && intentos < 10) {
                p2 = torneo(); // Volvemos a sortear al segundo padre
                intentos++;
            }


            Individuo h1 = p1; // Los hijos inician como copias
            Individuo h2 = p2;

            // B. Cruza (Crossover) - Probabilidad 70%
            if (prob(rng) < 0.7) { 
                int punto = crossDist(rng); // Elegimos dónde cortar el ADN
                // Intercambiamos la cola de los padres
                for (int i = punto; i < p1.genes.size(); ++i) { 
                    h1.genes[i] = p2.genes[i]; 
                    h2.genes[i] = p1.genes[i]; 
                }
            }

            // C. Mutación - Probabilidad según configuración (1%, 3%, 5%)
            // Recorremos cada bit para ver si "muta" (se invierte)
            for (int i = 0; i < h1.genes.size(); ++i) {
                if (prob(rng) < tasaMutacion) h1.genes[i] = std::abs(h1.genes[i] - 1); 
                if (prob(rng) < tasaMutacion) h2.genes[i] = std::abs(h2.genes[i] - 1);
            }

            // D. Nacimiento
            calcularFitness(h1);
            calcularFitness(h2);
            nuevaPoblacion.push_back(h1);
            
            // Solo insertamos al segundo hijo si aún hay espacio
            if(nuevaPoblacion.size() < tamPoblacion) nuevaPoblacion.push_back(h2); 
        }
        poblacion = nuevaPoblacion; // La nueva generación reemplaza a la vieja
    }

    // FUNCIÓN PRINCIPAL DEL ALGORITMO
    void ejecutar() {
        inicializar(); 
        
        // Creamos un nombre de archivo único para guardar los resultados de este experimento
        // Ejemplo: "F1_B4_M1"
        std::string config = "F" + std::to_string(idFuncion) + "_B" + std::to_string(numBits) + "_M" + std::to_string((int)(tasaMutacion*100));
        
        std::ofstream fitFile("../results/csv/fit_" + config + ".csv"); // Historial de mejora
        std::ofstream pobFile("../results/csv/pob_" + config + ".csv"); // Mapa de individuos
        
        fitFile << "Gen,Mejor,Promedio\n";
        pobFile << "Gen,X,Y,Fitness\n";

        // Ciclo de Generaciones
        for (int g = 0; g < maxGeneraciones; ++g) { 
            double suma = 0, mejor = 1e9;
            
            // Solo guardamos la población completa al inicio, mitad y fin para no llenar el disco
            bool guardarPoblacion = (g == 0 || g == maxGeneraciones/2 || g == maxGeneraciones-1);

            for (auto& ind : poblacion) {
                suma += ind.fitness;
                if (ind.fitness < mejor) mejor = ind.fitness;
                
                if (guardarPoblacion) {
                    double yVal = (numVariables > 1) ? ind.variables[1] : 0.0;
                    pobFile << g << "," << ind.variables[0] << "," << yVal << "," << ind.fitness << "\n";
                }
            }
            // Guardamos el progreso de la generación
            fitFile << g << "," << mejor << "," << (suma/tamPoblacion) << "\n";
            
            // Si no es la última vuelta, evolucionamos
            if (g < maxGeneraciones - 1) evolucionar();
        }
        
        fitFile.close();
        pobFile.close();
        std::cout << "  ✓ Experimento terminado: " << config << "\n";
    }
};

int main() {
    // 1. Preparamos el entorno creando carpetas
    system("mkdir -p ../results/csv ../results/plots");
    
    std::cout << "\n=== Optimizador Genético Binario ===\n";
    std::cout << "Iniciando serie de pruebas automatizadas...\n\n";
    
    // 2. Definimos todos los casos de prueba solicitados
    std::vector<int> funciones = {1, 2, 3};
    std::vector<int> bits = {4, 8};
    std::vector<double> mutaciones = {0.01, 0.03, 0.05};

    // 3. Ejecutamos todos los experimentos en cadena (18 combinaciones)
    for(int f : funciones) {
        for(int b : bits) {

            int poblacion;
            int generaciones;

            if (b == 4) {
            // El universo es diminuto (16 opciones).
            poblacion = 10;     
            generaciones = 20;  // Converge casi instantáneamente.
        } else {
            // b == 8
            // El universo es mediano (256 opciones).
            poblacion = 40;     
            generaciones = 50; 
        }

            for(double m : mutaciones) {
                AlgoritmoGenetico algoritmo(f, b, m, poblacion, generaciones); 
                algoritmo.ejecutar();
            }
        }
    }
    
    std::cout << "\n✓ Cálculos finalizados.\n";
    std::cout << "Generando visualizaciones con Python...\n\n";
    
    // 4. Llamamos al script de Python para crear las gráficas
    system("cd ../results/csv && python3 ../../src/graficar.py");
    
    std::cout << "✓ Proceso completo. Gráficas disponibles en results/plots/\n\n";
    
    return 0;
}