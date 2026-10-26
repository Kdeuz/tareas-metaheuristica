import pandas as pd
import matplotlib.pyplot as plt
import os
import warnings

warnings.filterwarnings("ignore")

CSV_DIR = "../results/csv/"
PLOT_DIR = "../results/plots/"

config = {
    "Michalewicz": {
        "opt": -1.8013, 
        "coords": [(2.20, 1.57)], 
        "lim_x": (0.0, 3.1416), "lim_y": (0.0, 3.1416)
    },
    "SixHumpCamel": {
        "opt": -1.0316, 
        "coords": [(0.0898, -0.7126), (-0.0898, 0.7126)], # Tiene dos mínimos globales
        "lim_x": (-3.0, 3.0), "lim_y": (-2.0, 2.0)
    }
}

def graficar(func):
    fit_path = os.path.join(CSV_DIR, f"fit_{func}_DE.csv")
    pob_path = os.path.join(CSV_DIR, f"pob_{func}_DE.csv")
    
    if not os.path.exists(fit_path) or not os.path.exists(pob_path): 
        print(f"No se encontraron los datos de {func}")
        return

    # --- Gráfica de Convergencia ---
    df_fit = pd.read_csv(fit_path)
    opt_teorico = config[func]["opt"]
    mejor_encontrado = df_fit['MejorFitness'].iloc[-1]
    
    plt.figure(figsize=(10, 5))
    plt.plot(df_fit['Gen'], df_fit['MejorFitness'], color='darkblue', linewidth=2, label='Mejor Fitness (DE)')
    plt.axhline(y=opt_teorico, color='gray', linestyle=':', label=f'Optimo Teorico ({opt_teorico})')
    
    plt.title(f'Convergencia DE - {func}')
    plt.xlabel('Generacion')
    plt.ylabel('Fitness')
    plt.grid(True, alpha=0.3)
    plt.legend()
    
    texto = (f"RESULTADOS\n"
             f"-------------------\n"
             f"Optimo teorico:\n{opt_teorico:.6f}\n\n"
             f"Valor encontrado:\n{mejor_encontrado:.6f}\n\n"
             f"Diferencia:\n{abs(opt_teorico - mejor_encontrado):.6f}\n\n"
             f"Gens: 200 | Parts: 50\nF: 0.8 | CR: 0.9")
    
    plt.subplots_adjust(right=0.75)
    plt.text(1.05, 0.5, texto, transform=plt.gca().transAxes,
             fontsize=10, verticalalignment='center',
             bbox=dict(boxstyle="round,pad=0.5", facecolor="white", edgecolor="darkblue"))
    
    plt.savefig(os.path.join(PLOT_DIR, f"Conv_{func}.png"), dpi=120, bbox_inches='tight')
    plt.close()

    # --- Gráfica de Población ---
    df_pob = pd.read_csv(pob_path)
    gens = sorted(df_pob['Gen'].unique())
    titulos = ["Inicial (Gen 0)", "Intermedia", "Final"]
    
    fig, axes = plt.subplots(1, 3, figsize=(18, 5))
    
    for i, g in enumerate(gens):
        data = df_pob[df_pob['Gen'] == g]
        im = axes[i].scatter(data['X'], data['Y'], c=data['Fitness'], cmap='viridis_r', s=40, edgecolor='gray')
        
        axes[i].set_title(f"Poblacion {titulos[i]} - {func}")
        axes[i].set_xlim(config[func]["lim_x"])
        axes[i].set_ylim(config[func]["lim_y"])
        axes[i].grid(True, alpha=0.2)
        
        # Marcar los mínimos globales teóricos
        for ox, oy in config[func]["coords"]:
            axes[i].scatter(ox, oy, marker='*', s=150, color='red', edgecolor='black', label='Optimo teorico' if i==0 and ox==config[func]["coords"][0][0] else "")
        if i == 0:
            axes[i].legend(loc='best', fontsize=8)
            
    plt.subplots_adjust(right=0.9)
    cax = fig.add_axes([0.92, 0.15, 0.015, 0.7])
    fig.colorbar(im, cax=cax, label='Fitness')
    
    plt.savefig(os.path.join(PLOT_DIR, f"Pob_{func}.png"), dpi=120, bbox_inches='tight')
    plt.close()

if __name__ == "__main__":
    graficar("Michalewicz")
    graficar("SixHumpCamel")