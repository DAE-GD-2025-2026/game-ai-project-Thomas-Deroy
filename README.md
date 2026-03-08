# Game AI Project - Algorithms 2

## Overview
This repository contains implementations of AI steering behaviors, a Boids flocking simulation, and performance optimizations using spatial partitioning.

## Features

### 1. Steering Behaviors
* **Seek / Flee:** Move directly towards or away from a target.
* **Arrive:** Move towards a target and gradually slow down as it enters a defined radius.
* **Face:** Rotate the agent to look directly at a target.
* **Pursuit / Evade:** Steer towards or away from a target's predicted future position based on its velocity.
* **Wander:** Generate randomized movement by targeting a point on a circle in front of the agent.

### 2. Combined Steering & Flocking
* **Blended Steering:** Combines multiple behaviors by calculating a weighted average of their outputs.
* **Priority Steering:** Evaluates a sequence of behaviors and selects the first valid steering output.
* **Flocking (Boids):** Simulates group movement using three blended rules:
  * *Separation:* Avoid colliding with nearby neighbors.
  * *Cohesion:* Steer towards the average position of local neighbors.
  * *Alignment:* Match the average velocity of local neighbors.

### 3. Algorithmic Optimizations
* **Spatial Partitioning:** Divides the 2D world into a uniform grid of cells. This reduces the $O(n^{2})$ complexity of neighbor-checking by only querying agents within the current and adjacent cells .
* **Memory Pooling:** Utilizes a fixed-size container to store agent neighborhood records, avoiding performance-heavy memory allocations and fragmentation during the simulation loop.
