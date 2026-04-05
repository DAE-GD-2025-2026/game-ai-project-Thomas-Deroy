#pragma once
#include <vector>

#include "NavGraphPathfinding.h"
#include "Movement/Pathfinding/Navmesh/TriPolygon.h"
#include "Shared/Graph/Graph.h"
#include "Shared/Graph/NavGraph/NavGraphNode.h"

namespace GameAI
{
	class SSFA final
	{
	public:
		// 2D cross product (orientation test)
		static float Cross2D(FVector2D const& a, FVector2D const& b)
		{
			return (a.X * b.Y) - (a.Y * b.X);
		}

		static std::vector<NavLine> FindPortals(std::vector<Node*> const & Path, TriPolygon const & NavPoly)
		{
			std::vector<NavLine> Portals = {};
			if (Path.size() < 2) return Portals;

			// Start portal (degenerate)
			FVector2D startPos = Path[0]->GetPosition();
			Portals.push_back(NavLine{ startPos, startPos });

			// Intermediate portals
			for (size_t i = 1; i < Path.size() - 1; ++i)
			{
				auto* pNavNode = static_cast<NavGraphNode*>(Path[i]);
				int edgeIdx = pNavNode->GetEdgeIdx();
				
				if (edgeIdx != -1)
				{
					auto const& edgeIndices = NavPoly.GetEdges()[edgeIdx].EdgeIndices;
					FVector2D p1 = FVector2D(NavPoly.GetVertices()[edgeIndices[0]]);
					FVector2D p2 = FVector2D(NavPoly.GetVertices()[edgeIndices[1]]);

					// Ensure correct left/right orientation
					FVector2D dir = Path[i + 1]->GetPosition() - Path[i - 1]->GetPosition();
					FVector2D portalVector = p2 - p1;

					if (Cross2D(dir, portalVector) < 0.0f)
					{
						std::swap(p1, p2);
					}

					Portals.push_back(NavLine{ p1, p2 });
				}
			}

			// End portal (degenerate)
			FVector2D endPos = Path.back()->GetPosition();
			Portals.push_back(NavLine{ endPos, endPos });

			return Portals;
		}

		static std::vector<FVector2D> OptimizePortals(std::vector<NavLine> const & Portals, TriPolygon const & NavPoly)
		{
			std::vector<FVector2D> Path{};
			if (Portals.empty()) return Path;

			// Initialize funnel
			FVector2D apex = Portals[0].P1;
			Path.push_back(apex);

			int apexIndex = 0;
			int leftLegIndex = 1;
			int rightLegIndex = 1;

			if (Portals.size() <= 1) return Path;

			FVector2D rightLeg = Portals[rightLegIndex].P1 - apex;
			FVector2D leftLeg = Portals[leftLegIndex].P2 - apex;

			for (int i = 1; i < static_cast<int>(Portals.size()); ++i)
			{
				FVector2D newRightLeg = Portals[i].P1 - apex;
				FVector2D newLeftLeg = Portals[i].P2 - apex;

				// Right side check
				if (Cross2D(rightLeg, newRightLeg) >= 0.0f)
				{
					if (Cross2D(leftLeg, newRightLeg) > 0.0f)
					{
						// Left becomes new apex
						apex = apex + leftLeg;
						apexIndex = leftLegIndex;
						Path.push_back(apex);

						i = leftLegIndex;
						leftLegIndex = i;
						rightLegIndex = i;

						if (i + 1 < Portals.size())
						{
							rightLeg = Portals[rightLegIndex + 1].P1 - apex;
							leftLeg = Portals[leftLegIndex + 1].P2 - apex;
						}
						continue;
					}
					else
					{
						// Tighten right
						rightLeg = newRightLeg;
						rightLegIndex = i;
					}
				}

				// Left side check
				if (Cross2D(leftLeg, newLeftLeg) <= 0.0f)
				{
					if (Cross2D(rightLeg, newLeftLeg) < 0.0f)
					{
						// Right becomes new apex
						apex = apex + rightLeg;
						apexIndex = rightLegIndex;
						Path.push_back(apex);

						i = rightLegIndex;
						leftLegIndex = i;
						rightLegIndex = i;

						if (i + 1 < Portals.size())
						{
							rightLeg = Portals[rightLegIndex + 1].P1 - apex;
							leftLeg = Portals[leftLegIndex + 1].P2 - apex;
						}
						continue;
					}
					else
					{
						// Tighten left
						leftLeg = newLeftLeg;
						leftLegIndex = i;
					}
				}
			}

			// Add final point
			Path.push_back(Portals.back().P2);

			return Path;
		}

	private:
		SSFA() {};
		~SSFA() {};
	};
}