# Individual Contribution Documentation: Physics Core & Digital Twin Module

**Author:** Hariom Tiwari **Role:** Physics Core & Industrial Digital Twin Modeling **Project:** VoltGuard Industrial OT Firewall & IPS **Stack:** Python 3, NumPy/SciPy, ordinary differential equations (ODE), POSIX TCP sockets

## Week 1: Mathematical Modeling & Baseline Fluid Dynamics

**Focus:** Physical baseline safety envelopes and differential equations for pipeline hydraulics.

- **Hydraulic model.** Formulated a continuous model of pipeline fluid mechanics, combining centrifugal pump affinity laws (pressure ∝ ω^1.6) with non-linear downstream valve resistance (Kv), to compute steady-state pressures.
- **Transient modeling (water hammer).** Modeled fluid deceleration with the Joukowsky equation to simulate pressure surges caused by rapid valve closure (valve opening below 3% at speeds above 2,200 RPM).
- **Safety baseline.** Defined equipment integrity limits: maximum operating pressure of 10.0 bar and maximum pump speed of 3,600 RPM.
- **Prototype simulation script.** Built a standalone Python/NumPy script that numerically solves the pressure response for variable flow rates and setpoints.

## Week 2: Simulation API & Integration Bridge

**Focus:** The digital twin server, the inter-process socket interface, and the verdict logic.

- **Simulation server (`simulation_api.py`).** Developed a socket service listening on `127.0.0.1:5555` that evaluates incoming command setpoints in real time.
- **Bridge interface.** Implemented JSON-based inter-process communication: the service receives the pump RPM and valve position parsed by the network parser and returns a synchronous `ALLOW` or `DROP` verdict.
- **Transient forecasting.** Added a dual-state prediction that returns both the steady-state trajectory and the peak transient pressure (for example, forecasting spikes of 11.80 bar and 20.74 bar for unsafe operations).
- **Hazard classification.** Classified unsafe commands as overpressure, overspeed, or water hammer, each with a human-readable engineering rationale for security forensics.
