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

## Mid-Project Review Deliverables (Physics Core)

- **Constraint enforcement.** Any setpoint above 3,600 RPM, or with predicted pressure above 10.0 bar, is intercepted and flagged `DROP` before it reaches the plant.
- **Digital twin behavior.** Safe commands produce smooth, dynamically calculated operating curves (1.27 bar to 4.24 bar) with no hardcoded outputs or state drift.
- **Latency.** Socket round-trip time was benchmarked at 1.2 ms to 4.5 ms, within the sub-10 ms target for real-time SCADA traffic.

## Summary

| Item | Value |
| --- | --- |
| Service endpoint | 127.0.0.1:5555 (TCP, JSON) |
| Verdicts | ALLOW, DROP |
| Maximum operating pressure | 10.0 bar |
| Maximum pump speed | 3,600 RPM |
| Hazard classes | Overpressure, overspeed, water hammer |
| Round-trip latency | 1.2 ms to 4.5 ms |
