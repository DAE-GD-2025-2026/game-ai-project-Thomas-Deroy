#include "SpacePartitioning.h"

// --- Cell ---
// ------------
Cell::Cell(float Left, float Bottom, float Width, float Height)
{
	BoundingBox.Min = { Left, Bottom };
	BoundingBox.Max = { BoundingBox.Min.X + Width, BoundingBox.Min.Y + Height };
}

std::vector<FVector2D> Cell::GetRectPoints() const
{
	const float left = BoundingBox.Min.X;
	const float bottom = BoundingBox.Min.Y;
	const float width = BoundingBox.Max.X - BoundingBox.Min.X;
	const float height = BoundingBox.Max.Y - BoundingBox.Min.Y;

	// return 4 corner points 
	return {
		{ left, bottom },
		{ left, bottom + height },
		{ left + width, bottom + height },
		{ left + width, bottom }
	};
}

// --- Partitioned Space ---
// -------------------------
CellSpace::CellSpace(UWorld* pWorld, float Width, float Height, int Rows, int Cols, int MaxEntities)
	: pWorld{pWorld}
	, SpaceWidth{Width}
	, SpaceHeight{Height}
	, NrOfRows{Rows}
	, NrOfCols{Cols}
	, NrOfNeighbors{0}
{
	Neighbors.SetNum(MaxEntities);

	CellWidth = SpaceWidth / NrOfCols;
	CellHeight = SpaceHeight / NrOfRows;

	// world centered at (0,0)
	CellOrigin = FVector2D(-SpaceWidth / 2.f, -SpaceHeight / 2.f);

	// create grid cells
	for (int r = 0; r < NrOfRows; ++r)
	{
		for (int c = 0; c < NrOfCols; ++c)
		{
			float left = CellOrigin.X + (c * CellWidth);
			float bottom = CellOrigin.Y + (r * CellHeight);
			Cells.emplace_back(left, bottom, CellWidth, CellHeight);
		}
	}
}

void CellSpace::AddAgent(ASteeringAgent& Agent)
{
	int index = PositionToIndex(Agent.GetPosition());
	Cells[index].Agents.push_back(&Agent);
}

void CellSpace::UpdateAgentCell(ASteeringAgent& Agent, const FVector2D& OldPos)
{
	int oldIndex = PositionToIndex(OldPos);
	int newIndex = PositionToIndex(Agent.GetPosition());

	// move only if cell changed
	if (oldIndex != newIndex)
	{
		Cells[oldIndex].Agents.remove(&Agent);
		Cells[newIndex].Agents.push_back(&Agent);
	}
}

void CellSpace::RegisterNeighbors(ASteeringAgent& Agent, float QueryRadius)
{
	NrOfNeighbors = 0;

	const FVector2D agentPos = Agent.GetPosition();
	const float queryRadiusSq = QueryRadius * QueryRadius;

	// query area around agent
	FRect queryBox;
	queryBox.Min = { agentPos.X - QueryRadius, agentPos.Y - QueryRadius };
	queryBox.Max = { agentPos.X + QueryRadius, agentPos.Y + QueryRadius };

	for (const Cell& cell : Cells)
	{
		// skip cells outside query box
		if (!DoRectsOverlap(queryBox, cell.BoundingBox))
			continue;

		for (ASteeringAgent* pOther : cell.Agents)
		{
			if (pOther == &Agent) continue; // ignore self

			// precise circle check
			if (FVector2D::DistSquared(agentPos, pOther->GetPosition()) < queryRadiusSq)
			{
				if (NrOfNeighbors < Neighbors.Num())
				{
					Neighbors[NrOfNeighbors++] = pOther;
				}
			}
		}
	}
}

void CellSpace::EmptyCells()
{
	// clear all agents from grid
	for (Cell& c : Cells)
		c.Agents.clear();
}

void CellSpace::RenderCells() const
{
	for (const Cell& cell : Cells)
	{
		const auto points = cell.GetRectPoints();

		FVector p0(points[0].X, points[0].Y, 0.f);
		FVector p1(points[1].X, points[1].Y, 0.f);
		FVector p2(points[2].X, points[2].Y, 0.f);
		FVector p3(points[3].X, points[3].Y, 0.f);

		// draw cell border
		DrawDebugLine(pWorld, p0, p1, FColor::Red, false, -1.f, 0, 5.f);
		DrawDebugLine(pWorld, p1, p2, FColor::Red, false, -1.f, 0, 5.f);
		DrawDebugLine(pWorld, p2, p3, FColor::Red, false, -1.f, 0, 5.f);
		DrawDebugLine(pWorld, p3, p0, FColor::Red, false, -1.f, 0, 5.f);

		// show agent count in center
		FVector center(
			(cell.BoundingBox.Min.X + cell.BoundingBox.Max.X) * 0.5f,
			(cell.BoundingBox.Min.Y + cell.BoundingBox.Max.Y) * 0.5f,
			0.5f
		);

		DrawDebugString(
			pWorld,
			center,
			FString::FromInt(static_cast<int>(cell.Agents.size())),
			nullptr,
			FColor::White,
			0.f,
			false,
			1.2f
		);
	}
}

int CellSpace::PositionToIndex(FVector2D const& Pos) const
{
	// convert world pos to grid space
	float relativeX = Pos.X - CellOrigin.X;
	float relativeY = Pos.Y - CellOrigin.Y;

	int col = static_cast<int>(relativeX / CellWidth);
	int row = static_cast<int>(relativeY / CellHeight);

	// clamp inside grid
	col = FMath::Clamp(col, 0, NrOfCols - 1);
	row = FMath::Clamp(row, 0, NrOfRows - 1);

	return (row * NrOfCols) + col;
}

bool CellSpace::DoRectsOverlap(FRect const& RectA, FRect const& RectB)
{
	// separated on X
	if (RectA.Max.X < RectB.Min.X || RectA.Min.X > RectB.Max.X)
		return false;

	// separated on Y
	if (RectA.Max.Y < RectB.Min.Y || RectA.Min.Y > RectB.Max.Y)
		return false;

	return true;
}