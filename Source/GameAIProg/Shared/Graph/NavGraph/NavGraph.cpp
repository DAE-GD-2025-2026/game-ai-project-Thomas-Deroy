#include "NavGraph.h"
#include "NavGraphNode.h"
#include <map>
#include <set>

using namespace GameAI;

NavGraph::NavGraph(std::unique_ptr<TriPolygon> && NavPoly)
    : Graph{false}
    , pNavPoly{std::move(NavPoly)}
{
    CreateNavigationGraph();
}

NavGraph::NavGraph(const NavGraph& Other)
    : Graph(false)
{
    // Copy nodes
    Nodes.reserve(Other.Nodes.size());
    for (std::unique_ptr<Node> const & OtherNode : Other.Nodes)
    {
       Nodes.push_back(std::make_unique<NavGraphNode>(*dynamic_cast<NavGraphNode*>(OtherNode.get())));
    }
        
    // Copy connections
    Connections.reserve(Other.Connections.size());
    for (std::unique_ptr<Connection> const & OtherConnection : Other.Connections)
    {
       Connections.push_back(std::make_unique<Connection>(*OtherConnection.get()));
    }
}

std::unique_ptr<NavGraph> NavGraph::Clone() const
{
    return std::make_unique<NavGraph>(*this);
}

int NavGraph::GetNodeIdFromEdgeIndex(int EdgeIdx) const
{
    if (EdgeIdx >= 0)
    {
       for (auto const & pNode : Nodes)
       {
          if (reinterpret_cast<NavGraphNode*>(pNode.get())->GetEdgeIdx() == EdgeIdx)
          {
             return pNode->GetId();
          }
       }
    }
    
    return Graphs::InvalidNodeId;
}

int NavGraph::GetNodeIdAtPosition(FVector2D const& Position) const
{
    // Find triangle at position
    auto const* pTriangle = pNavPoly->GetTriangleAtPosition(Position, true);
    if (pTriangle != nullptr)
    {
        // Return first valid portal node
        for (auto const& edge : pTriangle->GetEdges())
        {
            int edgeIdx = pNavPoly->FindEdgeIndex(edge).value_or(-1);
            int nodeId = GetNodeIdFromEdgeIndex(edgeIdx);
            if (nodeId != Graphs::InvalidNodeId)
            {
                return nodeId;
            }
        }
    }

    return Graphs::InvalidNodeId;
}

void NavGraph::CreateNavigationGraph()
{
    auto const& triangles = pNavPoly->GetTriangles();
    std::map<int, int> edgeUsage;

    // Count how many triangles share each edge
    for (auto const& triangle : triangles)
    {
        for (auto const& edge : triangle.GetEdges())
        {
            int edgeIdx = pNavPoly->FindEdgeIndex(edge).value_or(-1);
            if (edgeIdx != -1)
            {
                edgeUsage[edgeIdx]++;
            }
        }
    }

    std::set<int> createdEdges;

    // Create nodes and connections
    for (auto const& triangle : triangles)
    {
        auto vertices = triangle.GetVertices(*pNavPoly);
        auto edges = triangle.GetEdges();

        std::vector<int> validNodeIds;

        // Check triangle edges
        for (size_t i = 0; i < edges.size(); ++i)
        {
            int edgeIdx = pNavPoly->FindEdgeIndex(edges[i]).value_or(-1);

            // Shared edge = portal
            if (edgeIdx != -1 && edgeUsage[edgeIdx] > 1)
            {
                // Create node once per edge
                if (createdEdges.find(edgeIdx) == createdEdges.end())
                {
                    FVector v0 = vertices[i];
                    FVector v1 = vertices[(i + 1) % 3];
                    FVector2D centerPos = FVector2D((v0.X + v1.X) / 2.0f, (v0.Y + v1.Y) / 2.0f);

                    auto pNode = std::make_unique<NavGraphNode>(centerPos, edgeIdx);
                    AddNode(std::move(pNode));
                    createdEdges.insert(edgeIdx);
                }

                int nodeId = GetNodeIdFromEdgeIndex(edgeIdx);
                if (nodeId != Graphs::InvalidNodeId)
                {
                    validNodeIds.push_back(nodeId);
                }
            }
        }

        // Connect portals inside triangle
        if (validNodeIds.size() == 2)
        {
            AddConnection(std::make_unique<Connection>(validNodeIds[0], validNodeIds[1]));
            AddConnection(std::make_unique<Connection>(validNodeIds[1], validNodeIds[0]));
        }
        else if (validNodeIds.size() == 3)
        {
            AddConnection(std::make_unique<Connection>(validNodeIds[0], validNodeIds[1]));
            AddConnection(std::make_unique<Connection>(validNodeIds[1], validNodeIds[0]));
            
            AddConnection(std::make_unique<Connection>(validNodeIds[1], validNodeIds[2]));
            AddConnection(std::make_unique<Connection>(validNodeIds[2], validNodeIds[1]));
            
            AddConnection(std::make_unique<Connection>(validNodeIds[2], validNodeIds[0]));
            AddConnection(std::make_unique<Connection>(validNodeIds[0], validNodeIds[2]));
        }
    }
    
    // Set connection costs
    SetConnectionCostsToDistances();
}