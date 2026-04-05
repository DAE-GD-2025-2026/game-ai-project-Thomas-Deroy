#include "AStar.h"
#include <algorithm>
#include <limits>

using namespace GameAI;

AStar::AStar(Graph* const pGraph, HeuristicFunctions::Heuristic hFunction)
	: pGraph(pGraph)
	, HeuristicFunction(hFunction)
{
}

std::vector<Node*> AStar::FindPath(Node* const pStartNode, Node* const pDestinationNode)
{
	std::vector<Node*> path{};

	// If start or destination is missing, stop
	if (!pStartNode || !pDestinationNode)
	{
		return path;
	}

	// Lists used by A*
	std::vector<NodeRecord> openList;   // Nodes to explore
	std::vector<NodeRecord> closedList; // Already processed nodes
	NodeRecord currentRecord;

	// Initialize start node
	NodeRecord startRecord;
	startRecord.pNode = pStartNode;
	startRecord.pConnection = nullptr;
	startRecord.costSoFar = 0.f;
	startRecord.estimatedTotalCost = GetHeuristicCost(pStartNode, pDestinationNode);
	
	openList.push_back(startRecord);

	// Main A* loop
	while (!openList.empty())
	{
		// Get node with lowest estimated cost
		auto bestRecordIt = std::min_element(openList.begin(), openList.end());
		currentRecord = *bestRecordIt;

		// Stop if goal reached
		if (currentRecord.pNode == pDestinationNode)
		{
			break; 
		}

		// Check all neighbors
		auto connections = pGraph->FindConnectionsFrom(currentRecord.pNode->GetId());
		for (Connection* connection : connections)
		{
			// Get the neighbor node
			Node* pNextNode = pGraph->GetNode(connection->GetToId()).get();

			// Calculate new G-cost
			float totalGCost = currentRecord.costSoFar + connection->GetWeight();

			// Check if node is already in closed list
			auto closedIt = std::find_if(closedList.begin(), closedList.end(), 
				[pNextNode](const NodeRecord& r) { return r.pNode == pNextNode; });
			
			if (closedIt != closedList.end())
			{
				// Skip if existing path is cheaper
				if (closedIt->costSoFar <= totalGCost)
				{
					continue;
				}
				else
				{
					closedList.erase(closedIt);
				}
			}

			// Check if node is already in open list
			auto openIt = std::find_if(openList.begin(), openList.end(), 
				[pNextNode](const NodeRecord& r) { return r.pNode == pNextNode; });
			
			if (openIt != openList.end())
			{
				// Skip if existing path is cheaper
				if (openIt->costSoFar <= totalGCost)
				{
					continue;
				}
				else
				{
					openList.erase(openIt);
				}
			}

			// Create a new record for this path
			NodeRecord newRecord;
			newRecord.pNode = pNextNode;
			newRecord.pConnection = connection;
			newRecord.costSoFar = totalGCost;
			newRecord.estimatedTotalCost = totalGCost + GetHeuristicCost(pNextNode, pDestinationNode);
			
			openList.push_back(newRecord);
		}

		// Move current node from open to closed list
		openList.erase(std::remove(openList.begin(), openList.end(), currentRecord), openList.end());
		closedList.push_back(currentRecord);
	}

	// If destination wasn't reached, find a fallback path
	if (currentRecord.pNode != pDestinationNode)
	{
		float closestHeuristic = std::numeric_limits<float>::max();

		for (const auto& record : closedList)
		{
			float heuristicToGoal = GetHeuristicCost(record.pNode, pDestinationNode);

			if (heuristicToGoal < closestHeuristic)
			{
				closestHeuristic = heuristicToGoal;
				currentRecord = record; // backtrack from closest node
			}
		}

		// Safety check
		if (currentRecord.pNode == nullptr)
		{
			return path;
		}
	}

	// Reconstruct path by walking backwards
	while (currentRecord.pNode != pStartNode)
	{
		path.push_back(currentRecord.pNode);

		// Find the parent node in closed list
		int parentNodeId = currentRecord.pConnection->GetFromId();
		auto parentIt = std::find_if(closedList.begin(), closedList.end(), 
			[parentNodeId](const NodeRecord& r) { return r.pNode->GetId() == parentNodeId; });

		// Move to the parent record
		if (parentIt != closedList.end())
		{
			currentRecord = *parentIt;
		}
	}

	// Add start node
	path.push_back(pStartNode);

	// Reverse because path was built backwards
	std::reverse(path.begin(), path.end());

	return path;
}

float AStar::GetHeuristicCost(Node* const pStartNode, Node* const pEndNode) const
{
	// Distance from start to destination
	FVector2D toDestination = pGraph->GetNode(pEndNode->GetId())->GetPosition() - pGraph->GetNode(pStartNode->GetId())->GetPosition();
	// Apply the selected heuristic
	return HeuristicFunction(abs(toDestination.X), abs(toDestination.Y));
}