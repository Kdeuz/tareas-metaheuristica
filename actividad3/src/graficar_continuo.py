"""
SCRIPT DE VISUALIZACIÓN PARA ALGORITMO GENÉTICO CONTINUO
---------------------------------------------------------
Este script lee los archivos CSV generados por el programa C++
y crea gráficas automáticas.

AJUSTE REALIZADO:
- Detección automática de escala: Si hay valores negativos (Ackley modificado),
  usa escala LINEAL en lugar de LOGARÍTMICA para evitar errores de graficado.
"""

import pandas as pd              # Librería para manipulación de datos
import matplotlib.pyplot as plt  # Librería estándar para graficar
import glob                      # Herramienta para buscar archivos
import os                        # Interacción con el sistema operativo

# 1. PREPARACIÓN DEL ENTORNO
os.makedirs("../plots", exist_ok=True)

# Buscamos todos los archivos fit_*.csv
archivos_fit = glob.glob("fit_*.csv")
print(f"Graficando {len(archivos_fit)} archivos...")

for fit_file in archivos_fit:
    nombre_base = fit_file.replace("fit_", "").replace(".csv", "")
    pob_file = f"pob_{nombre_base}.csv"
    
    # -------------------------------------------------------
    # 1. GRÁFICA DE CONVERGENCIA (LINEAL/LOG) + RESULTADOS
    # -------------------------------------------------------
    df_fit = pd.read_csv(fit_file)
    
    # Extraemos estadísticas finales
    gen_final = df_fit['Gen'].iloc[-1]
    mejor_fitness = df_fit['Mejor'].iloc[-1]
    promedio_fitness = df_fit['Promedio'].iloc[-1]
    
    plt.figure(figsize=(12, 6)) 
    
    # Dibujamos líneas
    plt.plot(df_fit['Gen'], df_fit['Mejor'], label='Mejor Global', color='blue', linewidth=2)
    plt.plot(df_fit['Gen'], df_fit['Promedio'], label='Promedio', color='orange', linestyle='--', alpha=0.6)
    
    # --- CORRECCIÓN DE ESCALA ---
    # Verificamos si hay valores menores o iguales a 0.
    # La escala logarítmica falla con <= 0.
    min_valor = min(df_fit['Mejor'].min(), df_fit['Promedio'].min())
    
    if min_valor <= 0:
        plt.yscale('linear') # Ackley baja a negativo, usamos lineal
        tipo_escala = "Lineal (Datos <= 0)"
    else:
        plt.yscale('log')    # Esfera es siempre positiva, usamos log
        tipo_escala = "Logarítmica"

    # Etiquetas y título
    plt.title(f'Convergencia: {nombre_base}', fontsize=14)
    plt.xlabel('Generaciones')
    plt.ylabel(f'Fitness ({tipo_escala})')
    plt.grid(True, which="both", alpha=0.3)
    plt.legend(loc='upper right')
    
    # --- BLOQUE DE ESTADÍSTICAS LATERALES ---
    plt.subplots_adjust(right=0.75) 
    
    texto_stats = (
        f"RESULTADOS FINALES\n"
        f"------------------\n"
        f"Generaciones:\n{int(gen_final)}\n\n"
        f"Mejor Fitness:\n{mejor_fitness:.6e}\n\n"
        f"Promedio Final:\n{promedio_fitness:.6e}"
    )
    
    plt.text(1.05, 0.5, texto_stats, transform=plt.gca().transAxes, 
             fontsize=11, verticalalignment='center', 
             bbox=dict(boxstyle="round,pad=0.5", facecolor="white", edgecolor="gray", alpha=0.9))
    
    plt.savefig(f"../plots/Conv_{nombre_base}.png", dpi=100)
    plt.close()

    # -------------------------------------------------------
    # 2. GRÁFICA DE POBLACIÓN (SCATTER PLOT) - SOLO ACKLEY 2D
    # -------------------------------------------------------
    if os.path.exists(pob_file) and "Ackley" in nombre_base:
        df_pob = pd.read_csv(pob_file)
        gens = sorted(df_pob['Gen'].unique())
        
        fig, axes = plt.subplots(1, len(gens), figsize=(5 * len(gens) + 2, 5))
        if len(gens) == 1: axes = [axes]
        
        for i, gen in enumerate(gens):
            data = df_pob[df_pob['Gen'] == gen]
            mejor_del_gen = data['Fitness'].min()
            
            im = axes[i].scatter(data['X0'], data['X1'], c=data['Fitness'], 
                                 cmap='viridis', s=50, alpha=0.7, label='Individuo')
            
            axes[i].set_title(f'Gen {gen}\nMin: {mejor_del_gen:.4e}')
            axes[i].set_xlim(-5, 5)
            axes[i].set_ylim(-5, 5)
            axes[i].plot(0, 0, 'rx', markersize=10, markeredgewidth=2, label='Óptimo (0,0)') 
            
            if i == 0:
                axes[i].legend(loc='lower left', fontsize=8)
            axes[i].grid(True, alpha=0.3)

        # Barra de color
        cbar = fig.colorbar(im, ax=axes.ravel().tolist(), shrink=0.6)
        cbar.set_label('Valor de Fitness (Color)', rotation=270, labelpad=15)

        plt.suptitle(f'Evolución de la Población: {nombre_base}', fontsize=14)
        plt.tight_layout(rect=[0, 0, 0.9, 1]) 
        
        plt.savefig(f"../plots/Pob_{nombre_base}.png", dpi=100)
        plt.close()

print("✓ Proceso completado. Gráficas corregidas en '../results/plots'")