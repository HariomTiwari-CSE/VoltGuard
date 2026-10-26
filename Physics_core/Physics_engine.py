import numpy as np
import matplotlib.pyplot as plt
from scipy.integrate import solve_ivp

# Pipe and Fluid Parameters
V = 0.05              # Pipe volume (m^3) ~ 50 liters
beta = 1.5e9          # Effective Bulk Modulus of water (Pa)
rho = 1000.0          # Water density (kg/m^3)
P_in = 1e5            # Reservoir inlet pressure (1 bar in Pa)
P_out = 1e5           # Discharge back-pressure (1 bar in Pa)

# Pump and Valve Coefficients
c1 = 0.02             # Pump pressure head coefficient
c2 = 5e6              # Hydraulic friction coefficient
Cv_max = 2e-4         # Maximum valve flow coefficient (m^2)


# RPM to rad/s
def rpm_to_rads(rpm: float) -> float:
    return float(rpm) * 2.0 * np.pi / 60.0



# Flows Calculation
def pump_flow(P: float , omega :float) -> float:
    head_diff = c1 * (omega**2) - (P - P_in) / rho
    return np.sqrt(max(0.0, head_diff / c2))

def valve_flow(P: float, theta: float) -> float:
    delta_p = max(0.0, P - P_out)
    Cv = Cv_max * np.clip(theta, 0.0, 1.0)
    return Cv * np.sqrt(2.0 * delta_p / rho)



# Differential Equation (dP/dt)
def ode_system(t, state, target_rpm: float, valve_pct: float):
    P = state[0]
    omega = rpm_to_rads(target_rpm)
    theta = float(valve_pct) / 100.0

    Q_in = pump_flow(P, omega)
    Q_out = valve_flow(P, theta)

    dP_dt = (beta / V) * (Q_in - Q_out)
    return [dP_dt]

    
# Prediction API
    def simulate_transient(current_p_pa: float, target_rpm: float, valve_pct: float, horizon_sec: float = 3.0) -> dict:
        if target_rpm < 0.0 :
            raise ValueError(f"Impossible physics : Negative pump speed ({target_rpm} RPM) recieved. ")
        if valve_pct < 0.0 or valve_pct > 100.0 :
            raise ValueError("Impossible physics : Valve percentage ({valve_pct}%) outside the physical range[0,100].")
        if current_p_pa < 0.0 :
            raise ValueError("Impossible physics : Negative absolute pipeline pressure({current_p_pa} Pa).")
        
        t_span = (0.0 , horizon_sec)
        initial_state = [current_p_pa]
        

    # Radau solver -> to calculate complex beta/V ratio 
    sol = solve_ivp(
        ode_system,
        t_span,
        initial_state,
        args=(target_rpm, valve_pct),
        method="Radau",
        max_step=0.01
    )

    pressure_bar = sol.y[0] / 1e5           # Pa se bar me conversion

    return
    {
        "time" : sol.t ,
        "pressure_bar" : pressure_bar ,
        "peak_pressure_bar" : float(np.max(pressure_bar)) ,
        "min_pressure_bar" : float(np.min(pressure_bar)) ,
        "final_pressure_bar" : float(pressure_bar[-1]) ,
        "final_pressure_pa" : float(sol.y[0][-1])
    }

