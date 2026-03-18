# Unreal ArchViz Interaction System

## Overview

A modular interaction system built in Unreal Engine (C++) for ArchViz use cases, focused on scalable object interaction and real-time customization.
It currently includes a wall customization setup, and the system is designed to scale to other interactable objects like furniture.

---

## Core Idea

Uses an **interface-based system (`I_Interact`)** that any actor can implement.

* Currently implemented by **WallActor**
* Easily extendable to other actors (chairs, tables, etc.)

  * Example: switch models, materials, or configurations

---

## Features

* **Line Trace Interaction**

  * Detects interactable actors from camera
  * Triggers interface functions on input

* **Wall Customization**

  * Spawns a focused camera
  * Opens UI for real-time color change (Dynamic Materials)

* **Camera & UI Handling**

  * Smooth camera switching (`SetViewTargetWithBlend`)
  * Game ↔ UI input mode switching

---

## Tech

* Unreal Engine 5
* C++ + Blueprints
* UMG

---

