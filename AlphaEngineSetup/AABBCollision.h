//author : Tay Dylan
#ifndef AABB_COLLISION_H
#define AABB_COLLISION_H

#include "Utils.h" // Includes your existing Rect definition

/**
 * @brief Checks if a 2D coordinate point lies within the boundaries of a Rect.
 * @param px The X coordinate of the point.
 * @param py The Y coordinate of the point.
 * @param rect The bounding rectangle to check against.
 * @return True if the point is inside the rectangle, otherwise false.
 */
bool Collision_PointInRect(float px, float py, const Rect& rect);

/**
 * @brief Checks if a 2D coordinate point intersects with a center-aligned UI button.
 * @param px The X coordinate of the point (e.g., mouse cursor).
 * @param py The Y coordinate of the point.
 * @param btnX The X coordinate of the button's center.
 * @param btnY The Y coordinate of the button's center.
 * @param btnScaleX The full width (scale) of the button.
 * @param btnScaleY The full height (scale) of the button.
 * @return True if the point is inside the button, otherwise false.
 */
bool Collision_PointInButton(float px, float py, float btnX, float btnY, float btnScaleX, float btnScaleY);

/**
 * @brief Checks if two rectangular boundaries overlap in 2D space.
 * @param a The first rectangle.
 * @param b The second rectangle.
 * @return True if the rectangles overlap, otherwise false.
 */
bool Collision_RectToRect(const Rect& a, const Rect& b);

#endif