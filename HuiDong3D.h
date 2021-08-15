/**
 * @file	HuiDong3D.h
 * @brief	3D 运算库的头文件
 *
 * @author	huidong
 * @mail	huidong_mail@163.com
 * @version	Ver 0.1.2 (beta)
 * @date	创建时间 2021.8.2
 *			最后修改 2021.8.15
 *
 */

#pragma once

#include <math.h>
#include <time.h>
#include <algorithm>

 // 图形库
#include <easyx.h>
#include <stdio.h>

#define _HD3D_BEGIN	namespace HD3D {
#define _HD3D_END	}

_HD3D_BEGIN

//////// 宏定义

/**
 * @brief 多边形最大边数
*/
#define POLYGON_MAX_SIDES 16

/**
 * @brief 渲染花费的最少时间（秒）
 * @note 此值只在特殊情况下作为取值，不能改变所需渲染时间
*/
#define MIN_TIME_COST 1.0 / 9999

//////// 基础类型定义

/**
 * @brief 颜色信息（为负数则表示无颜色）
*/
typedef int Color;

/**
 * @brief 2D 坐标
*/
struct Point2D
{
	double x;
	double y;
};

/**
 * @brief 3D 坐标
*/
struct Point3D
{
	double x;
	double y;
	double z;
};

/**
 * @brief 带有颜色的 3D 坐标
*/
struct ColorPoint3D
{
	double x;
	double y;
	double z;
	Color color;

	/**
	 * @brief 转换为不带颜色的 3D 坐标
	*/
	operator Point3D() const { return { x,y,z }; }
};

/**
 * @brief 3D 立方体，存储立方体的最大和最小的 x,y,z 值
*/
struct Rectangle3D
{
	double min_x, min_y, min_z, max_x, max_y, max_z;
};

/**
 * @brief 缩放因子
*/
struct Zoom
{
	double x;	/** @brief x 方向缩放因子 */
	double y;	/** @brief y 方向缩放因子 */
};

/**
 * @brief 3D 多边形
*/
class Polygon3D
{
private:

	void init()
	{
		pPoints = new Point3D[POLYGON_MAX_SIDES];
		memset(pPoints, 0, sizeof Point3D * POLYGON_MAX_SIDES);
		nPointsNum = 0;
		color = 0;
	}

public:

	Polygon3D()
	{
		init();
	}

	/**
	 * @brief 根据顶点创建多边形
	 * @param[in] p : 顶点数组
	 * @param[in] n : 顶点数量
	 * @param[in] c : 多边形填充颜色
	 * @attention 顶点数量不得超过最大顶点数量
	 * @see POLYGON_MAX_SIDES
	*/
	Polygon3D(const Point3D* p, int n, Color c)
	{
		init();
		for (int i = 0; i < n; i++)
			pPoints[i] = p[i];
		nPointsNum = n;
		color = c;
	}

	/*~Polygon3D()
	{
		clear();
	}*/

	bool operator< (Polygon3D p)
	{
		return GetCenterZ() < p.GetCenterZ();
	}

	bool operator>= (Polygon3D p)
	{
		return GetCenterZ() >= p.GetCenterZ();
	}

	/**
	 * @brief 获取此多边形的 z 层次的中心
	*/
	double GetCenterZ()
	{
		if (nPointsNum <= 0) return 0;
		double min = pPoints[0].z, max = pPoints[0].z;
		for (int i = 0; i < nPointsNum; i++)
		{
			if (pPoints[i].z < min)
			{
				min = pPoints[i].z;
			}
			else if (pPoints[i].z > max)
			{
				max = pPoints[i].z;
			}
		}
		return min + (max - min) / 2;
	}

	/**
	 * @brief 清空内容，释放内存
	*/
	void clear()
	{
		if (pPoints) delete[] pPoints;
		pPoints = NULL;
	}

	Point3D* pPoints;	/** @brief 多边形顶点 */
	int nPointsNum;		/** @brief 多边形顶点数量 */
	Color color;		/** @brief 多边形填充颜色 */
};

/**
 * @brief 删除多边形数组
 * @param p : 多边形数组
 * @param num : 数组长度
 * @note 自带对空数组的判断
*/
inline void DeletePolygons(Polygon3D*& p, int num)
{
	if (!p)
		return;
	for(int i=0;i<num;i++)
		p[i].clear();
	delete[] p;
	p = NULL;
}

/**
 * @brief 复制多边形数组
 * @param pDst : 目标数组
 * @param pSrc : 源数组
 * @param num : 源数组长度
 * @return 返回目标数组的指针，如果任一数组为空则返回 NULL
*/
inline Polygon3D* CopyPolygons(Polygon3D* pDst, Polygon3D* pSrc, int num)
{
	if (!pDst || !pSrc)
	{
		return NULL;
	}
	for (int i = 0; i < num; i++)
	{
		for (int j = 0; j < pSrc[i].nPointsNum; j++)
		{
			pDst[i].pPoints[j] = pSrc[i].pPoints[j];
		}
		pDst[i].nPointsNum = pSrc[i].nPointsNum;
		pDst[i].color = pSrc[i].color;
	}
	return pDst;
}

/**
 * @brief 3D 物体的姿态
 * @attention 姿态的旋转角度都是基于物体自身的，并非相对于旋转原点
 * @note 以下角度都以逆时针作为正的旋转方向
 */
struct Attitude3D
{
	double a;	/** @brief 物体自身的 z 轴旋转角 */
	double e;	/** @brief 物体自身的 y 轴旋转角 */
	double r;	/** @brief 物体自身的 x 轴旋转角 */
};

/**
 * @brief 旋转顺序
*/
enum RotateOrder { rotate_x, rotate_y, rotate_z };

/**
 * @brief 默认旋转顺序
*/
int m_defaultRotateOrder[3] = { rotate_z,rotate_y,rotate_x };

//////// 基本函数实现

/**
 * @brief 将不带颜色的 3D 坐标转换为带颜色的 3D 坐标
*/
inline ColorPoint3D ToColorPoint3D(Point3D p)
{
	return { p.x,p.y,p.z,(Color)0 };
}

/**
 * @brief 将角度转换为弧度
 * @param[in] angle : 原角度
 * @return 角度转换后得到的弧度
*/
inline double ConvertToRadian(double angle)
{
	return 3.1415926535 / 180.0 * angle;
}

/**
 * @brief 旋转 2D 坐标
 * @param[in] x : x 坐标
 * @param[in] y : y 坐标
 * @param[in] angle : 逆时针旋转角度，为负数则表示顺时针旋转
 * @return 旋转后的 2D 坐标
*/
inline Point2D Rotate2D(double x, double y, double angle)
{
	if (angle == 0)	return { x,y };
	double t = ConvertToRadian(angle);
	return { x * cos(t) - y * sin(t),x * sin(t) + y * cos(t) };
}

/**
 * @brief 顺时针旋转 2D 坐标
 * @param[in] p : 要旋转的 2D 坐标
 * @param[in] angle : 旋转角度
 * @return 旋转后的 2D 坐标
*/
inline Point2D Rotate2D(Point2D p, double angle)
{
	return Rotate2D(p.x, p.y, angle);
}

/**
 * @brief 将 3D 坐标沿 X 轴顺时针旋转
 * @param[in] p : 要旋转的坐标
 * @param[in] angle : 旋转角度
 * @param[in] pOrigin : 旋转原点
 * @return 旋转后的坐标
*/
inline Point3D Rotate3D_X(Point3D p, double angle, Point3D pOrigin = { 0,0,0 })
{
	Point2D p2 = Rotate2D({ p.y - pOrigin.y,p.z - pOrigin.z }, angle);
	return { p.x,p2.x + pOrigin.y,p2.y + pOrigin.z };
}

/**
 * @brief 将 3D 坐标沿 Y 轴顺时针旋转
 * @param[in] p : 要旋转的坐标
 * @param[in] angle : 旋转角度
 * @param[in] pOrigin : 旋转原点
 * @return 旋转后的坐标
*/
inline Point3D Rotate3D_Y(Point3D p, double angle, Point3D pOrigin = { 0,0,0 })
{
	Point2D p2 = Rotate2D({ p.x - pOrigin.x,p.z - pOrigin.z }, angle);
	return { p2.x + pOrigin.x,p.y,p2.y + pOrigin.z };
}

/**
 * @brief 将 3D 坐标沿 Z 轴顺时针旋转
 * @param[in] p : 要旋转的坐标
 * @param[in] angle : 旋转角度
 * @param[in] pOrigin : 旋转原点
 * @return 旋转后的坐标
*/
inline Point3D Rotate3D_Z(Point3D p, double angle, Point3D pOrigin = { 0,0,0 })
{
	Point2D p2 = Rotate2D({ p.x - pOrigin.x,p.y - pOrigin.y }, angle);
	return { p2.x + pOrigin.x,p2.y + pOrigin.y,p.z };
}

/**
 * @brief 旋转 3D 坐标
 * @param[in] p : 原坐标
 * @param[in] a : 绕 z 轴的旋转角度
 * @param[in] e : 绕 y 轴的旋转角度
 * @param[in] r : 绕 x 轴的旋转角度
 * @param[in] pOrigin : 旋转原点
 * @param[in] pOrder : 旋转顺序
 * @return 旋转后的坐标
*/
inline Point3D Rotate3D(Point3D p, double a, double e, double r, Point3D pOrigin = { 0,0,0 }, const int pOrder[3] = m_defaultRotateOrder)
{
	for (int i = 0; i < 3; i++)
	{
		switch (pOrder[i])
		{
		case rotate_x: p = Rotate3D_X(p, r, pOrigin); break;
		case rotate_y: p = Rotate3D_Y(p, e, pOrigin); break;
		case rotate_z: p = Rotate3D_Z(p, a, pOrigin); break;
		}
	}
	return p;
}

/**
 * @brief 将多边形集合绕着相机旋转
 * @param[in] pPolygons : 多边形集合
 * @param[in] num : 多边形数量
 * @param[in] atiCamera : 相机姿态
 * @param[in] pCamera : 相机位置
 * @return 返回旋转后的多边形集合
*/
inline Polygon3D* RotateToCamera(Polygon3D* pPolygons, int num, Attitude3D atiCamera, Point3D pCamera)
{
	Polygon3D* pRotated = new Polygon3D[num];
	CopyPolygons(pRotated,pPolygons,num);

	for (int i = 0; i < num; i++)
	{
		for (int j = 0; j < pRotated[i].nPointsNum; j++)
		{
			pRotated[i].pPoints[j] = Rotate3D(pRotated[i].pPoints[j], -atiCamera.a, -atiCamera.e, -atiCamera.r, pCamera);
		}
	}
	return pRotated;
}

/**
 * @brief 将多边形集合转换坐标系
 * @param[in] pPolygons : 坐标系集合
 * @param[in] num : 多边形数量
 * @param[in] pOrigin : 新的坐标系的原点在旧坐标系中的位置
 * @return 返回转换坐标系后的多边形集合
*/
inline Polygon3D* ConvertCoordinateSystem(Polygon3D* pPolygons, int num, Point3D pOrigin)
{
	Polygon3D* pConverted = new Polygon3D[num];
	CopyPolygons(pConverted, pPolygons, num);

	for (int i = 0; i < num; i++)
	{
		for (int j = 0; j < pConverted[i].nPointsNum; j++)
		{
			pConverted[i].pPoints[j].x -= pOrigin.x;
			pConverted[i].pPoints[j].y -= pOrigin.y;
			pConverted[i].pPoints[j].z -= pOrigin.z;
		}
	}
	return pConverted;
}

/**
 * @brief 将多边形集合加入透视投影效果
 * @param[in] pPolygons : 多边形集合
 * @param[in] num : 多边形数量
 * @param[in] nFocal : 焦距
 * @return 返回透视投影后的多边形集合
 * @attention 透视中心点是坐标系原点
 * @note 不会对视口外的点进行裁剪
*/
inline Polygon3D* GetPerspectiveProjectionPolygons(Polygon3D* pPolygons, int num, int nFocal)
{
	Polygon3D* pProjection = new Polygon3D[num];
	CopyPolygons(pProjection, pPolygons, num);

	for (int i = 0; i < num; i++)
	{
		for (int j = 0; j < pProjection[i].nPointsNum; j++)
		{
			double zoom = (nFocal - pPolygons[i].pPoints[j].z) / nFocal;
			pProjection[i].pPoints[j].x *= zoom;
			pProjection[i].pPoints[j].y *= zoom;
		}
	}
	return pProjection;
}

/**
 * @brief 裁剪 NDC 多边形集合
 * @param[in] pPolygons : NDC 多边形集合
 * @param[in] num : 多边形数量
 * @param[in] nFocal : 焦距
 * @param[out] out_count : 返回裁剪后多边形总数
 * @return 返回裁剪后的多边形集合
*/
inline Polygon3D* CropNDCPolygons(Polygon3D* pPolygons, int num, int nFocal, int* out_count)
{

	/////////// 暂时做的是一个比较简单粗暴的判断
	/////////// 现在只是做了在视口区域外的裁剪 (x[-1 ~ 1]  y [-1 ~ 1]  z[0 ~ f])
	/////////// 后续要改成多边形交集判断

	Polygon3D* pCrop = new Polygon3D[num];

	int count = 0;
	for (int i = 0; i < num; i++)
	{
		int crop_count = 0;
		for (int j = 0; j < pPolygons[i].nPointsNum; j++)
		{
			// 统计一个多边形超出视口的点的个数
			if (pPolygons[i].pPoints[j].x < -1 || pPolygons[i].pPoints[j].x > 1 ||
				pPolygons[i].pPoints[j].y < -1 || pPolygons[i].pPoints[j].y > 1 ||
				pPolygons[i].pPoints[j].z < 0 || pPolygons[i].pPoints[j].z > nFocal)
			{
				crop_count++;
			}

			// 但凡有一个点在视口内就可以被保留
			else
			{
				break;
			}
		}

		// 并非所有点都超出视口的情况下才保留此多边形，否则被裁剪。
		if (crop_count < pPolygons[i].nPointsNum)
		{
			CopyPolygons(&pCrop[count], &pPolygons[i], 1);	
			count ++;
		}
	}

	// 保留下来的内容
	Polygon3D* pReserve = new Polygon3D[count];
	CopyPolygons(pReserve, pCrop, count);

	// 释放创建多的数组
	DeletePolygons(pCrop, num);

	*out_count = count;
	return pReserve;
}

//////// 绘图设备相关

/**
 * @brief 初始化绘图设备
 * @param[in] w : 绘图设备宽
 * @param[in] w : 绘图设备高
 * @param[in] flag : 附加属性，默认为 0
*/
inline void InitDrawingDevice(int w, int h, int flag = 0)
{
	// easyx
	initgraph(w, h, flag);
	//setorigin(w / 4, h / 4);
}

/**
 * @brief	关闭绘图设备
*/
inline void CloseDrawingDevice()
{
	// easyx
	closegraph();
}

/**
 * @brief 获取绘图设备宽度
*/
inline int GetDrawingDeviceWidth()
{
	return getwidth();
}

/**
 * @brief 获取绘图设备高度
*/
inline int GetDrawingDeviceHeight()
{
	return getheight();
}

/**
 * @brief 绘制点
 * @param[in] x : 绘制位置
 * @param[in] y : 绘制位置
 * @param[in] c : 绘制颜色
*/
inline void DrawPixel(int x, int y, Color c)
{
	if (c < 0) return;

	// easyx 绘制
	// 直接操作显存画点
	if (x >= 0 && y >= 0 && x < getwidth() && y < getheight())
		GetImageBuffer()[y * getwidth() + x] = BGR((COLORREF)c);
}

/**
 * @brief 绘制点
 * @param[in] p : 绘制位置
 * @param[in] c : 绘制颜色
*/
inline void DrawPixel(Point2D p, Color c)
{
	DrawPixel((int)p.x, (int)p.y, c);
}

/**
 * @brief 将 3D 的 NDC 坐标 转为 3D 的屏幕坐标
 * @param[in] p : 原坐标
 * @param[in] zoom : 坐标缩放因子
 * @return 返回转换后的屏幕坐标
 * @note 默认观察面为 xoy
*/
inline Point3D ConvertNDC3DToScreenPoint(Point3D p, Zoom zoom = { 1,1 })
{
	return { (p.x * zoom.x + 1) * GetDrawingDeviceWidth(),(1 - p.y * zoom.y) * GetDrawingDeviceHeight() };
}

/**
 * @brief 绘制填充多边形
 * @param[in] p : 3D 多边形
 * @param[in] offset_x : 输出的图像的 x 坐标偏移
 * @param[in] offset_y : 输出的图像的 y 坐标偏移
 * @param[in] zoom : 图像缩放因子
 * @param[in] grid : 多边形网格颜色，为负数表示不绘制网格
 * @attention 只取多边形的 x,y 坐标绘制到屏幕
*/
inline void DrawFillPolygon(Polygon3D p, int offset_x = 0, int offset_y = 0, Zoom zoom = { 1,1 }, Color grid = -1)
{
	if (p.nPointsNum <= 0) return;

	POINT* pPoints = new POINT[p.nPointsNum];
	for (int j = 0; j < p.nPointsNum; j++)
	{
		Point3D pp = ConvertNDC3DToScreenPoint(p.pPoints[j], zoom);
		pPoints[j] = { (long)(pp.x) + offset_x,(long)(pp.y) + offset_y };
	}

	//// easyx 绘制

	// 有填充颜色，需要输出	
	if (p.color >= 0)
	{
		// 单点
		if (p.nPointsNum == 1)
		{
			DrawPixel(pPoints[0].x, pPoints[0].y, p.color);
		}
		else
		{
			// 有特定网格颜色
			if (grid >= 0)
			{
				setlinecolor((COLORREF)grid);
			}
			else
			{
				setlinecolor((COLORREF)p.color);
			}
			setfillcolor((COLORREF)p.color);
			fillpolygon(pPoints, p.nPointsNum);
		}
	}

	// 无填充颜色，不绘制多边形，除非需要绘制网格
	else
	{
		if (grid >= 0)
		{
			setlinecolor((COLORREF)grid);
			setfillcolor(WHITE);
			polygon(pPoints, p.nPointsNum);
		}
	}

	delete[] pPoints;
}

//////// 类定义

/**
 * @brief 3D 物体
*/
class Object3D
{
private:

	Polygon3D* pPolygons;			/** @brief 物体的多边形坐标 */
	Polygon3D* pRotatedPolygons;	/** @brief 计算过姿态后物体的多边形坐标 */
	int nPolygonsNum;				/** @brief 物体的多边形数量 */

	Point3D pCenter;		/** @brief 物体中心点 */
	Attitude3D attitude;	/** @brief 物体姿态 */
	int rotate_order[3];	/** @brief 旋转顺序 */

	/**
	 * @brief	更新物体中心点位置
	*/
	void UpdateCenterPoint()
	{
		Rectangle3D r = GetRectangle(false);

		pCenter = {
			(r.max_x - r.min_x) / 2 + r.min_x,
			(r.max_y - r.min_y) / 2 + r.min_y,
			(r.max_z - r.min_z) / 2 + r.min_z
		};
	}

	/**
	 * @brief 更新旋转点数组长度
	 * @attention 将擦除原数组中的数据
	*/
	void UpdateRotatedPointsArrayLength()
	{
		Polygon3D* newArray = new Polygon3D[nPolygonsNum];
		DeletePolygons(pRotatedPolygons, nPolygonsNum);
		pRotatedPolygons = newArray;
	}

	/**
	 * @brief 更新元素数组
	 * @attention 物体的元素数量发生更改时，调用此函数
	*/
	void UpdateArray()
	{
		UpdateCenterPoint();
		UpdateRotatedPointsArrayLength();
		UpdateRotatedPoints();
	}

public:

	Object3D()
	{
		pPolygons = NULL;
		pRotatedPolygons = NULL;
		nPolygonsNum = 0;
		attitude = { 0,0,0 };
		pCenter = { 0,0,0 };
		rotate_order[0] = rotate_z;
		rotate_order[1] = rotate_y;
		rotate_order[2] = rotate_x;
	}

	~Object3D()
	{
		Polygon3D* p = NULL;
		for (int i = 0; i < 2; i++)
		{
			if (i == 0) p = pPolygons;
			else p = pRotatedPolygons;
			DeletePolygons(p, nPolygonsNum);
		}
	}

	/**
	 * @brief 设置旋转顺序
	*/
	void SetRotateOrder(int pOrder[3])
	{
		for (int i = 0; i < 3; i++)
			rotate_order[i] = pOrder[i];
	}

	/**
	 * @brief 获取旋转顺序
	*/
	int* GetRotateOrder()
	{
		return rotate_order;
	}

	/**
	 * @brief 获取物体中点的数量
	*/
	int GetPointsNum()
	{
		int num = 0;
		for (int i = 0; i < nPolygonsNum; i++)
		{
			num += pPolygons[i].nPointsNum;
		}
		return num;
	}

	/**
	 * @brief 获取物体中多边形的数量
	*/
	int GetPolygonsNum()
	{
		return nPolygonsNum;
	}

	/**
	 * @brief 获取物体的所有点
	 * @param[in] rotated : 是否获取旋转后的点
	 * @attention 调用 GetPointsNum 函数来获取点的数量
	*/
	ColorPoint3D* GetPoints(bool rotated = true)
	{
		Polygon3D* polygons = pPolygons;
		if (rotated) polygons = pRotatedPolygons;
		ColorPoint3D* p = new ColorPoint3D[GetPointsNum()];
		for (int i = 0, index = 0; i < nPolygonsNum; i++)
		{
			for (int j = 0; j < polygons[i].nPointsNum; j++)
			{
				p[index] = ToColorPoint3D(polygons[i].pPoints[j]);
				p[index].color = polygons[i].color;
				index++;
			}
		}
		return p;
	}

	/**
	 * @brief 获取物体中的所有多边形
	 * @param[in] rotated : 是否获取旋转后的多边形
	 * @attention 调用 GetPolygonsNum 函数来获取多边形的数量
	*/
	Polygon3D* GetPolygons(bool rotated = true)
	{
		if (rotated)
		{
			return pRotatedPolygons;
		}
		else
		{
			return pPolygons;
		}
	}

	/**
	 * @brief 获取物体中心点坐标
	*/
	Point3D GetCenterPoint()
	{
		return pCenter;
	}

	/**
	 * @brief 获取物体位置，效果与 GetCenterPoint 函数相同
	*/
	Point3D GetPosition()
	{
		return GetCenterPoint();
	}

	/**
	 * @brief 移动物体到某位置，并立即计算新坐标
	*/
	void MoveTo(Point3D pNew)
	{
		double offset_x = pNew.x - pCenter.x;
		double offset_y = pNew.y - pCenter.y;
		double offset_z = pNew.z - pCenter.z;
		for (int i = 0; i < nPolygonsNum; i++)
		{
			for (int j = 0; j < pPolygons[i].nPointsNum; j++)
			{
				pPolygons[i].pPoints[j].x += offset_x;
				pPolygons[i].pPoints[j].y += offset_y;
				pPolygons[i].pPoints[j].z += offset_z;

				pRotatedPolygons[i].pPoints[j].x += offset_x;
				pRotatedPolygons[i].pPoints[j].y += offset_y;
				pRotatedPolygons[i].pPoints[j].z += offset_z;
			}
		}

		pCenter = pNew;
	}

	/**
	 * @brief 将物体沿 x 轴方向移动，并立即计算新坐标
	*/
	void MoveX(double n)
	{
		for (int i = 0; i < nPolygonsNum; i++)
		{
			for (int j = 0; j < pPolygons[i].nPointsNum; j++)
			{
				pPolygons[i].pPoints[j].x += n;
				pRotatedPolygons[i].pPoints[j].x += n;
			}
		}

		pCenter.x += n;
	}

	/**
	 * @brief 将物体沿 y 轴方向移动，并立即计算新坐标
	*/
	void MoveY(double n)
	{
		for (int i = 0; i < nPolygonsNum; i++)
		{
			for (int j = 0; j < pPolygons[i].nPointsNum; j++)
			{
				pPolygons[i].pPoints[j].y += n;
				pRotatedPolygons[i].pPoints[j].y += n;
			}
		}

		pCenter.y += n;
	}

	/**
	 * @brief 将物体沿 z 轴方向移动，并立即计算新坐标
	*/
	void MoveZ(double n)
	{
		for (int i = 0; i < nPolygonsNum; i++)
		{
			for (int j = 0; j < pPolygons[i].nPointsNum; j++)
			{
				pPolygons[i].pPoints[j].z += n;
				pRotatedPolygons[i].pPoints[j].z += n;
			}
		}

		pCenter.z += n;
	}

	/**
	 * @brief 设置物体姿态
	*/
	void SetAttitude(Attitude3D ati)
	{
		attitude = ati;
	}

	/**
	 * @brief 获取物体姿态
	*/
	Attitude3D GetAttitude()
	{
		return attitude;
	}

	/**
	 * @brief 绕 x 轴顺时针调整物体姿态
	*/
	void RotateX(double angle)
	{
		attitude.r += angle;
	}

	/**
	 * @brief 绕 y 轴顺时针调整物体姿态
	*/
	void RotateY(double angle)
	{
		attitude.e += angle;
	}

	/**
	 * @brief 绕 z 轴顺时针调整物体姿态
	*/
	void RotateZ(double angle)
	{
		attitude.a += angle;
	}

	/**
	 * @brief 获取物体的三维矩形边框
	 * @param[in] bRotated : 是否获取已经旋转的物体的边框
	 * @return 返回物体边框
	*/
	Rectangle3D GetRectangle(bool bRotated = true)
	{
		Rectangle3D r = {};
		Polygon3D* p = pPolygons;
		if(bRotated && pRotatedPolygons)
			p = pRotatedPolygons;
		for (int i = 0; i < nPolygonsNum; i++)
		{
			for (int j = 0; j < p[i].nPointsNum; j++)
			{
				// init
				if (i + j == 0)
				{
					r.min_x = r.max_x = p[i].pPoints[j].x;
					r.min_y = r.max_y = p[i].pPoints[j].y;
					r.min_z = r.max_z = p[i].pPoints[j].z;
					continue;
				}

				if (p[i].pPoints[j].x < r.min_x)
					r.min_x = p[i].pPoints[j].x;
				else if (p[i].pPoints[j].x > r.max_x)
					r.max_x = p[i].pPoints[j].x;

				if (p[i].pPoints[j].y < r.min_y)
					r.min_y = p[i].pPoints[j].y;
				else if (p[i].pPoints[j].y > r.max_y)
					r.max_y = p[i].pPoints[j].y;

				if (p[i].pPoints[j].z < r.min_z)
					r.min_z = p[i].pPoints[j].z;
				else if (p[i].pPoints[j].z > r.max_z)
					r.max_z = p[i].pPoints[j].z;
			}
		}
		return r;
	}

	/**
	 * @brief 更新旋转点数组，计算调整姿态后的新坐标
	 * @attention 设置或调整物体姿态后不会立即计算新的物体坐标，需要手动调用
	 *			此函数进行计算。
	*/
	void UpdateRotatedPoints()
	{
		for (int i = 0; i < nPolygonsNum; i++)
		{
			for (int j = 0; j < pPolygons[i].nPointsNum; j++)
			{
				pRotatedPolygons[i].pPoints[j] = Rotate3D(pPolygons[i].pPoints[j], attitude.a, attitude.e, attitude.r, pCenter, rotate_order);
			}
			pRotatedPolygons[i].nPointsNum = pPolygons[i].nPointsNum;
			pRotatedPolygons[i].color = pPolygons[i].color;
		}
	}

	/**
	 * @brief 在物体中添加点
	 * @attention 此函数中以单个点为单位添加，所以每个点都独立作为一个多边形插入数组
	 * @param[in] pNew : 要添加的点的数组
	 * @param[in] num : 要添加的点的数量
	 * @return 返回新添加的（多个）点在数组中的第一个索引，若插入数据失败返回 -1
	*/
	int AddPoints(ColorPoint3D* pNew, int num)
	{
		if (num <= 0 || !pNew)	return -1;

		Polygon3D* newArray = new Polygon3D[nPolygonsNum + num];
		CopyPolygons(newArray, pPolygons, nPolygonsNum);

		for (int i = nPolygonsNum, j = 0; j < num; i++, j++)
		{
			newArray[i].pPoints[0] = pNew[j];
			newArray[i].nPointsNum = 1;
			newArray[i].color = pNew[j].color;
		}

		DeletePolygons(pPolygons, nPolygonsNum);

		pPolygons = newArray;
		nPolygonsNum += num;

		UpdateArray();

		return nPolygonsNum - num;
	}

	/**
	 * @brief 在物体中添加多边形
	 * @param[in] pNew : 要添加的多边形的数组
	 * @param[in] num : 要添加的多边形的数量
	 * @return 返回新添加的（多个）多边形在数组中的第一个索引，若插入数据失败返回 -1
	*/
	int AddPolygons(Polygon3D* pNew, int num)
	{
		if (num <= 0 || !pNew)	return -1;

		Polygon3D* newArray = new Polygon3D[nPolygonsNum + num];
		CopyPolygons(newArray, pPolygons, nPolygonsNum);
		CopyPolygons(newArray+ nPolygonsNum, pNew, num);

		DeletePolygons(pPolygons, nPolygonsNum);

		pPolygons = newArray;
		nPolygonsNum += num;

		UpdateArray();

		return nPolygonsNum - num;
	}

	/**
	 * @brief 删除物体中的某个多边形
	 * @param[in] p : 要删除的点的索引
	*/
	void DeletePolygon(int index)
	{
		Polygon3D* newArray = new Polygon3D[nPolygonsNum - 1];
		for (int i = 0, j = 0; j < nPolygonsNum; i++, j++)
		{
			if (j != index)
			{
				newArray[i] = pPolygons[j];
			}
			else
			{
				i--;
			}
		}

		DeletePolygons(pPolygons, nPolygonsNum);

		pPolygons = newArray;
		nPolygonsNum--;

		UpdateArray();
	}

};


/**
 * @brief 3D 场景
*/
class Scence3D
{

private:
	Object3D* pObjects;	/** @brief 场景内的物体集合 */
	int nObjectsNum;	/** @brief 场景内的物体数量 */

	Point3D pCamera;			/** @brief 相机坐标 */
	Attitude3D attitudeCamera;	/** @brief 相机姿态 */
	int nViewportWidth;			/** @brief 相机视口宽度 */
	int nViewportHeight;		/** @brief 相机视口高度 */

	/**
	 * @brief 相机焦距
	 * @attention 为了画面的正常显示，焦距最好不小于 600
	*/
	int nFocalLength;

	bool bPerspectiveProjection;	/** @brief 是否使用透视投影 */

public:

	Scence3D()
	{
		pObjects = NULL;
		nObjectsNum = 0;

		pCamera = { 0,0,0 };
		attitudeCamera = { 0,0,0 };

		nViewportWidth = 640;
		nViewportHeight = 480;
		nFocalLength = 1000;

		bPerspectiveProjection = true;
	}

	~Scence3D()
	{
		if (pObjects) delete[] pObjects;
	}

	/**
	 * @brief 设置相机位置
	*/
	void SetCameraPosition(Point3D p)
	{
		pCamera = p;
	}

	/**
	 * @brief 获取相机位置
	*/
	Point3D GetCameraPosition()
	{
		return pCamera;
	}

	/**
	 * @brief 移动相机，效果与 SetCameraPosition 相同
	*/
	void MoveCameraTo(Point3D p)
	{
		SetCameraPosition(p);
	}

	/**
	 * @brief 在 x 轴方向移动相机
	*/
	void MoveCameraX(double n)
	{
		pCamera.x += n;
	}

	/**
	 * @brief 在 y 轴方向移动相机
	*/
	void MoveCameraY(double n)
	{
		pCamera.y += n;
	}

	/**
	 * @brief 在 z 轴方向移动相机
	*/
	void MoveCameraZ(double n)
	{
		pCamera.z += n;
	}

	/**
	 * @brief 设置相机姿态
	*/
	void SetCameraAttitude(Attitude3D ati)
	{
		attitudeCamera = ati;
	}

	/**
	 * @brief 获取相机姿态
	*/
	Attitude3D GetCameraAttitude()
	{
		return attitudeCamera;
	}

	/**
	 * @brief 将相机绕 x 轴旋转
	*/
	void RotateCameraX(double angle)
	{
		attitudeCamera.r += angle;
	}

	/**
	 * @brief 将相机绕 y 轴旋转
	*/
	void RotateCameraY(double angle)
	{
		attitudeCamera.e += angle;
	}

	/**
	 * @brief 将相机绕 z 轴旋转
	*/
	void RotateCameraZ(double angle)
	{
		attitudeCamera.a += angle;
	}

	/**
	 * @brief 设置相机视口大小
	*/
	void SetCameraViewportSize(int w, int h)
	{
		nViewportWidth = w;
		nViewportHeight = h;
	}

	/**
	 * @brief 获取相机视口大小
	 * @param[out]	w : 视口宽度
	 * @param[out]	h : 视口高度
	*/
	void GetCameraViewportSize(int* w, int* h)
	{
		*w = nViewportWidth;
		*h = nViewportHeight;
	}

	/**
	 * @brief 设置相机焦距
	*/
	void SetCameraFocalLength(int f)
	{
		nFocalLength = f;
	}

	/**
	 * @brief 获取相机焦距
	*/
	int GetCameraFocalLength()
	{
		return nFocalLength;
	}

	/**
	 * @brief 设置透视投影的开闭
	 * @param b : 是否开启透视投影
	*/
	void EnablePerspectiveProjection(bool b = true)
	{
		bPerspectiveProjection = b;
	}

	/**
	 * @brief 获取透视投影状态
	*/
	bool GetPerspectiveProjectionState()
	{
		return bPerspectiveProjection;
	}

	/**
	 * @brief 在场景中添加物体
	 * @param[in] obj : 要添加的物体
	 * @return 返回添加的物体在数组中的索引
	*/
	int AddObject(Object3D& obj)
	{
		Object3D* newObjects = new Object3D[nObjectsNum + 1];
		for (int i = 0; i < nObjectsNum; i++)
		{
			newObjects[i].AddPolygons(pObjects[i].GetPolygons(), pObjects[i].GetPolygonsNum());
			newObjects[i].SetAttitude(pObjects[i].GetAttitude());
			newObjects[i].SetRotateOrder(pObjects[i].GetRotateOrder());
		}

		newObjects[nObjectsNum].AddPolygons(obj.GetPolygons(), obj.GetPolygonsNum());
		newObjects[nObjectsNum].SetAttitude(obj.GetAttitude());
		newObjects[nObjectsNum].SetRotateOrder(obj.GetRotateOrder());

		if (pObjects) delete[] pObjects;
		pObjects = newObjects;
		nObjectsNum++;
		return nObjectsNum - 1;
	}

	/**
	 * @brief 删除场景中的物体
	 * @param[in] pObj : 要删除的物体的索引
	*/
	void DeleteObject(int index)
	{
		Object3D* newObjects = new Object3D[nObjectsNum - 1];
		for (int i = 0, j = 0; j < nObjectsNum; i++, j++)
		{
			if (j != index)
			{
				newObjects[i] = pObjects[j];
			}
			else
			{
				i--;
			}
		}
		if (pObjects)
			delete[] pObjects;
		pObjects = newObjects;
		nObjectsNum--;
	}

	/**
	 * @brief 获取场景中物体的集合
	*/
	Object3D* GetObjects()
	{
		return pObjects;
	}

	/**
	 * @brief 获取场景中物体的数量
	*/
	int GetObjectsNum()
	{
		return nObjectsNum;
	}

	/**
	 * @brief 获取场景中所有的物体的多边形的数量总和
	*/
	int GetAllPolygonsNum()
	{
		int index = 0;
		for (int i = 0; i < nObjectsNum; i++)
			index += pObjects[i].GetPolygonsNum();
		return index;
	}

	/**
	 * @brief 获取场景中所有物体的多边形的集合
	*/
	Polygon3D* GetAllPolygons()
	{
		Polygon3D* p = new Polygon3D[GetAllPolygonsNum()];
		for (int i = 0, index = 0; i < nObjectsNum; i++)
		{
			Polygon3D* pp = pObjects[i].GetPolygons();
			int num = pObjects[i].GetPolygonsNum();
			CopyPolygons(p + index, pp, num);
			index += num;
		}
		return p;
	}

	/**
	 * @brief 获取视口坐标系下的多边形集合（平行投影）
	 * @param[out] count : 返回多边形总数
	*/
	Polygon3D* GetViewportPolygons(int* count = NULL)
	{
		int nAllPolygonsNum = GetAllPolygonsNum();
		if (nAllPolygonsNum <= 0) return NULL;

		Polygon3D* pAllPolygons = GetAllPolygons();
		Polygon3D* pRotated = NULL;
		Polygon3D* pConverted = NULL;

		// 相机视口的原点
		Point3D pOriginViewport = { pCamera.x - nViewportWidth / 2,pCamera.y - nViewportHeight / 2,pCamera.z };

		// 旋转到相机视角（但不平移）
		pRotated = RotateToCamera(pAllPolygons, nAllPolygonsNum, attitudeCamera, pCamera);

		// 平移到视口坐标系
		pConverted = ConvertCoordinateSystem(pRotated, nAllPolygonsNum, pOriginViewport);

		DeletePolygons(pAllPolygons, nAllPolygonsNum);
		DeletePolygons(pRotated, nAllPolygonsNum);
		
		return pConverted;
	}

	/**
	 * @brief 获取 GetViewportPolygons 函数返回的多边形集合的标准化设备坐标（NDC）形式的多边形集合
	*/
	Polygon3D* GetViewportNDCPolygons()
	{
		int nPolygonsNum = GetAllPolygonsNum();
		if (nPolygonsNum <= 0) return NULL;
		Polygon3D* pPolygons = GetViewportPolygons();
		Polygon3D* pConverted = ConvertCoordinateSystem(pPolygons, nPolygonsNum, { nViewportWidth / 2.0,nViewportHeight / 2.0,0 });

		for (int i = 0; i < nPolygonsNum; i++)
		{
			for (int j = 0; j < pConverted[i].nPointsNum; j++)
			{
				pConverted[i].pPoints[j].x /= nViewportWidth / 2.0;
				pConverted[i].pPoints[j].y /= nViewportHeight / 2.0;
			}
		}

		DeletePolygons(pPolygons, nPolygonsNum);
		
		return pConverted;
	}

	/**
	 * @brief 获取要渲染的多边形集合
	 * @param[out] count : 返回要渲染的多边形数量
	 * @return 返回在渲染范围内的多边形集合，且已按 z 坐标升序排列
	 * @note 使用此函数可以获取真正要绘制到设备的多边形集合
	*/
	Polygon3D* GetRenderPolygons(int* count)
	{
		int nPolygonsNum = GetAllPolygonsNum();
		int nCropNum = 0;
		if (nPolygonsNum <= 0) return NULL;
		Polygon3D* pPolygons = GetViewportNDCPolygons();
		Polygon3D* pCrop = NULL;
		Polygon3D* pShow = NULL;
		
		// 开启透视投影的话就进行计算
		if (bPerspectiveProjection)
		{
			////////////////////////////////////////////////////////
			// 透视投影时远处物体扭曲的 bug 的暂时性解决方案：        //
			// 透视的时候使用二倍焦距，然后裁剪的时候只裁剪到一倍焦距  //
			////////////////////////////////////////////////////////

			pCrop = CropNDCPolygons(pPolygons, nPolygonsNum, nFocalLength * 2, &nCropNum);
			pShow = GetPerspectiveProjectionPolygons(pCrop, nCropNum, nFocalLength * 2);
			Polygon3D* pCrop2 = CropNDCPolygons(pShow, nCropNum, nFocalLength, &nCropNum);

			DeletePolygons(pCrop, nCropNum);
			DeletePolygons(pShow, nCropNum);

			pShow = pCrop2;
		}
		else
		{
			pCrop = CropNDCPolygons(pPolygons, nPolygonsNum, nFocalLength, &nCropNum);
			pShow = pCrop;
		}

		// 多边形 z 轴层次排序
		std::sort(pShow, pShow + nCropNum);

		DeletePolygons(pPolygons, nPolygonsNum);
		*count = nCropNum;
		return pShow;
	}

	/**
	 * @brief 绘制场景到屏幕
	 * @param[in] x : 图像输出到屏幕的 x 坐标
	 * @param[in] y : 图像输出到屏幕的 y 坐标
	 * @param[in] zoom : 图像缩放因子
	 * @param[in] grid : 多边形网格颜色，为负数表示不绘制网格
	 * @return 返回绘制耗时（单位：秒）
	*/
	double Render(int x = 0, int y = 0, Zoom zoom = { 1,1 }, Color grid = -1)
	{
		int t = clock();

		int nPolygonsNum = 0;
		Polygon3D* pPolygons = GetRenderPolygons(&nPolygonsNum);

		if (nPolygonsNum <= 0)
			return MIN_TIME_COST;

		for (int i = nPolygonsNum - 1; i >= 0; i--)
		{
			DrawFillPolygon(pPolygons[i], x, y, zoom, grid);
		}

		DeletePolygons(pPolygons, nPolygonsNum);
		
		double cost = (double)(clock() - t);
		if(cost <= 0)
			cost = MIN_TIME_COST;

		return cost / CLOCKS_PER_SEC;
	}

};

_HD3D_END
