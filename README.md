# BezierCurve
---
# 2/25/2022
---
Draw bezier curve with Direct2D(default bezier is cubic bezier) and try to calculate the y-coordinate with x-coordinate.
---
## Sample Project Files
### BezierData(.h and .cpp)
Save bezier data and handle mouse message from the window. In Direct2D, a bezier curve was divided into several segments. Each segment saves three points: the right control point of the previous kont point, current knot point and the left control point of current knot point. See the image below to make it clear. In this sample, the bezier curve was divided into 4 segments. As for the first segment, it contains three points: Control Point 1, Control Point 2 and Knot Point. An extra start point is neccessary while creating the Direct2D geometry with the function _BeginFigure()_.
![neutral](https://raw.githubusercontent.com/se6en/BezierCurve/main/BezierSample.jpg)
### BezierCurveControl(.h and .cpp)
This is the control to draw bezier curve. I also try to get the y-coordinate with x-coordinate here. Chech the function _CalculateBezierPoint(ComPtr<ID2D1PathGeometry> pBezierGeometry, float fXValue)_, you will see that I just use a vertical line that parallel to the x-axis and check the intersect point to get the y-coordinate of the x-coordinate. In this step, we must implement a child class that inherit from _ID2D1SimplifiedGeometrySink_ to obtain the intersect part of two geometries. What is odd to me that even here is only intersect point, we willl still found that the interserct information will be a line start at the point which x-coordinate is the same and y-coordinate is at the middle of the bezier curve bounds and the line ended at the intersect point. In order to get the correct intersect point, we must check if the point is on the bezier curve with the function _StrokeContainsPoint_.
