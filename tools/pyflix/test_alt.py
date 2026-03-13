import time
import sys
from flix import Flix
from pymavlink import mavutil


def log(msg):
    ts = time.strftime("%H:%M:%S")
    print(f"[{ts}] {msg}", flush=True)


class MavMonitor:
    def __init__(self, udp="udp:0.0.0.0:14550"):
        self.udp = udp
        self.m = None
        self.last_hb = 0.0
        self.sysid = None
        self.compid = None

    def connect(self, timeout=8.0):
        log(f"Opening MAVLink UDP listener on {self.udp} ...")
        self.m = mavutil.mavlink_connection(self.udp, autoreconnect=True)
        t0 = time.time()
        while time.time() - t0 < timeout:
            msg = self.m.recv_match(type="HEARTBEAT", blocking=False)
            if msg:
                self.last_hb = time.time()
                self.sysid = self.m.target_system
                self.compid = self.m.target_component
                log(f"Heartbeat OK (sys={self.sysid}, comp={self.compid})")
                return True
            time.sleep(0.1)
        return False

    def poll(self, max_msgs=20):
        if not self.m:
            return
        for _ in range(max_msgs):
            msg = self.m.recv_match(blocking=False)
            if not msg:
                break
            if msg.get_type() == "HEARTBEAT":
                self.last_hb = time.time()

    def heartbeat_alive(self, max_age_sec=2.5):
        return (time.time() - self.last_hb) <= max_age_sec


def main():
    log("Starting test_alt_auto.py")

    log("Connecting to drone via pyflix (Flix)...")
    try:
        f = Flix()
    except Exception as e:
        log(f"ERROR: Could not create Flix() connection: {e}")
        sys.exit(1)

    try:
        log(
            f"pyflix connected={getattr(f, 'connected', 'unknown')} "
            f"mode={getattr(f, 'mode', 'unknown')} "
            f"armed={getattr(f, 'armed', 'unknown')} "
            f"landed={getattr(f, 'landed', 'unknown')}"
        )
    except Exception:
        pass

    mon = MavMonitor("udp:0.0.0.0:14550")
    if mon.connect(timeout=8.0):
        log("MAVLink link looks alive ✅")
    else:
        log("WARNING: No MAVLink heartbeat seen on udp:14550.")

    def run_cli(cmd):
        log(f"> CLI: {cmd}")
        try:
            out = f.cli(cmd)
            if out is not None and str(out).strip() != "":
                log(f"< {out}")
        except Exception as e:
            log(f"ERROR running cli('{cmd}'): {e}")
            raise

    log("Preflight telemetry (2s)...")
    t0 = time.time()
    while time.time() - t0 < 2.0:
        mon.poll()
        try:
            log(f"state: mode={f.mode} armed={f.armed} landed={f.landed}")
        except Exception:
            pass
        time.sleep(0.5)

    run_cli("auto")
    time.sleep(0.3)

    run_cli("althold 1")
    time.sleep(0.2)

    try:
        run_cli("baro")
    except Exception:
        log("baro command not available")

    # automatic start delay so you have a few seconds after running the script
    log("Auto takeoff starts in 5 seconds...")
    time.sleep(5)

    run_cli("arm")
    time.sleep(1.0)

    run_cli("alt 0.30")
    log("Target altitude set to 0.30m. Monitoring for 3 seconds...")

    t0 = time.time()
    while time.time() - t0 < 3.0:
        mon.poll()
        alive = mon.heartbeat_alive()
        try:
            log(f"alive={alive} mode={f.mode} armed={f.armed} landed={f.landed}")
        except Exception:
            log(f"alive={alive}")
        time.sleep(0.5)

    run_cli("alt 0.00")
    log("Landing requested. Monitoring for 5 seconds...")

    t0 = time.time()
    while time.time() - t0 < 5.0:
        mon.poll()
        alive = mon.heartbeat_alive()
        try:
            log(f"alive={alive} mode={f.mode} armed={f.armed} landed={f.landed}")
        except Exception:
            log(f"alive={alive}")
        time.sleep(0.5)

    run_cli("disarm")
    log("DISARM sent. Done ✅")


if __name__ == "__main__":
    main()