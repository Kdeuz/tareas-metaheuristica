import pandas as pd              # Librería para manejar tablas de datos (como Excel pero en código)
import matplotlib.pyplot as plt  # Librería estándar para crear gráficas científicas
import glob                      # Herramienta para buscar archivos con patrones (ej: *.csv)
import os                        # Para interactuar con el sistema operativo (crear carpetas)

# 1. PREPARACIÓN DEL ENTORNO
# Creamos una carpeta llamada "plots" para guardar las imágenes.
# exist_ok=True evita que el programa falle si la carpeta ya existe.
os.makedirs("plots", exist_ok=True)

# 2. BÚSQUEDA DE DATOS
# Buscamos todos los archivos que comiencen con "fit_" y terminen en ".csv".
# Estos archivos contienen el historial de evolución (fitness por generación).
archivos_csv = glob.glob("fit_*.csv")

# 3. CICLO DE PROCESAMIENTO
# Repetimos el proceso para cada experimento encontrado (cada archivo)
for archivo_fit in archivos_csv:
    # Extraemos el nombre de la configuración (ej: de "fit_F1_B4_M1.csv" obtenemos "F1_B4_M1")
    config = archivo_fit.replace("fit_", "").replace(".csv", "")
    
    # --- GRÁFICA 1: CONVERGENCIA (Evolución del Fitness) ---
    
    # Leemos el CSV y lo convertimos en un DataFrame (tabla)
    df_fit = pd.read_csv(archivo_fit)
    
    # Configuramos el lienzo de la figura (tamaño 10x6 pulgadas)
    plt.figure(figsize=(10, 6))
    
    # Dibujamos dos líneas:
    # 1. La línea sólida azul: El MEJOR individuo (debe bajar siempre)
    plt.plot(df_fit['Gen'], df_fit['Mejor'], label='Mejor Global', linewidth=2)
    # 2. La línea punteada naranja: El PROMEDIO de la población (muestra la diversidad)
    plt.plot(df_fit['Gen'], df_fit['Promedio'], label='Promedio Pob.', linestyle='--', linewidth=2)
    
    # Decoración de la gráfica (Títulos, etiquetas y cuadrícula)
    plt.title(f'Convergencia del Algoritmo: {config}', fontsize=14)
    plt.xlabel('Generación (Tiempo)', fontsize=12)
    plt.ylabel('Valor de Fitness (Costo)', fontsize=12)
    plt.legend(fontsize=10) # Muestra el cuadro con los nombres de las líneas
    plt.grid(True, alpha=0.3) # Cuadrícula suave de fondo
    plt.tight_layout() # Ajusta los márgenes automáticamente
    
    # Guardamos la imagen en la carpeta 'plots' con buena calidad (dpi=150)
    plt.savefig(f"plots/conv_{config}.png", dpi=150)
    plt.close() # Cerramos la figura para liberar memoria RAM

    # --- GRÁFICA 2: POBLACIÓN (Dispersión de Individuos) ---
    
    # Construimos el nombre del archivo de población correspondiente (ej: "pob_F1_B4_M1.csv")
    archivo_pob = f"pob_{config}.csv"
    
    # Solo intentamos graficar si el archivo existe
    if os.path.exists(archivo_pob):
        df_pob = pd.read_csv(archivo_pob)
        
        # Identificamos qué generaciones se guardaron (usualmente: 0, mitad y final)
        generaciones = sorted(df_pob['Gen'].unique())[:3]
        
        # Creamos una figura con 3 sub-gráficas lado a lado (1 fila, 3 columnas)
        fig, ax = plt.subplots(1, 3, figsize=(15, 4))
        
        # Iteramos sobre las 3 generaciones capturadas
        for i, gen in enumerate(generaciones): # i: índice en la lista (0, 1, 2) y gen: generación (el número real de generación)
            # Filtramos los datos solo para la generación actual
            datos = df_pob[df_pob['Gen'] == gen]
            
            # CASO A: FUNCIÓN ACKLEY (F3) -> Es 3D, necesitamos ver X vs Y
            if "F3" in config:
                # Usamos un Scatter Plot (dispersión). El color 'c' indica la altura (fitness).
                scatter = ax[i].scatter(datos['X'], datos['Y'], c=datos['Fitness'], 
                                       cmap='viridis', s=100, alpha=0.6)
                ax[i].set_xlabel('Coordenada X', fontsize=10)
                ax[i].set_ylabel('Coordenada Y', fontsize=10)
                ax[i].set_title(f'Gen {gen} (Vista Superior)', fontsize=11)
                # Añadimos barra de color solo en la última gráfica para referencia
                if i == 2: plt.colorbar(scatter, ax=ax[i], label='Fitness')

            # CASO B: FUNCIONES 1D (F1, F2) -> Solo X vs Fitness
            else:
                # Graficamos la posición X contra su valor de función
                ax[i].scatter(datos['X'], datos['Fitness'], color='blue', s=100, alpha=0.6)
                ax[i].set_xlabel('Variable X', fontsize=10)
                ax[i].set_ylabel('Fitness f(x)', fontsize=10)
                ax[i].set_title(f'Gen {gen}', fontsize=11)
                ax[i].grid(True, alpha=0.3)
        
        # Título general para las 3 sub-gráficas
        plt.suptitle(f'Evolución de la Población: {config}', fontsize=14)
        plt.tight_layout()
        plt.savefig(f"plots/pob_{config}.png", dpi=150)
        plt.close()

print("✓ Gráficas generadas exitosamente en la carpeta 'plots'")