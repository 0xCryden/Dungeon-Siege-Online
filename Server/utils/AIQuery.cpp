#include "AIQuery.h"

bool AIQuery :: IsInRange (const Go * a, const Go * b, double range)
{
	//cout << "IsInRange begin" << endl;

	if (a == NULL || b == NULL)
	{
		return false;
	}
	
	if (a->HasPlacement() != true || b->HasPlacement() != true)
	{
		return false;
	}
	
	if (a->Placement()->GetRegion() != b->Placement()->GetRegion())
	{
		return false;
	}
	
	if (a->Placement()->GetRegion().empty())
	{
		return false;
	}
	
	Region * region = g_world.GetRegion (a->Placement()->GetRegion());
	double dist = region->GetSiegeDistance (a->Placement()->Position(), b->Placement()->Position());
	//cout << a->Goid() << " is in range of " << b->Goid() << " | Range: " << (double)dist << endl;
	return range >= dist;
}
