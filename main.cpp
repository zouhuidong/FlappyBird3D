#include "HuiDong3D.h"
#include "MusicMCI.h"
#include <conio.h>

// Windows GDI ͸����ͼ�������� lib
#pragma comment( lib, "MSIMG32.lib")

// �жϼ��̰���
#define KEY_DOWN(VK_NONAME) (GetForegroundWindow() == GetHWnd() && (GetAsyncKeyState(VK_NONAME) & 0x8000) ? 1:0)

#define PILLAR_SIZE 300		// ���Ӷ���͵���ı߳�
#define PILLAR_HEIGHT 600	// ��������߶�

#define PILLAR_MIN_INTERVAL 700		// ǰ��������С��϶
#define PILLAR_MAX_INTERVAL 1400	// ǰ����������϶

#define PILLAR_MIN_HEIGHT 0		// ����λ�õ���͸߶�
#define PILLAR_MAX_HEIGHT 300	// ����λ�õ���߸߶�

#define PILLAR_MIN_GAP 170	// ����ȱ����С��С
#define PILLAR_MAX_GAP 300	// ����ȱ������С

#define PILLARS_NUM 100	// ��������

// ������
HD3D::Scence3D m_scence;

// ����ڴ�������ʱ��λ��
POINT m_pCursorCenter;

// ��¼��Խ����������
int m_nGoPillarsNum;

// �ٶȣ�Z ��ĺ� Y ��ģ�
int m_speedZ, m_speedY;

// ��������ɫ
COLORREF m_bkColor;

IMAGE m_imgStar;	// ���ǽ���ͼ��
IMAGE m_imgCup;		// ����ͼ��
IMAGE m_imgGuide;	// ��Ϸ����˵��

// ��Ч
MusicMCI m_musicBgm, m_musicFly, m_musicHit, m_musicScore, m_musicWind,m_musicWin;

// ������Ϣ��¼
struct Pillar
{
	int z;	// �������� z
	int y;	// ��������ӵ� y ����
	int gap;// ����ȱ�ڴ�С

}m_pillars[PILLARS_NUM];

/**
 * @brief Windows GDI ����ʵ��͸����ͼ
 * @param x ͼ�����λ��
 * @param y ͼ�����λ��
 * @param img Ҫ�����ͼ��
 * @param bkcolor ͼ�񱳾�ɫ����Ϊ���������ɫ
*/
inline void putimage_transparent(int x, int y, IMAGE img, COLORREF bkcolor)
{
	TransparentBlt(GetImageHDC(GetWorkingImage()), x, y, img.getwidth(), img.getheight(), GetImageHDC(&img), 0, 0, img.getwidth(), img.getheight(), bkcolor);
}

/**
 * @brief ��ȡһ�� 3D ����
 * @param w : ���ӿ��
 * @param h : ���Ӹ߶�
 * @param d : ������ȣ�z����
 * @return ���غ�����������ε�����
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
 * @brief ��������ڴ�����
 * @param hwnd : ���ھ��
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
 * @brief �����λ�������ڴ�������
 * @param hwnd : ���ھ��
 * @return ��������ڴ�������ʱ��λ��
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
 * @brief ��ȡ�����������ڴ������ĵ�ƫ����
 * @param center : ����ڴ�������ʱ��λ��
 * @return ���ص�ǰ�������ڴ������ĵ�λ��
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
 * @brief ��ʾ fps
 * @param fps : ��ǰ֡��
*/
void showfps(double fps)
{
	wchar_t str[32] = { 0 };
	wsprintf(str, L"fps: %d", (int)fps);
	outtextxy(0, 0, str);
}

/**
 * @brief ��ȡ����ͼ��
 * @param strPrize : ���������ַ�����0 ��ʾ���£�1 ��ʾ����
 * @return ���ؽ���ͼ��
*/
IMAGE GetPrizeImage(LPCTSTR strPrize)
{
	const int imgInterval = 10;	// ͼ����
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
 * @brief ��ʾ��ǰ��Ϸ��������
*/
void ShowGamingInfo()
{
	const int size = 256;
	wchar_t str[size] = { 0 };
	IMAGE imgPrize;

	HD3D::Point3D p = m_scence.GetCameraPosition();

	settextstyle(12,0,L"system");

	// ��̬����

	settextcolor(WHITE);
	setbkmode(TRANSPARENT);

	memset(str, 0, sizeof(wchar_t) * size);
	wsprintf(str, L"Progress:           meter(s), %d pillar(s)", m_nGoPillarsNum);
	outtextxy(0, 20, str);

	// ��̬����
	
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

	// ����

	settextcolor(WHITE);
	setbkmode(TRANSPARENT);

	memset(str, 0, sizeof(wchar_t) * size);
	wsprintf(str, L"(%d, %d, %d)", (int)p.x, (int)p.y, (int)p.z);
	outtextxy(0, 40, str);

	// �����б�
	putimage_transparent(getwidth() - imgPrize.getwidth(), 0, imgPrize, WHITE);
}

/**
 * @brief ��Ϸ��Ⱦ
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
 * @brief ��Ϸ��ʼ��
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
		int y = rand() % (PILLAR_MAX_HEIGHT - PILLAR_MIN_HEIGHT + i * 5 /* ʹ���ӵĸߵͱ仯Խ����Խ�� */) + PILLAR_MIN_HEIGHT;
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
 * @brief �û������¼�
*/
void Input()
{
	static int last_time[2] = { 0 };
	POINT cursor;

	// ���
	if (KEY_DOWN(VK_LBUTTON))
	{
		// ����
		if ((double)(clock() - last_time[0]) / CLOCKS_PER_SEC > 0.1)
		{
			m_musicWind.setStartTime(0);
			m_musicWind.play();

			m_scence.MoveCameraZ(10);
			m_scence.MoveCameraY(-20);
			last_time[0] = clock();
		}
	}

	// �Ҽ�
	if (KEY_DOWN(VK_RBUTTON))
	{
		// ����
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

	// �ӽ���ת
	cursor = GetCursorMoveCenter(m_pCursorCenter);
	if ((cursor.y > 0 && m_scence.GetCameraAttitude().r < 9) || (cursor.y < 0 && m_scence.GetCameraAttitude().r > -9))
		m_scence.RotateCameraX(cursor.y / 4);
	if ((cursor.x < 0 && m_scence.GetCameraAttitude().e < 9) || (cursor.x > 0 && m_scence.GetCameraAttitude().e > -9))
		m_scence.RotateCameraY(-cursor.x / 4);

	// �����
	SetCursorAtWindowCenter(GetHWnd());
}

/**
 * @brief ��Ϸ�˵�
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
 * @brief �������Ƿ��������ӣ���������Ҵ�������������
 * @return �����ײ�����ӷ��� false�����򷵻� true
*/
bool HitProgress()
{
	HD3D::Point3D pCamera = m_scence.GetCameraPosition();

	// ��ײ���
	
	// Խ��
	if (pCamera.y < PILLAR_MIN_HEIGHT - PILLAR_HEIGHT || pCamera.y > PILLAR_MAX_HEIGHT + PILLAR_MAX_GAP + PILLAR_HEIGHT)
	{
		return false;
	}

	// ����ҵ�����ǰ����
	if (pCamera.z > m_pillars[m_nGoPillarsNum].z)
	{
		// ��������
		if (pCamera.z < m_pillars[m_nGoPillarsNum].z + PILLAR_SIZE)
		{
			if (pCamera.y > m_pillars[m_nGoPillarsNum].y)
			{
				// �����ӵĿ�϶��
				if (pCamera.y < m_pillars[m_nGoPillarsNum].y + m_pillars[m_nGoPillarsNum].gap)
				{
					// �˴�������⴦��
					// ...
				}

				// ײ�����������
				else
				{
					m_musicHit.setStartTime(0);
					m_musicHit.play();

					return false;
				}
			}

			// ײ�����������
			else
			{
				m_musicHit.setStartTime(0);
				m_musicHit.play();

				return false;
			}
		}

		// �Ѿ�������ǰ������
		// �÷�
		else
		{
			m_musicScore.setStartTime(0);
			m_musicScore.play();

			m_nGoPillarsNum++;
		}
	}

	// ���������ӵļ�϶��
	else
	{
		// �˴�������⴦��
		// ...
	}

	return true;
}

/**
 * @brief ʤ��
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
	outtextxy(30,230,L"û�뵽����������ܴ������л���棡");
	outtextxy(350,280,L"huidong 2021-8-14");

	settextstyle(20, 0, L"system");
	settextcolor(BLACK);
	outtextxy(260, 430, L"��huidong ͵���У�");

	FlushBatchDraw();

	Sleep(500);
	flushmessage();
	getmessage(EM_KEY);

	exit(0);
}

/**
 * @brief ��Ϸ����
 * @return �����������򷵻� false
*/
bool Progress()
{
	static int last_time = 0;

	// ��ʱ�������ǰ�ƽ�
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
 * @brief ʧ�ܽ���
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
 * @brief ѭ������ bgm
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

	// �����
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

