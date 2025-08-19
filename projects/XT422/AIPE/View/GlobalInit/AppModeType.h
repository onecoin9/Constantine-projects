#pragma once

/// <summary>
/// 连接类型
/// </summary>
enum class ConnectType
{
	None = 0,
	USB,
	Ethernet,
	Demo
};

extern ConnectType g_AppMode;