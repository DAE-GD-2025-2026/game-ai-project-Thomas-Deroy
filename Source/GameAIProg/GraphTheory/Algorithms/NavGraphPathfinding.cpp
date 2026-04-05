#include "NavGraphPathfinding.h"

#include "AStar.h"
#include "PathSmoothing.h"
#include "VectorTypes.h"
#include "Shared/Graph/NavGraph/NavGraph.h"
#include "Shared/Graph/NavGraph/NavGraphNode.h"

using namespace GameAI;

std::vector<FVector2D> NavMeshPathfinding::FindPath(const FVector2D& startPos, const FVector2D& endPos,
    NavGraph* const pNavGraph, std::vector<FVector2D>& debugNodePositions, std::vector<NavLine>& debugPortals) 
{
    // Path result
    std::vector<FVector2D> finalPath{};

    // Get start and end triangles
    auto const* pStartTriangle = pNavGraph->GetNavPolygon()->GetTriangleAtPosition(startPos, true);
    auto const* pEndTriangle = pNavGraph->GetNavPolygon()->GetTriangleAtPosition(endPos, true);

    // No valid path if outside navmesh
    if (pStartTriangle == nullptr || pEndTriangle == nullptr)
        return finalPath;

    // Same triangle -> straight line
    if (pStartTriangle == pEndTriangle)
    {
        finalPath.push_back(startPos);
        finalPath.push_back(endPos);
        return finalPath;
    }

    // Clone graph to modify safely
    auto pCloneGraph = pNavGraph->Clone();

    // Create start node
    auto pStartNode = std::make_unique<NavGraphNode>(startPos, -1); 
    auto pRawStartNode = pStartNode.get();
    
    pCloneGraph->AddNode(std::move(pStartNode));
    int startNodeId = pRawStartNode->GetId();

    // Connect start node to triangle portals
    for (auto const& edge : pStartTriangle->GetEdges())
    {
        int edgeIdx = pNavGraph->GetNavPolygon()->FindEdgeIndex(edge).value_or(-1);
        int nodeId = pCloneGraph->GetNodeIdFromEdgeIndex(edgeIdx);
        
        if (nodeId != Graphs::InvalidNodeId)
            pCloneGraph->AddConnection(std::make_unique<Connection>(startNodeId, nodeId));
    }

    // Create end node
    auto pEndNode = std::make_unique<NavGraphNode>(endPos, -1);
    auto pRawEndNode = pEndNode.get();
    
    pCloneGraph->AddNode(std::move(pEndNode));
    int endNodeId = pRawEndNode->GetId();

    // Connect triangle portals to end node
    for (auto const& edge : pEndTriangle->GetEdges())
    {
        int edgeIdx = pNavGraph->GetNavPolygon()->FindEdgeIndex(edge).value_or(-1);
        int nodeId = pCloneGraph->GetNodeIdFromEdgeIndex(edgeIdx);
        
        if (nodeId != Graphs::InvalidNodeId)
            pCloneGraph->AddConnection(std::make_unique<Connection>(nodeId, endNodeId));
    }

    // Update connection costs
    pCloneGraph->SetConnectionCostsToDistances();

    // Run A*
    AStar pathfinder(pCloneGraph.get(), HeuristicFunctions::Euclidean);
    std::vector<Node*> nodePath = pathfinder.FindPath(
        pCloneGraph->GetNode(startNodeId).get(),
        pCloneGraph->GetNode(endNodeId).get());

    // No path found
    if (nodePath.empty())
        return finalPath;

    // Convert nodes to positions
    for (Node* pNode : nodePath)
    {
        finalPath.push_back(pNode->GetPosition());
        debugNodePositions.push_back(pNode->GetPosition());
    }

    // Smooth path
    debugPortals = SSFA::FindPortals(nodePath, *pNavGraph->GetNavPolygon());
    finalPath = SSFA::OptimizePortals(debugPortals, *pNavGraph->GetNavPolygon());
    
    return finalPath;
}

std::vector<FVector2D> NavMeshPathfinding::FindPath(const FVector2D& startPos, const FVector2D& endPos, NavGraph* const pNavGraph)
{
    std::vector<FVector2D> debugNodePositions{};
    std::vector<NavLine> debugPortals{};

    return FindPath(startPos, endPos, pNavGraph, debugNodePositions, debugPortals);
}