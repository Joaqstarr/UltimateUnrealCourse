#pragma once
#include "DrawDebugHelpers.h"

#define DRAW_SPHERE(Location) if(GetWorld()) DrawDebugSphere(GetWorld(), Location, 25.0f, 12, FColor::Magenta, true);
#define DRAW_SPHERE_WITH_DETAILS(Location, Color, Radius, Time)if(GetWorld()) DrawDebugSphere(GetWorld(), Location, Radius, 12, Color, false, Time);
#define DRAW_LINE(Start, End) if(GetWorld()) DrawDebugLine(GetWorld(), Start, End, FColor::Red, true, -1.f, 0, 1.f);
#define DRAW_POINT(Location, Size) if(GetWorld()) DrawDebugPoint(GetWorld(), Location, Size, FColor::Red, true);
#define DRAW_VECTOR(Start, End) if(GetWorld()){DrawDebugPoint(GetWorld(), End, 12.f, FColor::Red, true); DrawDebugLine(GetWorld(), Start, End, FColor::Red, true, -1.f, 0, 1.f);}

#define DRAW_SPHERE_SingleFrame(Location) if(GetWorld()) DrawDebugSphere(GetWorld(), Location, 5.0f, 12, FColor::Magenta, false, -1.f);
#define DRAW_LINE_SingleFrame(Start, End) if(GetWorld()) DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, -1.f, 0, 1.f);
#define DRAW_POINT_SingleFrame(Location, Size) if(GetWorld()) DrawDebugPoint(GetWorld(), Location, Size, FColor::Red, false, -1.f);
#define DRAW_VECTOR_SingleFrame(Start, End) if(GetWorld()){DrawDebugPoint(GetWorld(), End, 12.f, FColor::Red, false, -1.f); DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, -1.f, 0, 1.f);}

