# ESP32 Smart Water Quality Monitoring System 🌊

An IoT-based environmental monitoring platform using ESP32 to measure real-time water quality parameters and transmit data to Firebase and Blynk IoT.

## Features

- pH monitoring
- Turbidity measurement
- TDS measurement
- Temperature & humidity monitoring
- Carbon monoxide detection
- Air quality monitoring
- Cloud dashboard visualization

## System Architecture

![Architecture](Documentation/System_Architecture.png)


## Hardware Components

| Component | Purpose |
|---|---|
| ESP32 | Main controller |
| pH Sensor | Acidity measurement |
| TDS Sensor | Dissolved solids |
| Turbidity Sensor | Water clarity |
| DHT22 | Temperature/Humidity |
| MQ-7 | CO detection |
| MQ-135 | Air quality |


## Software Stack

- Arduino Framework
- ESP32
- Firebase Realtime Database
- Blynk IoT
- C++


## Pin Configuration

| Sensor | GPIO |
|-|-|
| pH | GPIO34 |
| Turbidity | GPIO35 |
| TDS | GPIO32 |
| MQ-7 | GPIO33 |
| MQ-135 | GPIO36 |
| DHT22 | GPIO4 |


## Future Improvements

- Custom PCB design
- Solar powered deployment
- AI-based water quality prediction
- Mobile application

