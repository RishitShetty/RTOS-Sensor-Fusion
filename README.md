System Architecture (Core RTOS Concepts)
Tasks (prioritized):

High-priority Control Task (e.g., 1kHz or 500Hz).

Sensor Tasks A/B (e.g., 1kHz and 400Hz) driven by hardware timers or software timer callbacks that release semaphores.

Sensor Fusion Task (e.g., 100Hz) consuming from queues.

Logger Task (medium-low) draining a lock-free ring buffer to network.

CLI/Command Task (low) for runtime tuning and diagnostics.

Watchdog/Health Task to monitor deadlines and reset subsystems.

Synchronization:

Queues for sensor data to fusion.

Binary semaphores for ISR-to-task synchronization.

Counting semaphore or event groups for multi-sensor availability.

Mutex with priority inheritance for shared resources (simulate a shared “bus”).

Timing/Determinism:

Use vTaskDelayUntil for periodic tasks.

Use high-resolution timers and record jitter; log histogram.

Memory:

Static task allocation for critical tasks.

Bounded queues and buffers; demonstrate failure/overflow handling.

Fault Tolerance:

Software watchdogs per task; global watchdog.

Panic/recovery path; persistent error counters in NVS.

Connectivity:

Wi‑Fi + TCP server or WebSocket to stream telemetry; simple JSON lines.

OTA + Config:

ESP-IDF OTA example integrated; runtime-configurable rates/priors saved to NVS.

Testing:

Synthetic load task to induce contention.

Priority inversion demo with/without mutex PI.

Automated timing tests: assert jitter bounds and queue latencies.

Unit tests for fusion and control math