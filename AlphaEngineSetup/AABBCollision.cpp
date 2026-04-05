//author : Tay Dylan, Dyaln Lim
#include "AABBCollision.h"

bool Collision_PointInRect(float px, float py, const Rect& rect)
{
    return (px >= static_cast<float>(rect.left) && px <= static_cast<float>(rect.right) &&
        py >= static_cast<float>(rect.bottom) && py <= static_cast<float>(rect.top));
}

bool Collision_PointInButton(float px, float py, float btnX, float btnY, float btnScaleX, float btnScaleY)
{
    float halfW = btnScaleX / 2.0f;
    float halfH = btnScaleY / 2.0f;

    return (px >= (btnX - halfW) && px <= (btnX + halfW) &&
        py >= (btnY - halfH) && py <= (btnY + halfH));
}

bool Collision_RectToRect(const Rect& a, const Rect& b)
{
    return a.IsIntersect(b);
}