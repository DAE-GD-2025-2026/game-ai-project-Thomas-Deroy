#pragma once
#include <stack>
#include "Shared/Graph/Graph.h"

namespace GameAI
{
	enum class Eulerianity
	{
		notEulerian,
		semiEulerian,
		eulerian,
	};

	class EulerianPath final
	{
	public:
		EulerianPath(Graph* const pGraph);

		Eulerianity IsEulerian() const;
		std::vector<Node*> FindPath(Eulerianity& eulerianity) const;

	private:
		void VisitAllNodesDFS(const std::vector<Node*>& pNodes, std::vector<bool>& visited, int startIndex) const;
		bool IsConnected() const;

		Graph* m_pGraph;
	};

	inline EulerianPath::EulerianPath(Graph* const pGraph)
		: m_pGraph(pGraph)
	{
	}

	inline Eulerianity EulerianPath::IsEulerian() const
	{
		// If graph is not connected, then no Eulerian
		if (!IsConnected())
		{
			return Eulerianity::notEulerian;
		}

		int oddDegreeNodeCount = 0;
		std::vector<Node*> Nodes = m_pGraph->GetActiveNodes();

		// Count nodes with odd degree
		for (const Node* node : Nodes)
		{
			// Get connections from this node to find its degree
			int degree = m_pGraph->FindConnectionsFrom(node->GetId()).size();
			
			if (degree % 2 != 0)
			{
				oddDegreeNodeCount++;
			}
		}

		// Eulerian 
		if (oddDegreeNodeCount == 0)
		{
			return Eulerianity::eulerian;
		}
		// Semi-Eulerian 
		else if (oddDegreeNodeCount == 2)
		{
			return Eulerianity::semiEulerian;
		}

		// No Eulerian 
		return Eulerianity::notEulerian;
	}

inline std::vector<Node*> EulerianPath::FindPath(Eulerianity& eulerianity) const
	{
		// Get a copy
		Graph graphCopy = m_pGraph->Clone();
		std::vector<Node*> Path = {};
		std::vector<Node*> Nodes = graphCopy.GetActiveNodes();
		int currentNodeId{ Graphs::InvalidNodeId };
		
		// Check if euler path
		eulerianity = IsEulerian();
		
		// If graph is not eulerian
		if (eulerianity == Eulerianity::notEulerian || Nodes.empty())
		{
			return Path;
		}
		
		// Choose a starting node 
		if (eulerianity == Eulerianity::eulerian)
		{
			currentNodeId = Nodes[0]->GetId(); 
		}
		else if (eulerianity == Eulerianity::semiEulerian)
		{
			// Start at node with odd degree
			for (const Node* n : Nodes)
			{
				if (graphCopy.FindConnectionsFrom(n->GetId()).size() % 2 != 0)
				{
					currentNodeId = n->GetId();
					break;
				}
			}
		}
		
		std::stack<int> nodeStack;

		// Repeat until the current node has no more connections and stack is empty 
		while (graphCopy.FindConnectionsFrom(currentNodeId).size() > 0 || !nodeStack.empty())
		{
			auto connections = graphCopy.FindConnectionsFrom(currentNodeId);
			
			if (connections.size() > 0)
			{
				// Save current node
				nodeStack.push(currentNodeId);
				
				// Take neighbors 
				int neighborId = connections[0]->GetToId();
				
				graphCopy.RemoveConnection(currentNodeId, neighborId);
				currentNodeId = neighborId;
			}
			else
			{
				// Add the last current node to the path
				Path.push_back(m_pGraph->GetNode(currentNodeId).get());
				
				currentNodeId = nodeStack.top();
				nodeStack.pop();
			}
		}
		
		// Add final node 
		Path.push_back(m_pGraph->GetNode(currentNodeId).get());

		// Path was built backwards
		std::reverse(Path.begin(), Path.end());
		return Path;
	}

	inline void EulerianPath::VisitAllNodesDFS(const std::vector<Node*>& Nodes, std::vector<bool>& visited, int startIndex) const
	{
		// Mark node as visited 
		visited[startIndex] = true;

		// Iterate over all connections from that node 
		auto connections = m_pGraph->FindConnectionsFrom(Nodes[startIndex]->GetId());
		
		for (size_t i = 0; i < connections.size(); ++i)
		{
			int toId = connections[i]->GetToId();
			int toIndex = -1;

			// Find note index
			for (size_t j = 0; j < Nodes.size(); ++j)
			{
				if (Nodes[j]->GetId() == toId)
				{
					toIndex = j;
					break;
				}
			}

			// Visit neighbor if not visited
			if (toIndex != -1 && !visited[toIndex])
			{
				VisitAllNodesDFS(Nodes, visited, toIndex);
			}
		}
	}

	inline bool EulerianPath::IsConnected() const
	{
		std::vector<Node*> Nodes = m_pGraph->GetActiveNodes();
		if (Nodes.size() == 0)
			return false;

		std::vector<bool> visited(Nodes.size(), false);
		int startIndex = -1;

		// Choose a starting node 
		for (size_t i = 0; i < Nodes.size(); ++i)
		{
			if (m_pGraph->FindConnectionsFrom(Nodes[i]->GetId()).size() > 0)
			{
				startIndex = i;
				break;
			}
		}

		// Handle graphs/nodes without connections
		if (startIndex == -1 && Nodes.size() > 1) return false;
		if (startIndex == -1) return true; 

		// Run DFS
		VisitAllNodesDFS(Nodes, visited, startIndex);

		// Check if all nodes were reached
		for (size_t i = 0; i < Nodes.size(); ++i)
		{
			if (!visited[i] && Nodes[i]->GetId() != Graphs::InvalidNodeId)
			{
				return false;
			}
		}
		
		return true;
	}
}