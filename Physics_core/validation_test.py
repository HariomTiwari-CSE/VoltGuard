"""
Validates baseline ODE stability and tests rejection of unphysical commands.
"""

from physics_engine import simulate_transient, rpm_to_rads, pump_flow, valve_flow

def run_tests():
    print("========================================")
    print("   VOLTGUARD: PHYSICS CORE VALIDATION   ")
    print("========================================\n")
    
    passed_tests = 0
    total_tests = 4



  
    # Test 1: Standard Baseline Convergence
    print("[TEST 1] Verifying standard convergence at 1500 RPM, 40% Valve...")
    try:
        res = simulate_transient(current_p_pa=1e5, target_rpm=1500.0, valve_pct=40.0, horizon_sec=3.0)
        assert 5.5 <= res["final_pressure_bar"] <= 5.8, "Unexpected settled pressure range"
        print(f"  -> PASS: Settled cleanly at {res['final_pressure_bar']:.2f} bar.")
        passed_tests += 1
    except Exception as e:
        print(f"  -> FAIL: {e}")


  

    # Test 2: Impossible Command - Negative Valve Percentage
    print("\n[TEST 2] Testing rejection of impossible negative valve command (-15%)...")
    try:
        simulate_transient(current_p_pa=1e5, target_rpm=1500.0, valve_pct=-15.0)
        print("  -> FAIL: Engine accepted negative valve position without error.")
    except ValueError as e:
        print(f"  -> PASS: Engine rejected command with message: \"{e}\"")
        passed_tests += 1


  

    # Test 3: Impossible Command - Negative Pump RPM
    print("\n[TEST 3] Testing rejection of impossible negative pump RPM (-500 RPM)...")
    try:
        simulate_transient(current_p_pa=1e5, target_rpm=-500.0, valve_pct=50.0)
        print("  -> FAIL: Engine accepted negative RPM without error.")
    except ValueError as e:
        print(f"  -> PASS: Engine rejected command with message: \"{e}\"")
        passed_tests += 1


  
  
    # Test 4: Extreme Dynamic Range (Zero Valve / Shutoff Pressure)
    print("\n[TEST 4] Testing valve shutoff condition (1500 RPM, 0% Valve)...")
    try:
        res = simulate_transient(current_p_pa=1e5, target_rpm=1500.0, valve_pct=0.0, horizon_sec=1.0)
      
        # Pump deadhead pressure should remain bounded and strictly positive
      
        assert res["peak_pressure_bar"] > 1.0, "Pipe did not pressurize under shutoff"
        print(f"  -> PASS: Deadhead pressure peak detected at {res['peak_pressure_bar']:.2f} bar.")
        passed_tests += 1
    except Exception as e:
        print(f"  -> FAIL: {e}")

    print("\n----------------------------------------")
    print(f"Validation Summary: {passed_tests}/{total_tests} Tests Passed.")
    print("----------------------------------------")



if __name__ == "__main__":
    run_tests()
