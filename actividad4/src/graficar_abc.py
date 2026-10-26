import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.patches as patches
import os

# Asegurar que el directorio de gráficas exista
os.makedirs("../plots", exist_ok=True)

print("Generando graficas actualizadas...")

# 1. VALORES PARA COMPARACIÓN
# El valor teórico es una constante conocida para la función Eggholder
valor_teorico = -959.6407

# 1. GRÁFICA DE CONVERGENCIA CON COMPARACIÓN

archivo_fit = "fit_Eggholder_ABC.csv"

if os.path.exists(archivo_fit):
    df_fit = pd.read_csv(archivo_fit)
    mejor_encontrado = df_fit['MejorCosto'].iloc[-1]
    gen_final = df_fit['Gen'].iloc[-1]
    
    plt.figure(figsize=(12, 6))
    plt.plot(df_fit['Gen'], df_fit['MejorCosto'], color='red', linewidth=2, label='Mejor Encontrado')
    
    # Línea horizontal para representar el objetivo teórico
    plt.axhline(y=valor_teorico, color='black', linestyle=':', alpha=0.7, label='Objetivo Teórico')
    
    plt.title('Convergencia Algoritmo ABC: Función Eggholder', fontsize=14)
    plt.xlabel('Generaciones')
    plt.ylabel('Costo de la Función f(x,y)')
    plt.grid(True, alpha=0.4)
    plt.legend(loc='upper right')
    
    # Ajustar margen para el cuadro de comparación
    plt.subplots_adjust(right=0.7) 
    
    # Cuadro de comparación detallado
    texto_comparativo = (
        f"COMPARACIÓN DE RESULTADOS\n"
        f"--------------------------\n"
        f"Valor Teórico:\n {valor_teorico:.4f}\n\n"
        f"Valor Encontrado:\n {mejor_encontrado:.4f}\n\n"
        f"Diferencia:\n {abs(valor_teorico - mejor_encontrado):.6f}\n\n"
        f"Generaciones: {int(gen_final)}"
    )
    
    plt.text(1.05, 0.5, texto_comparativo, transform=plt.gca().transAxes, 
             fontsize=11, verticalalignment='center', 
             bbox=dict(boxstyle="round,pad=0.5", facecolor="white", edgecolor="gray", alpha=0.9))
    
    plt.savefig("../plots/Conv_Eggholder_ABC.png", dpi=100)
    plt.close()

# 2. GRÁFICA DE POBLACIÓN (REPARACIÓN DE BARRA DE COLOR)

archivo_pob = "pob_Eggholder_ABC.csv"

if os.path.exists(archivo_pob):
    df_pob = pd.read_csv(archivo_pob)
    gens = sorted(df_pob['Gen'].unique())
    
    # Aumentamos el ancho total para dar espacio real a la barra lateral
    fig, axes = plt.subplots(1, len(gens), figsize=(6 * len(gens), 5))
    if len(gens) == 1: axes = [axes]
    
    nombres_fases = ["Inicial", "Intermedia", "Final"]
    
    for i, gen in enumerate(gens):
        data = df_pob[df_pob['Gen'] == gen]
        
        # Graficar abejas
        im = axes[i].scatter(data['X'], data['Y'], c=data['Costo'], cmap='coolwarm', 
                             s=45, alpha=0.8, edgecolor='k', zorder=2)
        
        titulo_fase = nombres_fases[i] if i < len(nombres_fases) else f"Gen {gen}"
        axes[i].set_title(f'Población {titulo_fase}\n(Gen {gen})')
        
        # Encuadre
        axes[i].set_xlim(-550, 550)
        axes[i].set_ylim(-550, 550)
        
        # Marco del espacio de búsqueda
        rect = patches.Rectangle((-512, -512), 1024, 1024, linewidth=1, 
                                 edgecolor='gray', facecolor='none', linestyle='--', zorder=1)
        axes[i].add_patch(rect)
        
        # Óptimo teórico (marcador ajustado)
        axes[i].scatter(512, 404.2319, marker='o', s=80, facecolor='red', 
                        edgecolor='black', linewidth=1.5, label='Óptimo Teórico', zorder=10)
        
        if i == 0:
            axes[i].legend(loc='lower left', fontsize=9)
        axes[i].grid(True, alpha=0.3)

    # CORRECCIÓN DE LA BARRA: 
    # subplots_adjust libera espacio a la derecha y fig.colorbar usa ese espacio específico
    plt.subplots_adjust(right=0.88, wspace=0.3)
    cax = fig.add_axes([0.91, 0.15, 0.02, 0.7]) # [left, bottom, width, height]
    cbar = fig.colorbar(im, cax=cax)
    cbar.set_label('Costo f(x,y)', rotation=270, labelpad=15)

    plt.suptitle('Evolución de las Fuentes de Alimento (Comparativa Visual)', fontsize=15, y=0.98)
    
    plt.savefig("../plots/Pob_Eggholder_ABC.png", dpi=100, bbox_inches='tight')
    plt.close()

print("✓ Gráficas corregidas. Se añadió cuadro de comparación y se movió la barra de color.")