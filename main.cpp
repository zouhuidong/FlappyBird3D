#include "HuiDong3D.h"
#include "MusicMCI.h"
#include <conio.h>

// Windows GDI 透明贴图函数所需 lib
#pragma comment( lib, "MSIMG32.lib")

// 判断键盘按下
#define KEY_DOWN(VK_NONAME) (GetForegroundWindow() == GetHWnd() && (GetAsyncKeyState(VK_NONAME) & 0x8000) ? 1:0)

#define PILLAR_SIZE 300		// 柱子顶面和底面的边长
#define PILLAR_HEIGHT 600	// 柱子物体高度

#define PILLAR_MIN_INTERVAL 700		// 前后柱子最小间隙
#define PILLAR_MAX_INTERVAL 1400	// 前后柱子最大间隙

#define PILLAR_MIN_HEIGHT 0		// 柱子位置的最低高度
#define PILLAR_MAX_HEIGHT 300	// 柱子位置的最高高度

#define PILLAR_MIN_GAP 170	// 柱子缺口最小大小
#define PILLAR_MAX_GAP 300	// 柱子缺口最大大小

#define PILLARS_NUM 100	// 柱子数量

// 主场景
HD3D::Scence3D m_scence;

// 鼠标在窗口中心时的位置
POINT m_pCursorCenter;

// 记录跨越的柱子数量
int m_nGoPillarsNum;

// 速度（Z 轴的和 Y 轴的）
int m_speedZ, m_speedY;

// 场景背景色
COLORREF m_bkColor;

IMAGE m_imgStar;	// 星星奖章图像
IMAGE m_imgCup;		// 奖杯图像
IMAGE m_imgGuide;	// 游戏操作说明

// 音效
MusicMCI m_musicBgm, m_musicFly, m_musicHit, m_musicScore, m_musicWind,m_musicWin;

// 柱子信息记录
struct Pillar
{
	int z;	// 柱子坐标 z
	int y;	// 下面的柱子的 y 坐标
	int gap;// 柱子缺口大小

}m_pillars[PILLARS_NUM];

/**
 * @brief Windows GDI 函数实现透明贴图
 * @param x 图像输出位置
 * @param y 图像输出位置
 * @param img 要输出的图像
 * @param bkcolor 图像背景色，即为不输出的颜色
*/
inline void putimage_transparent(int x, int y, IMAGE img, COLORREF bkcolor)
{
	TransparentBlt(GetImageHDC(GetWorkingImage()), x, y, img.getwidth(), img.getheight(), GetImageHDC(&img), 0, 0, img.getwidth(), img.getheight(), bkcolor);
}

/**
 * @brief 获取一个 3D 柱子
 * @param w : 柱子宽度
 * @param h : 柱子高度
 * @param d : 柱子深度（z方向）
 * @return 返回含有六个多边形的数组
*/
HD3D::Polygon3D* GetPillar(double w, double h, double d)
{
	HD3D::Polygon3D* pPolygons = new HD3D::Polygon3D[6];
	HD3D::Point3D pPoints[8] = {
		{0,0,0}, {w,0,0}, {w,0,d}, {0,0,d},
		{0,-h,d}, {w,-h,d}, {0,-h,0}, {w,-h,0}
	};

	pPolygons[0].nPointsNum = 4;
	pPolygons[0].pPoints[0] = pPoints[0];
	pPolygons[0].pPoints[1] = pPoints[1];
	pPolygons[0].pPoints[2] = pPoints[7];
	pPolygons[0].pPoints[3] = pPoints[6];
	pPolygons[0].color = GREEN;

	pPolygons[1].nPointsNum = 4;
	pPolygons[1].pPoints[0] = pPoints[0];
	pPolygons[1].pPoints[1] = pPoints[3];
	pPolygons[1].pPoints[2] = pPoints[4];
	pPolygons[1].pPoints[3] = pPoints[6];
	pPolygons[1].color = GREEN;

	pPolygons[2].nPointsNum = 4;
	pPolygons[2].pPoints[0] = pPoints[2];
	pPolygons[2].pPoints[1] = pPoints[3];
	pPolygons[2].pPoints[2] = pPoints[4];
	pPolygons[2].pPoints[3] = pPoints[5];
	pPolygons[2].color = GREEN;

	pPolygons[3].nPointsNum = 4;
	pPolygons[3].pPoints[0] = pPoints[1];
	pPolygons[3].pPoints[1] = pPoints[2];
	pPolygons[3].pPoints[2] = pPoints[5];
	pPolygons[3].pPoints[3] = pPoints[7];
	pPolygons[3].color = GREEN;

	pPolygons[4].nPointsNum = 4;
	pPolygons[4].pPoints[0] = pPoints[0];
	pPolygons[4].pPoints[1] = pPoints[1];
	pPolygons[4].pPoints[2] = pPoints[2];
	pPolygons[4].pPoints[3] = pPoints[3];
	pPolygons[4].color = RED;

	pPolygons[5].nPointsNum = 4;
	pPolygons[5].pPoints[0] = pPoints[4];
	pPolygons[5].pPoints[1] = pPoints[5];
	pPolygons[5].pPoints[2] = pPoints[7];
	pPolygons[5].pPoints[3] = pPoints[6];
	pPolygons[5].color = GREEN;

	return pPolygons;
}

/**
 * @brief 锁定鼠标在窗口内
 * @param hwnd : 窗口句柄
*/
void ClipCursor(HWND hwnd)
{
	RECT rt;
	POINT lt, rb;
	GetClientRect(hwnd, &rt);
	lt.x = rt.left;
	lt.y = rt.top;
	rb.x = rt.right;
	rb.y = rt.bottom;
	ClientToScreen(hwnd, &lt);
	ClientToScreen(hwnd, &rb);
	rt.left = lt.x;
	rt.top = lt.y;
	rt.right = rb.x;
	rt.bottom = rb.y;
	ClipCursor(&rt);
}

/**
 * @brief 将鼠标位置设置在窗口中心
 * @param hwnd : 窗口句柄
 * @return 返回鼠标在窗口中心时的位置
*/
POINT SetCursorAtWindowCenter(HWND hwnd)
{
	RECT pos;
	POINT cursor;
	GetWindowRect(hwnd, &pos);
	SetCursorPos(pos.left + (pos.right - pos.left) / 2, pos.top + (pos.bottom - pos.top) / 2);
	GetCursorPos(&cursor);
	return cursor;
}

/**
 * @brief 获取鼠标坐标相对于窗口中心的偏移量
 * @param center : 鼠标在窗口中心时的位置
 * @return 返回当前鼠标相对于窗口中心的位置
*/
POINT GetCursorMoveCenter(POINT center)
{
	POINT p;
	GetCursorPos(&p);
	p.x -= center.x;
	p.y -= center.y;
	return p;
}


/**
 * @brief 显示 fps
 * @param fps : 当前帧数
*/
void showfps(double fps)
{
	wchar_t str[32] = { 0 };
	wsprintf(str, L"fps: %d", (int)fps);
	outtextxy(0, 0, str);
}

/**
 * @brief 获取奖牌图像
 * @param strPrize : 奖牌内容字符串，0 表示奖章，1 表示奖杯
 * @return 返回奖牌图像
*/
IMAGE GetPrizeImage(LPCTSTR strPrize)
{
	const int imgInterval = 10;	// 图像间距
	int imgWidth = 0;
	int imgHeight = m_imgStar.getheight() > m_imgCup.getheight() ? m_imgStar.getheight() : m_imgCup.getheight();
	IMAGE img;

	for (int i = 0; i < lstrlen(strPrize); i++)
	{
		switch (strPrize[i])
		{
		case L'0': imgWidth += m_imgStar.getwidth();	break;
		case L'1': imgWidth += m_imgCup.getwidth();		break;
		}
	}

	imgWidth += (lstrlen(strPrize) - 1) * imgInterval;
	img.Resize(imgWidth, imgHeight);

	SetWorkingImage(&img);
	setbkcolor(WHITE);
	cleardevice();
	for (int i = 0, w = 0; i < lstrlen(strPrize); i++)
	{
		switch (strPrize[i])
		{
		case L'0':
			putimage(w, 0, &m_imgStar);
			w += m_imgStar.getwidth() + imgInterval;
			break;
		case L'1':
			putimage(w, 0, &m_imgCup);
			w += m_imgCup.getwidth() + imgInterval;
			break;
		}
	}
	SetWorkingImage();

	return img;
}

/**
 * @brief 显示当前游戏进度数据
*/
void ShowGamingInfo()
{
	const int size = 256;
	wchar_t str[size] = { 0 };
	IMAGE imgPrize;

	HD3D::Point3D p = m_scence.GetCameraPosition();

	settextstyle(12,0,L"system");

	// 静态分数

	settextcolor(WHITE);
	setbkmode(TRANSPARENT);

	memset(str, 0, sizeof(wchar_t) * size);
	wsprintf(str, L"Progress:           meter(s), %d pillar(s)", m_nGoPillarsNum);
	outtextxy(0, 20, str);

	// 动态分数
	
	if (p.z > 50000)
	{
		settextcolor(YELLOW);
		setbkcolor(RED);
		setbkmode(OPAQUE);
		m_bkColor = RGB(255, 0, 0);
		imgPrize = GetPrizeImage(L"1111000");
	}
	else if (p.z > 39000)
	{
		settextcolor(LIGHTBLUE);
		setbkcolor(YELLOW);
		setbkmode(OPAQUE);
		m_bkColor = RGB(202, 26, 38);
		imgPrize = GetPrizeImage(L"111000");
	}
	else if (p.z > 29000)
	{
		settextcolor(RED);
		setbkcolor(YELLOW);
		setbkmode(OPAQUE);
		m_bkColor = RGB(210, 30, 88);
		imgPrize = GetPrizeImage(L"11000");
	}
	else if (p.z > 23000)
	{
		settextcolor(LIGHTBLUE);
		setbkcolor(YELLOW);
		setbkmode(OPAQUE);
		m_bkColor = RGB(125, 38, 172);
		imgPrize = GetPrizeImage(L"1000");
	}
	else if (p.z > 16000)
	{
		settextcolor(GREEN);
		setbkcolor(YELLOW);
		setbkmode(OPAQUE);
		m_bkColor = RGB(167, 77, 215);
		imgPrize = GetPrizeImage(L"000");
	}
	else if (p.z > 12000)
	{
		settextcolor(YELLOW);
		m_bkColor = RGB(75, 114, 216);
		imgPrize = GetPrizeImage(L"00");
	}
	else if (p.z > 8000)
	{
		settextcolor(BLUE);
		m_bkColor = RGB(45, 191, 184);
		imgPrize = GetPrizeImage(L"0");
	}
	else if (p.z > 3000)
	{
		settextcolor(LIGHTBLUE);
	}
	else if (p.z > 1000)
	{
		settextcolor(GREEN);
	}

	memset(str, 0, sizeof(wchar_t) * size);
	wsprintf(str, L"%d", (int)p.z);
	outtextxy(70, 20, str);

	// 坐标

	settextcolor(WHITE);
	setbkmode(TRANSPARENT);

	memset(str, 0, sizeof(wchar_t) * size);
	wsprintf(str, L"(%d, %d, %d)", (int)p.x, (int)p.y, (int)p.z);
	outtextxy(0, 40, str);

	// 奖牌列表
	putimage_transparent(getwidth() - imgPrize.getwidth(), 0, imgPrize, WHITE);
}

/**
 * @brief 游戏渲染
*/
void GameRender()
{
	setbkcolor(m_bkColor);
	cleardevice();
	double fps = 1.0 / m_scence.Render(-350, -150, { 0.8,2 }, WHITE);
	showfps(fps);
	ShowGamingInfo();
	FlushBatchDraw();
}

/**
 * @brief 游戏初始化
*/
void Init()
{
	srand((unsigned int)time(NULL));

	int z = 0;
	for (int i = 0; i < PILLARS_NUM; i++)
	{
		HD3D::Object3D obj_bottom, obj_top;
		obj_bottom.AddPolygons(GetPillar(PILLAR_SIZE, PILLAR_HEIGHT, PILLAR_SIZE), 6);
		obj_top.AddPolygons(GetPillar(PILLAR_SIZE, PILLAR_HEIGHT, PILLAR_SIZE), 6);

		z += rand() % (PILLAR_MAX_INTERVAL - PILLAR_MIN_INTERVAL) + PILLAR_MIN_INTERVAL + PILLAR_SIZE;
		int y = rand() % (PILLAR_MAX_HEIGHT - PILLAR_MIN_HEIGHT + i * 5 /* 使柱子的高低变化越往后越大 */) + PILLAR_MIN_HEIGHT;
		int gap = rand() % (PILLAR_MAX_GAP - PILLAR_MIN_GAP) + PILLAR_MIN_GAP;

		obj_bottom.MoveZ(z);
		obj_bottom.MoveY(y);
		obj_top.MoveZ(z);
		obj_top.MoveY(y + gap + PILLAR_HEIGHT);

		m_scence.AddObject(obj_bottom);
		m_scence.AddObject(obj_top);

		m_pillars[i] = { z,y,gap };
	}

	m_scence.SetCameraPosition({ 130,400,0 });
	m_scence.SetCameraAttitude({ 0,0,0 });
	m_scence.SetCameraViewportSize(640, 480);
	m_scence.SetCameraFocalLength(2000);

	m_nGoPillarsNum = 0;

	m_speedZ = 3;
	m_speedY = -5;

	loadimage(&m_imgStar, L"./res/pic/star.png");
	loadimage(&m_imgCup, L"./res/pic/cup.png");
	loadimage(&m_imgGuide, L"./res/pic/guide.png");

	m_musicBgm.open(L"./res/music/bgm.mp3");
	m_musicFly.open(L"./res/music/fly.mp3");
	m_musicHit.open(L"./res/music/hit.mp3");
	m_musicScore.open(L"./res/music/score.mp3");
	m_musicWind.open(L"./res/music/wind.mp3");
	m_musicWin.open(L"./res/music/win.mp3");
}

/**
 * @brief 用户输入事件
*/
void Input()
{
	static int last_time[2] = { 0 };
	POINT cursor;

	// 左键
	if (KEY_DOWN(VK_LBUTTON))
	{
		// 俯冲
		if ((double)(clock() - last_time[0]) / CLOCKS_PER_SEC > 0.1)
		{
			m_musicWind.setStartTime(0);
			m_musicWind.play();

			m_scence.MoveCameraZ(10);
			m_scence.MoveCameraY(-20);
			last_time[0] = clock();
		}
	}

	// 右键
	if (KEY_DOWN(VK_RBUTTON))
	{
		// 提升
		if ((double)(clock() - last_time[1]) / CLOCKS_PER_SEC > 0.1)
		{
			m_musicFly.setStartTime(0);
			m_musicFly.play();

			m_scence.MoveCameraZ(5);
			m_scence.MoveCameraY(50);
			last_time[1] = clock();
		}
	}

	// exit
	if (KEY_DOWN(VK_ESCAPE))
	{
		exit(0);
	}

	// 视角旋转
	cursor = GetCursorMoveCenter(m_pCursorCenter);
	if ((cursor.y > 0 && m_scence.GetCameraAttitude().r < 9) || (cursor.y < 0 && m_scence.GetCameraAttitude().r > -9))
		m_scence.RotateCameraX(cursor.y / 4);
	if ((cursor.x < 0 && m_scence.GetCameraAttitude().e < 9) || (cursor.x > 0 && m_scence.GetCameraAttitude().e > -9))
		m_scence.RotateCameraY(-cursor.x / 4);

	// 锁鼠标
	SetCursorAtWindowCenter(GetHWnd());
}

/**
 * @brief 游戏菜单
*/
void Menu()
{
	GameRender();

	setbkcolor(RGB(88, 216, 209));
	//cleardevice();

	settextstyle(70,0,L"Consolas");
	settextcolor(BLUE);
	outtextxy(40, 60, L"Flappy bird 3D");
	settextstyle(20, 0, L"Consolas", 0, 0, 0, 0, 0, 1);
	outtextxy(515, 100, L"(bug edition)");

	settextstyle(40, 0, L"Consolas", 0, 0, 0, 0, 0, 0);
	outtextxy(230, 320, L"Any key to start");

	LPCTSTR str = L"huidong 2021-8-14";
	settextstyle(20, 0, L"Consolas");
	outtextxy(getwidth() - textwidth(str), getheight() - textheight(str), str);

	putimage_transparent(0, getheight() - m_imgGuide.getheight(), m_imgGuide, WHITE);

	FlushBatchDraw();
	
	flushmessage();
	ExMessage msg;
	while (!_kbhit())
	{
		if (peekmessage(&msg, EM_MOUSE))
		{
			if (msg.lbutton || msg.rbutton)
			{
				break;
			}
		}
	}
}

/**
 * @brief 检测玩家是否碰到柱子，并更新玩家穿过的柱子数量
 * @return 若玩家撞到柱子返回 false，否则返回 true
*/
bool HitProgress()
{
	HD3D::Point3D pCamera = m_scence.GetCameraPosition();

	// 碰撞检测
	
	// 越界
	if (pCamera.y < PILLAR_MIN_HEIGHT - PILLAR_HEIGHT || pCamera.y > PILLAR_MAX_HEIGHT + PILLAR_MAX_GAP + PILLAR_HEIGHT)
	{
		return false;
	}

	// 若玩家到柱子前面了
	if (pCamera.z > m_pillars[m_nGoPillarsNum].z)
	{
		// 在柱子内
		if (pCamera.z < m_pillars[m_nGoPillarsNum].z + PILLAR_SIZE)
		{
			if (pCamera.y > m_pillars[m_nGoPillarsNum].y)
			{
				// 在柱子的空隙内
				if (pCamera.y < m_pillars[m_nGoPillarsNum].y + m_pillars[m_nGoPillarsNum].gap)
				{
					// 此处无需额外处理
					// ...
				}

				// 撞到上面的柱子
				else
				{
					m_musicHit.setStartTime(0);
					m_musicHit.play();

					return false;
				}
			}

			// 撞到下面的柱子
			else
			{
				m_musicHit.setStartTime(0);
				m_musicHit.play();

				return false;
			}
		}

		// 已经超出当前柱子了
		// 得分
		else
		{
			m_musicScore.setStartTime(0);
			m_musicScore.play();

			m_nGoPillarsNum++;
		}
	}

	// 在两端柱子的间隙中
	else
	{
		// 此处无需额外处理
		// ...
	}

	return true;
}

/**
 * @brief 胜利
*/
void win()
{
	m_musicWin.play();

	ClipCursor((HWND)NULL);

	setbkcolor(LIGHTBLUE);
	cleardevice();

	putimage(280, 30, &m_imgCup);

	settextstyle(70, 0, L"system");
	settextcolor(YELLOW);
	outtextxy(260,150,L"WIN");

	settextstyle(30, 0, L"system");
	settextcolor(BLACK);
	outtextxy(30,230,L"没想到，真的有人能打到这里。感谢游玩！");
	outtextxy(350,280,L"huidong 2021-8-14");

	settextstyle(20, 0, L"system");
	settextcolor(BLACK);
	outtextxy(260, 430, L"（huidong 偷懒中）");

	FlushBatchDraw();

	Sleep(500);
	flushmessage();
	getmessage(EM_KEY);

	exit(0);
}

/**
 * @brief 游戏运行
 * @return 如果玩家死亡则返回 false
*/
bool Progress()
{
	static int last_time = 0;

	// 定时将玩家往前推进
	if ((double)(clock() - last_time) / CLOCKS_PER_SEC > 0.1)
	{
		m_scence.MoveCameraZ(m_speedZ);
		m_scence.MoveCameraY(m_speedY);
	}

	if (m_nGoPillarsNum >= PILLARS_NUM)
	{
		win();
	}

	return HitProgress();
	//return true;
}

/**
 * @brief 失败界面
*/
void Lost()
{
	ClipCursor((HWND)NULL);

	setlinecolor(LIGHTBLUE);
	setfillcolor(LIGHTBLUE);
	fillrectangle(0, 130, getwidth(), 240);

	settextstyle(70, 0, L"Consolas");
	settextcolor(BLUE);
	outtextxy(180, 140, L"YOU LOST");

	settextstyle(20, 0, L"Consolas");
	settextcolor(BLACK);
	outtextxy(230, 215, L"Any key to back");

	FlushBatchDraw();

	Sleep(500);
	flushmessage();
	while (!_kbhit());
}

/**
 * @brief 循环播放 bgm
*/
void bgm()
{
	DWORD now, total;
	m_musicBgm.getCurrentTime(now);
	m_musicBgm.getTotalTime(total);

	if (now == 0 || now == total)
	{
		m_musicBgm.setStartTime(0);
		m_musicBgm.play();
	}
}

int main(int argc, char** argv)
{
	HD3D::InitDrawingDevice(640, 480, 1);
	m_bkColor = RGB(88, 216, 209);
	BeginBatchDraw();

	Init();
	bgm();
	Menu();

	// 锁鼠标
	ClipCursor(GetHWnd());
	m_pCursorCenter = SetCursorAtWindowCenter(GetHWnd());

	const int sleep_time = 20;

	while (true)
	{
		int t = clock();
		
		Input();

		if (!Progress())
		{
			Lost();
			WinExec(argv[0], SW_SHOW);
			break;
		}

		bgm();
		GameRender();

		int now = clock();
		if (now - t < sleep_time)
		{
			Sleep(sleep_time - now + t);
		}
	}

	HD3D::CloseDrawingDevice();

	return 0;
}

