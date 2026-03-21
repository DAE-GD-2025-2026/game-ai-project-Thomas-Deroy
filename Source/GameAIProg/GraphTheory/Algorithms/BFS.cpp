#include "BFS.h"

#include <map>
#include <queue>
#include <algorithm> 

#include "Shared/Graph/Graph.h"

using namespace GameAI;

BFS::BFS(Graph* const pGraph)
	: pGraph(pGraph)
{
}

// Breadth First Search
std::vector<Node*> BFS::FindPath(Node* const pStartNode, Node* const pDestinationNode) const
{
	std::vector<Node*> path;

	// If start or destination missing then no path
	if (!pStartNode || !pDestinationNode)
	{
		return path;
	}

	// Data needed
	std::queue<Node*> openList;       // Nodes to explore
	std::map<Node*, bool> visited;    // Track visited nodes
	std::map<Node*, Node*> parentMap; // Stores where each node came from
	
	openList.push(pStartNode);
	visited[pStartNode] = true; 

	bool bFoundDestination = false;

	// If there are nodes keep going
	while (!openList.empty())
	{
		// Get next node to process
		Node* currentNode = openList.front();
		openList.pop();

		// If reached goal then stop
		if (currentNode == pDestinationNode)
		{
			bFoundDestination = true;
			break;
		}

		// Check all neighbors of current node
		auto connections = pGraph->FindConnectionsFrom(currentNode->GetId());
		for (auto* connection : connections)
		{
			// Get neighbor node
			Node* neighborNode = pGraph->GetNode(connection->GetToId()).get();

			// If not visited yet
			if (!visited[neighborNode])
			{
				visited[neighborNode] = true;          // Mark visited
				parentMap[neighborNode] = currentNode; // Remember how we got here
				openList.push(neighborNode);           // Explore it later
			}
		}
	}

	// Rebuild path if goal was found
	if (bFoundDestination)
	{
		Node* current = pDestinationNode; // Start from goal
		
		// Walk back to start using the parent map
		while (current != pStartNode)
		{
			path.push_back(current);
			current = parentMap[current];
		}
		
		// Add start node
		path.push_back(pStartNode); 

		// Reverse because built it backwards
		std::reverse(path.begin(), path.end());
	}

	return path; // Empty if no path found
}