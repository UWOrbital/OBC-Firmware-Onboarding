# Firmware Onboarding Challenge - CubeSat Temperature Management

## Overview

This challenge involved developing a peripheral driver for the LM75BD temperature sensor, implementing a thermal management task, and handling OS interrupts. The goal was to provide a deeper understanding of embedded systems, real-time operating systems (FreeRTOS), and interfacing with sensors using the I2C protocol.

## Background

### Embedded Systems

Embedded systems are specialized computing systems designed for specific tasks within larger systems. They often operate in real-time and are embedded into devices like microcontrollers, ensuring efficient and dedicated functionality.

### Real-Time Operating Systems (RTOS)

RTOS is a specialized operating system designed for real-time applications, where timely and predictable responses to events are crucial. FreeRTOS is a popular open-source RTOS used in embedded systems.

### I2C Protocol

I2C (Inter-Integrated Circuit) is a communication protocol commonly used to connect and communicate between integrated circuits. It enables data exchange between devices with a master-slave architecture, allowing efficient sensor integration.

## Completed Tasks

### 1. LM75BD Sensor Driver

- **Implementation:**
  - Developed C functions for the LM75BD sensor driver using the I2C protocol.
  - Referenced the LM75BD datasheet for precise register selection and timing diagrams.

### 2. Thermal Management Task

- **Implementation:**
  - Utilized FreeRTOS for task scheduling and telemetry collection.
  - Ensured consistent temperature monitoring for CubeSat system safety.

### 3. OS Interrupt Handling

- **Implementation:**
  - Designed interrupt functions aligning with LM75BD specifications.
  - Enhanced CubeSat defense against overtemperature scenarios through RTOS mechanisms for the RM46 microcontroller.

