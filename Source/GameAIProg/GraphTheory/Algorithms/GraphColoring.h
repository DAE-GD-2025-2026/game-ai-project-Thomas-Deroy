#pragma once
#include <map>
#include <set>
#include "Shared/Graph/Graph.h"

namespace GameAI
{
	class GraphColoring final
	{
	public:
		// Returns NodeId : ColorIndex
		static std::map<int, int> ColorGraph(Graph* const pGraph)
		{
			std::map<int, int> nodeColors;

			// Loop all nodes
			for (Node* pNode : pGraph->GetActiveNodes())
			{
				int nodeId = pNode->GetId();
				std::set<int> usedNeighborColors;

				// Collect neighbor colors
				auto connections = pGraph->FindConnectionsFrom(nodeId);
				for (auto* pConnection : connections)
				{
					int neighborId = pConnection->GetToId();

					if (nodeColors.find(neighborId) != nodeColors.end())
					{
						usedNeighborColors.insert(nodeColors[neighborId]);
					}
				}

				// Find first free color
				int colorIndex = 0;
				while (usedNeighborColors.find(colorIndex) != usedNeighborColors.end())
				{
					colorIndex++;
				}

				// Assign color
				nodeColors[nodeId] = colorIndex;
			}

			return nodeColors;
		}

	private:
		GraphColoring() = default;
		~GraphColoring() = default;
	};
}