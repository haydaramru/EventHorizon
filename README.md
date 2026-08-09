# EventHorizon

**EventHorizon** is an open-source framework for deploying and analyzing multiprotocol IoT tarpits. 
It provides a modular, containerized environment where each protocol emulator runs inside its own Docker container, 
and all telemetry data is collected and visualized through a Prometheus + Grafana stack.

### 🌌 Why the name *EventHorizon*?

<table>
  <tr>
    <td width="30%" align="center">
      <a href="https://en.wikipedia.org/wiki/Event_horizon">
        <img src="https://raw.githubusercontent.com/haydaramru/EventHorizon/GSoC_2026/black-hole.gif" alt="Event Horizon depiction" width="200px" />
      </a>
    </td>
    <td width="70%">
      In astrophysics, the <em>event horizon</em> is the boundary around a black hole beyond which nothing can escape.<br><br>
      Similarly, the <strong>EventHorizon</strong> framework acts as a boundary for malicious network activity: once an automated scanner crosses into it, the connection cannot progress or escape—it becomes trapped indefinitely.<br><br>
      This captures the essence of what the framework does: slowing, containing, and observing automated attacks without letting them spread.
    </td>
  </tr>
</table>

---

## 🚀 How to Run

### 1️⃣ Start all services

The programs are run using Docker. To start all components, simply run:

```bash
docker compose up
```
