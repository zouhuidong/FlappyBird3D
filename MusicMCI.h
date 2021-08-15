#pragma once

/************************************************************************
 * 作者 : 悠远的苍穹
 * QQ   : 2237505658
 * 邮箱 : 2237505658@qq.com
 * 最后修改  : 2020-8-17
 * 编译环境  : Visual Studio 2019
 ************************************************************************/

#ifdef _DEBUG
#include <iostream>
#endif

#include <fstream>
#include <string>
#include <vector>
#include <array>
#include <ctime>
#include <cmath>
#include <io.h>
#include <conio.h>
#include <graphics.h>
#include <windows.h>
#include <mmsystem.h>
#include <Digitalv.h>
#pragma comment(lib, "winmm.lib")

using std::wstring;
using std::vector;
using std::array;

class MusicMCI
{
private:
	// 设备 ID
	MCIDEVICEID nDeviceID;

public:

	MusicMCI()noexcept
	{
		nDeviceID = -1;
	}

	~MusicMCI()
	{
		if (nDeviceID != -1) this->close();
	}

	// 打开文件
	// 成功返回 true，失败返回 false
	bool open(LPCWSTR strSongPath)noexcept
	{
		MCI_OPEN_PARMS mciOP;

		mciOP.lpstrDeviceType = nullptr;
		mciOP.lpstrElementName = strSongPath;
		const DWORD dwReturn = mciSendCommand(0, MCI_OPEN,
			MCI_OPEN_ELEMENT | MCI_WAIT | MCI_OPEN_SHAREABLE, (DWORD_PTR)(static_cast<LPVOID>(&mciOP)));

		if (dwReturn == 0)
		{
			nDeviceID = mciOP.wDeviceID;
			return true;
		}
		else
		{
			nDeviceID = -1;
			return false;
		}
	}

	// 播放
	// 成功返回非零值，失败返回0
	bool play()noexcept
	{
		MCI_PLAY_PARMS mciPP{};

		const DWORD dwReturn = mciSendCommand(nDeviceID, MCI_PLAY,
			MCI_NOTIFY, (DWORD_PTR)(static_cast<LPVOID>(&mciPP)));
		if (dwReturn == 0)
			return true;
		else
			return false;
	}

	// 暂停播放
	// 成功返回 true，失败返回 false
	bool pause()noexcept
	{
		MCI_GENERIC_PARMS mciGP{};

		const DWORD dwReturn = mciSendCommand(nDeviceID, MCI_PAUSE,
			MCI_NOTIFY | MCI_WAIT, (DWORD_PTR)(static_cast<LPVOID>(&mciGP)));
		if (dwReturn == 0)
			return true;
		else
			return false;
	}

	// 停止播放并使进度返回到开头
	// 成功返回 true，失败返回 false
	bool stop()noexcept
	{
		MCI_SEEK_PARMS mciSP{};

		const DWORD dwReturn = mciSendCommand(nDeviceID, MCI_SEEK,
			MCI_WAIT | MCI_NOTIFY | MCI_SEEK_TO_START, (DWORD_PTR)(static_cast<LPVOID>(&mciSP)));
		if (dwReturn == 0)
			return true;
		else
			return false;
	}

	// 关闭MCI设备
	// 成功返回 true，失败返回 false
	bool close()noexcept
	{
		MCI_GENERIC_PARMS mciGP{};

		const DWORD dwReturn = mciSendCommand(nDeviceID, MCI_CLOSE,
			MCI_NOTIFY | MCI_WAIT, (DWORD_PTR)(static_cast<LPVOID>(&mciGP)));
		if (dwReturn == 0)
		{
			nDeviceID = -1;
			return true;
		}
		else
			return false;
	}

	// 获得当前播放进度，pos 以 ms 为单位
	// 成功返回 true，失败返回 false
	bool getCurrentTime(DWORD& pos)noexcept
	{
		MCI_STATUS_PARMS mciSP{};

		mciSP.dwItem = MCI_STATUS_POSITION;
		const DWORD dwReturn = mciSendCommand(nDeviceID, MCI_STATUS,
			MCI_STATUS_ITEM, (DWORD_PTR)(static_cast<LPVOID>(&mciSP)));
		if (dwReturn == 0)
		{
			pos = static_cast<DWORD>(mciSP.dwReturn);
			return true;
		}
		else
		{
			pos = 0;
			return false;
		}
	}

	// 获取音乐总时长，time 以ms 为单位
	// 成功返回 true，失败返回 false
	bool getTotalTime(DWORD& time)noexcept
	{
		MCI_STATUS_PARMS mciSP{};

		mciSP.dwItem = MCI_STATUS_LENGTH;
		const DWORD dwReturn = mciSendCommand(nDeviceID, MCI_STATUS,
			MCI_WAIT | MCI_STATUS_ITEM, (DWORD_PTR)(static_cast<LPVOID>(&mciSP)));	// 关键，取得长度
		if (dwReturn == 0)
		{
			time = static_cast<DWORD>(mciSP.dwReturn);
			return true;
		}
		else
		{
			time = 0;
			return false;
		}
	}

	// 设置音量大小，音量值范围在 0 到 1000
	// 成功返回 true，失败返回 false
	bool setVolume(size_t nVolumeValue)noexcept
	{
		if (nVolumeValue > 1000)
		{
			nVolumeValue = 1000;
		}
		else if (nVolumeValue < 0)
		{
			nVolumeValue = 0;
		}

		MCI_DGV_SETAUDIO_PARMS mciDSP;
		mciDSP.dwItem = MCI_DGV_SETAUDIO_VOLUME;
		mciDSP.dwValue = static_cast<DWORD>(nVolumeValue);
		const DWORD dwReturn = mciSendCommand(nDeviceID, MCI_SETAUDIO,
			MCI_DGV_SETAUDIO_VALUE | MCI_DGV_SETAUDIO_ITEM, (DWORD_PTR)(static_cast<LPVOID>(&mciDSP)));
		if (dwReturn == 0)
			return true;
		else
			return false;
	}

	// 设置播放起始位置
	// 成功返回 true，失败返回 false
	bool setStartTime(size_t start_time) noexcept
	{
		DWORD end_time = 0;
		this->getTotalTime(end_time);

		if (start_time > end_time)
			return false;

		MCI_PLAY_PARMS mciPlay{};
		mciPlay.dwFrom = static_cast<DWORD>(start_time);
		mciPlay.dwTo = static_cast<DWORD>(end_time);
		const DWORD dwReturn = mciSendCommand(nDeviceID, MCI_PLAY,
			MCI_TO | MCI_FROM, (DWORD_PTR)(static_cast<LPVOID>(&mciPlay)));

		if (dwReturn == 0)
			return true;
		else
			return false;
	}
};