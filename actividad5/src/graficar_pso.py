import pandas as pd
import matplotlib.pyplot as plt
import os

# Configuración de carpetas
CSV_DIR = "../results/csv/"
PLOT_DIR = "../results/plots/"
os.makedirs(PLOT_DIR, exist_ok=True)

config = {
    "Rosenbrock": {"opt": 0.0, "coords": (1.0, 1.0), "lim": (-5, 10)},
    "Michalewicz": {"opt": -1.8013, "coords": (2.20, 1.57), "lim": (0, 3.1416)}
}

def graficar(func):
    fit_path = os.path.join(CSV_DIR, f"fit_{func}_PSO.csv")
    pob_path = os.path.join(CSV_DIR, f"pob_{func}_PSO.csv")
    
    if not os.path.exists(fit_path): return

    # 1. Convergencia
    df_fit = pd.read_csv(fit_path)
    plt.figure(figsize=(10, 5))
    plt.plot(df_fit['Gen'], df_fit['MejorFitness'], color='darkred')
    plt.axhline(y=config[func]["opt"], color='gray', linestyle='--')
    plt.title(f'Convergencia PSO - {func}')
    plt.savefig(os.path.join(PLOT_DIR, f"Conv_{func}.png"))
    plt.close()

    # 2. Enjambre
    df_pob = pd.read_csv(pob_path)
    gens = sorted(df_pob['Gen'].unique())
    fig, axes = plt.subplots(1, 3, figsize=(15, 5))
    for i, g in enumerate(gens):
        data = df_pob[df_pob['Gen'] == g]
        axes[i].scatter(data['X'], data['Y'], c=data['Fitness'], cmap='viridis')
        axes[i].set_title(f"Gen {g}")
        axes[i].set_xlim(config[func]["lim"])
        axes[i].set_ylim(config[func]["lim"])
    plt.suptitle(f'Evolución Enjambre - {func}')
    plt.savefig(os.path.join(PLOT_DIR, f"Pob_{func}.png"))
    plt.close()

if __name__ == "__main__":
    graficar("Rosenbrock")
    graficar("Michalewicz")
    print("Graficas guardadas en ../results/plots/")