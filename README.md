# EventHorizon

**EventHorizon** is an open-source framework for deploying and analyzing multiprotocol IoT tarpits.  
It provides a modular, containerized environment where each protocol emulator runs inside its own Docker container, 
and all telemetry data is collected and visualized through a Prometheus + Grafana stack.

### 🌌 Why the name *EventHorizon*?
<a href="https://en.wikipedia.org/wiki/Optical_illusion">   
<img 
  src="https://raw.github.com/haydaramru/EventHorizon/GSoC_2026/black-hole.gif" 
  alt="Event Horizon depiction"
  style="margin-top:20px;margin-right:20px"
  align="left" 
  height="200px"
/>
</a>
&nbsp;&nbsp;&nbsp;
In astrophysics, the *event horizon* is the boundary around a black hole beyond which nothing can escape.  
Similarly, the **EventHorizon** framework acts as a boundary for malicious network activity:  
once an automated scanner crosses into it, the connection cannot progress or escape—it becomes trapped indefinitely.  
This captures the essence of what the framework does: slowing, containing, and observing automated attacks without letting them spread.

---



## 🚀 How to Run

### 1️⃣ Start all services
The programs are run using Docker.  
To start all components, simply run:

```bash
docker compose up
```


