# EventHorizon

**EventHorizon** is an open-source framework for deploying and analyzing multiprotocol IoT tarpits. 
It provides a modular, containerized environment where each protocol emulator runs inside its own Docker container, 
and all telemetry data is collected and visualized through a Prometheus + Grafana stack.

### 🌌 Why the name *EventHorizon*?

<a href="https://en.wikipedia.org/wiki/Event_horizon">
  <img src="https://raw.githubusercontent.com/haydaramru/EventHorizon/GSoC_2026/black-hole.gif" alt="Event Horizon depiction" width="220" align="left" />
</a>

In astrophysics, the *event horizon* is the boundary around a black hole beyond which nothing can escape. 

Similarly, the **EventHorizon** framework acts as a boundary for malicious network activity: once an automated scanner crosses into it, the connection cannot progress or escape—it becomes trapped indefinitely. 

This captures the essence of what the framework does: slowing, containing, and observing automated attacks without letting them spread.

<br clear="all" />

---

## 🚀 How to Run

### 1️⃣ Start all services

The programs are run using Docker. To start all components, simply run:

```bash
docker compose up
```
