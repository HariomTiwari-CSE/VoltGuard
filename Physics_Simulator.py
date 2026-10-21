import numpy as np
import matplotlib.pyplot as plt
from scipy.integrate import solve_ivp

# 1. Pipe and Fluid Parameters
V = 0.05              # Pipe volume (m^3) ~ 50 liters
beta = 1.5e9          # Effective Bulk Modulus of water (Pa)
rho = 1000.0          # Water density (kg/m^3)
P_in = 1e5            # Reservoir inlet pressure (1 bar in Pa)
P_out = 1e5           # Discharge back-pressure (1 bar in Pa)

# 2. Pump and Valve Coefficients
c1 = 0.02             # Pump pressure head coefficient
c2 = 5e6              # Hydraulic friction coefficient
Cv_max = 2e-4         # Maximum valve flow coefficient (m^2)

# Helper: RPM to rad/s
def rpm_to_rads(rpm):
    return rpm * 2.0 * np.pi / 60.0

# 3. Flows Calculation
def pump_flow(P, omega):
    head_diff = c1 * (omega**2) - (P - P_in) / rho
    return np.sqrt(max(0.0, head_diff / c2))

def valve_flow(P, theta):
    delta_p = max(0.0, P - P_out)
    Cv = Cv_max * np.clip(theta, 0.0, 1.0)
    return Cv * np.sqrt(2.0 * delta_p / rho)

# 4. Differential Equation (dP/dt)
def ode_system(t, state, target_rpm, valve_pct):
    P = state[0]
    omega = rpm_to_rads(target_rpm)
    theta = valve_pct / 100.0

    Q_in = pump_flow(P, omega)
    Q_out = valve_flow(P, theta)

    dP_dt = (beta / V) * (Q_in - Q_out)
    return [dP_dt]

# 5. Run Simulation
def run_simulation(rpm=1500.0, valve_pct=40.0, sim_time=5.0):
    P_initial = [1e5] # Shuru me pipe me 1 bar (ambient) pressure hai
    t_span = (0.0, sim_time)

    # Radau solver use kar rahe hain kyunki beta/V ratio bohot bada hota hai
    sol = solve_ivp(
        ode_system,
        t_span,
        P_initial,
        args=(rpm, valve_pct),
        method="Radau",
        max_step=0.01
    )

    # Results in user-friendly units
    time_array = sol.t
    pressure_bar = sol.y[0] / 1e5 # Pa se bar me conversion

    # Plotting
    plt.figure(figsize=(9, 4.5))
    plt.plot(time_array, pressure_bar, 'b-', linewidth=2, label="Pipe Pressure")
    plt.title(f"Basic Pipe Simulator (Pump: {rpm} RPM, Valve: {valve_pct}%)")
    plt.xlabel("Time (seconds)")
    plt.ylabel("Pressure (bar)")
    plt.grid(True, linestyle="--", alpha=0.6)
    plt.legend()
    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    # Test: Pump 1500 RPM par chalaya aur valve 40% open rakha
    run_simulation(rpm=1500.0, valve_pct=40.0, sim_time=3.0)
