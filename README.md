# Game AI Project - Algorithms 2

> [!IMPORTANT]
> **HOW TO RUN IT?** You right click on the .uproject and open it with **Rider**, Otherwise it wouldn't run.

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

### 4. Graph Theory
* **Graph Representations:** Implements the foundation for defining and visualizing relationships between components using Nodes (Vertices) and Connections (Edges). The project utilizes an Edge List notation to represent both directed and undirected graphs.
* **Eulerian Paths:** Features algorithms to determine graph Eulerianity (whether a path can traverse every connection exactly once). It uses Depth-First Search (DFS) to verify graph connectivity before constructing the final Eulerian trail or cycle.

### 5. Pathfinding Algorithms
* **Breadth-First Search (BFS):** An uninformed search algorithm that explores the graph level-by-level using a queue. It guarantees finding the optimal path in unweighted graphs.
* **A\* Search (A-Star):** An informed search algorithm that combines the best aspects of Dijkstra and Greedy Best-First-Search. It uses a heuristic function (estimated cost to the goal) combined with the actual travel cost to efficiently calculate the shortest path.

### 6. Navigation Meshes
* **NavGraph Generation:** Converts an abstraction of walkable space (triangulated polygons) into a traversable graph structure. Nodes are placed in the middle of connecting triangle edges to allow for pathfinding.
* **Path Smoothing:** Since raw A\* paths on a navmesh jump between the center of edges, the **Simple Stupid Funnel Algorithm (SSFA)** is used to optimize the path. It acts like "string pulling" to generate a smoother route from the start to the goal.
