/**
 * ========================================================================================
 * ACTIVIDAD 3: ALGORITMO GENÉTICO CONTINUO (REAL-CODED GA)
 * ========================================================================================
 * Materia: Algoritmos Metaheurísticos
 * * DESCRIPCIÓN TÉCNICA:
 * Este programa implementa un AG que trabaja directamente con números reales (doubles),
 * eliminando la necesidad de decodificación binaria.
 * * CARACTERÍSTICAS PRINCIPALES:
 * 1. Codificación: Directa (Genotipo = Fenotipo). Vector de doubles.
 * 2. Selección: Torneo Binario (presión selectiva controlada).
 * 3. Cruza: Uniforme (intercambio de variables entre padres).
 * 4. Mutación: Desplazamiento uniforme basado en la fórmula de clase: 
 * M[i] = h[i] + Fm * rand(min, max).
 * 5. Control de Diversidad: Mecanismo "Anti-Incest" para evitar cruza de clones.
 * 6. Elitismo: Conservación del mejor individuo de cada generación.
 * ========================================================================================
 */

// 1. LIBRERÍAS
#include <iostream>  // Entrada y salida estándar (cout)
#include <vector>    // Contenedor dinámico para el cromosoma y la población
#include <cmath>     // Funciones matemáticas (exp, sqrt, cos, pow) para Ackley
#include <random>    // Generador de números aleatorios profesional (Mersenne Twister)
#include <algorithm> // Funciones de ayuda (sort para el elitismo)
#include <fstream>   // Manejo de archivos para exportar a CSV
#include <string>    // Manejo de cadenas de texto
#include <cstdlib>   // Para llamadas al sistema (system)
#include <iomanip>   // Para formatear la salida de números (precisión decimal)

// Constantes matemáticas globales necesarias para la función Ackley
const double PI = 3.14159265358979323846;
const double E = 2.71828182845904523536;

// --- ESTRUCTURA DEL INDIVIDUO ---
// A diferencia del AG Binario, aquí no usamos ints (0,1).
// Usamos 'double' para tener mayor precisión decimal.
struct Individuo {
    std::vector<double> variables; // El ADN son los propios valores de las variables (x, y, z...)
    double fitness;                // Calificación del individuo (menor es mejor en minimización)
};

class AlgoritmoGeneticoContinuo {
private:
    // Configuración del problema
    std::string nombreFuncion; // Nombre para identificar archivos (ej: "Ackley")
    int numVariables;          // Dimensionalidad (2 para Ackley, 4 o 6 para Esfera)
    double minRango;           // Límite inferior del espacio de búsqueda (ej: -5.0)
    double maxRango;           // Límite superior del espacio de búsqueda (ej: 5.0)
    
    // Hiperparámetros del AG
    int tamPoblacion;          // Cantidad de individuos (ej: 100)
    int maxGeneraciones;       // Criterio de paro (ej: 200)
    double tasaMutacion;       // Probabilidad de que un gen mute (ej: 0.1 o 10%)
    double factorMutacion;     // variable 'Fm' de la fórmula de clase (Amplitud de la mutación)

    // Estado interno
    std::vector<Individuo> poblacion; // Lista actual de individuos
    std::mt19937 rng;                 // Motor aleatorio (Mersenne Twister 19937)

public:
    // --- CONSTRUCTOR ---
    // Inicializa el generador aleatorio y configura los parámetros del experimento.
    AlgoritmoGeneticoContinuo(std::string nombre, int nVars, double minR, double maxR, int pob, int gens, double mutRate, double fm)
        : nombreFuncion(nombre), numVariables(nVars), minRango(minR), maxRango(maxR), 
          tamPoblacion(pob), maxGeneraciones(gens), tasaMutacion(mutRate), factorMutacion(fm) {
        
        std::random_device rd;    // Obtiene una semilla aleatoria del hardware
        rng = std::mt19937(rd()); // Inicializa el motor Mersenne Twister (más robusto que rand())
    }

    // --- 1. FUNCIÓN DE EVALUACIÓN (FITNESS) ---
    // Recibe el vector de variables y calcula el valor Z o f(x)
    double evaluar(const std::vector<double>& v) {
        double fitness = 0.0;

        if (nombreFuncion == "Ackley") {
            // Función de Ackley (Multimodal - Difícil):
            // Tiene muchos mínimos locales y un solo mínimo global en 0.
            double x = v[0];
            double y = v[1];
            // Parte 1: Exponencial negativa de la distancia (hace el hoyo)
            double term1 = -20 * std::exp(-20 * std::sqrt(0.5 * (x*x + y*y)));
            // Parte 2: Cosenos (crean las ondulaciones/montañas pequeñas)
            double term2 = -std::exp(0.5 * (std::cos(2*PI*x) + std::cos(2*PI*y)));
            fitness = term1 + term2 + E;
        } 
        else if (nombreFuncion.find("Esfera") != std::string::npos) {
            // Función Esfera (Unimodal - Fácil):
            // Es la suma de los cuadrados. f(x) = x1^2 + x2^2 + ... + xn^2
            // Sirve para probar la velocidad de convergencia en N dimensiones.
            for(double val : v) {
                fitness += val * val;
            }
        }
        return fitness;
    }

    // --- 2. INICIALIZACIÓN DE LA POBLACIÓN ---
    // Crea la primera generación con valores reales aleatorios dentro del rango [min, max].
    void inicializar() {
        poblacion.clear();
        // Distribución uniforme real (flotante) entre minRango y maxRango
        std::uniform_real_distribution<double> dist(minRango, maxRango);

        for (int i = 0; i < tamPoblacion; ++i) {
            Individuo ind;
            for (int j = 0; j < numVariables; ++j) {
                ind.variables.push_back(dist(rng)); // Genera gen aleatorio
            }
            ind.fitness = evaluar(ind.variables); // Calcula qué tan bueno es
            poblacion.push_back(ind);
        }
    }

    // --- 3. SELECCIÓN POR TORNEO ---
    // Elige 2 individuos al azar y retorna el mejor de ellos.
    Individuo torneo() {
        std::uniform_int_distribution<int> dist(0, tamPoblacion - 1);
        Individuo a = poblacion[dist(rng)];
        Individuo b = poblacion[dist(rng)];
        // Minimización: Retornamos el que tenga MENOR fitness
        return (a.fitness < b.fitness) ? a : b;
    }

    // --- 4. OPERADOR DE MUTACIÓN ---
    // Implementa la fórmula vista en clase para perturbar soluciones.
    void mutar(Individuo& ind) {
        std::uniform_real_distribution<double> prob(0.0, 1.0);
        
        // Genera un número aleatorio dentro de todo el espacio de búsqueda.
        std::uniform_real_distribution<double> randRango(minRango, maxRango);

        for (int i = 0; i < numVariables; ++i) {
            // Evaluamos si este gen específico debe mutar
            if (prob(rng) < tasaMutacion) {
                
                // === FÓRMULA DE MUTACIÓN ===
                // M[i] = h[i] + Fm * rand(min, max)
                // Nuevo = Actual + (FactorFm * ValorAleatorioGrande)
                double valorAleatorio = randRango(rng); 
                double nuevoValor = ind.variables[i] + (factorMutacion * valorAleatorio);

                // === VALIDACIÓN DE LÍMITES ===
                // Si la mutación empuja al individuo fuera del mapa (ej: x = 6.5 cuando max es 5.0),
                // lo regresamos al borde. Esto es vital para mantener la validez de la solución.
                if (nuevoValor < minRango) nuevoValor = minRango;
                if (nuevoValor > maxRango) nuevoValor = maxRango;
                
                ind.variables[i] = nuevoValor;
            }
        }
    }

    // --- 5. MOTOR PRINCIPAL (EJECUTAR) ---
    // Orquesta todo el proceso evolutivo
    void ejecutar() {
        inicializar();

        // Configuración de archivos CSV para guardar datos
        std::string filename = nombreFuncion + "_Vars" + std::to_string(numVariables);
        std::ofstream fitFile("../results/csv/fit_" + filename + ".csv"); // Historial de fitness
        std::ofstream pobFile("../results/csv/pob_" + filename + ".csv"); // Historial de posiciones

        // Escribimos encabezados CSV
        fitFile << "Gen,Mejor,Promedio\n";
        pobFile << "Gen";
        for(int i=0; i<numVariables; ++i) pobFile << ",X" << i;
        pobFile << ",Fitness\n";

        std::cout << "-> Procesando experimento: " << filename << "...\n";

        // Bucle Generacional
        for (int g = 0; g < maxGeneraciones; ++g) {
            
            // A. ELITISMO Y ESTADÍSTICAS
            // Ordenamos la población del mejor al peor
            std::sort(poblacion.begin(), poblacion.end(), [](const Individuo& a, const Individuo& b) {
                return a.fitness < b.fitness; 
            });

            // Guardamos datos
            double sumaFit = 0;
            // Solo guardamos la población completa al inicio, mitad y fin para ahorrar espacio
            bool guardarPob = (g == 0 || g == maxGeneraciones/2 || g == maxGeneraciones-1);

            for (const auto& ind : poblacion) {
                sumaFit += ind.fitness;
                if (guardarPob) {
                    pobFile << g;
                    for(double x : ind.variables) pobFile << "," << x;
                    pobFile << "," << ind.fitness << "\n";
                }
            }
            fitFile << g << "," << poblacion[0].fitness << "," << (sumaFit / tamPoblacion) << "\n";

            // B. CREACIÓN DE NUEVA GENERACIÓN
            std::vector<Individuo> nuevaPoblacion;
            
            // Pasamos al mejor individuo directamente (Elitismo) para no perder la mejor solución encontrada
            nuevaPoblacion.push_back(poblacion[0]); 

            std::uniform_real_distribution<double> prob(0.0, 1.0);
            
            // Llenamos el resto de la población con hijos
            while (nuevaPoblacion.size() < tamPoblacion) {
                
                // 1. Selección de Padres
                Individuo p1 = torneo();
                Individuo p2 = torneo();

                // ==========================================
                //    MECANISMO ANTI-INCESTO (DIVERSIDAD)
                // ==========================================
                // Problema: Si p1 y p2 son idénticos, el hijo será un clon y no habrá evolución.
                // Solución: Si detectamos que son iguales, intentamos buscar otro padre hasta 10 veces.
                int intentos = 0;
                while (p1.variables == p2.variables && intentos < 10) {
                    p2 = torneo(); 
                    intentos++;
                }
                // ==========================================

                Individuo hijo = p1; // El hijo comienza como copia de P1

                // 2. Cruza Uniforme
                // Recorremos gen por gen. Con 50% de probabilidad, tomamos el gen de P2.
                // Esto mezcla la información de ambos padres.
                for (int i = 0; i < numVariables; ++i) {
                    if (prob(rng) < 0.5) hijo.variables[i] = p2.variables[i];
                }

                // 3. Mutación
                mutar(hijo); 
                
                // 4. Evaluación del nuevo hijo
                hijo.fitness = evaluar(hijo.variables);
                nuevaPoblacion.push_back(hijo);
            }
            // Reemplazo generacional completo
            poblacion = nuevaPoblacion;
        }

        // Cerramos archivos y mostramos resultado final
        fitFile.close();
        pobFile.close();
        std::cout << "   [OK] Completado. Mejor Fitness Final: " << std::fixed << std::setprecision(6) << poblacion[0].fitness << "\n";
    }
};

int main() {
    // Paso 0: Crear carpetas para organizar los resultados
    // -p evita error si ya existen
    system("mkdir -p ../results/csv ../results/plots");

    std::cout << "=== ALGORITMO GENETICO CONTINUO CON ANTI-INCESTO ===\n";
    std::cout << "=== Materia: Algoritmos Metaheuristicos ===\n\n";

    // --- CONFIGURACIÓN DE PARÁMETROS ---
    int pob = 100;      // Población (100 individuos es bueno para continuo)
    int gen = 200;      // Generaciones
    double mut = 0.1;   // Tasa de mutación (10% de probabilidad)
    double Fm = 0.5;    // Factor de Mutación (Fuerza del salto aleatorio)

    // --- EJECUCIÓN DE LOS 3 CASOS DE PRUEBA ---

    // Caso 1: Función Ackley (Compleja, 2 variables, Rango -5 a 5)
    AlgoritmoGeneticoContinuo ackley("Ackley", 2, -5.0, 5.0, pob, gen, mut, Fm);
    ackley.ejecutar();

    // Caso 2: Función Esfera (Simple, 4 variables, Rango -5.12 a 5.12)
    AlgoritmoGeneticoContinuo esfera4("Esfera", 4, -5.12, 5.12, pob, gen, mut, Fm);
    esfera4.ejecutar();

    // Caso 3: Función Esfera (Simple, 6 variables, Rango -5.12 a 5.12)
    AlgoritmoGeneticoContinuo esfera6("Esfera", 6, -5.12, 5.12, pob, gen, mut, Fm);
    esfera6.ejecutar();

    // --- GENERACIÓN DE GRÁFICAS ---
    std::cout << "\nLlamando al script de Python para visualizar resultados...\n";
    system("cd ../results/csv && python3 ../../src/graficar_continuo.py");
    
    std::cout << "\nProceso finalizado exitosamente.\n";
    return 0;
}